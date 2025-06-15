/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/OsProcess.h>
#include <sys/wait.h>
#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/poll.h>
#endif

using namespace std;
using namespace sptk;

namespace {
FILE* popen2(const string& command, const string& type, int& pid);
int   pclose2(FILE* fp, pid_t pid);
} // namespace

OsProcess::OsProcess(sptk::String command, std::function<void(const sptk::String&)> onData)
    : m_command(std::move(command))
    , m_onData(std::move(onData))
{
}

OsProcess::~OsProcess()
{
    close();
}

void OsProcess::start()
{
    m_terminated = false;

#ifdef _WIN32
    STARTUPINFO         si;
    SECURITY_ATTRIBUTES saAttr;

    ZeroMemory(&saAttr, sizeof(saAttr));
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    // Create a pipe for the child process's STDOUT.

    if (!CreatePipe(&m_stdout, &m_stdin, &saAttr, 0))
    {
        throw runtime_error("Can't create pipe");
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if (!SetHandleInformation(m_stdout, HANDLE_FLAG_INHERIT, 0))
    {
        throw runtime_error("Can't modify pipe handle");
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = m_stdin;
    si.hStdOutput = m_stdin;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&m_processInformation, sizeof(m_processInformation));

    // Start the child process.
    if (!CreateProcessA(nullptr,               // No module name (use command line)
                        m_command.data(),      // Command line
                        nullptr,               // Process handle not inheritable
                        nullptr,               // Thread handle not inheritable
                        true,                  // Set the handle inheritance
                        0,                     // No creation flags
                        nullptr,               // Use parent's environment block
                        nullptr,               // Use parent's starting directory
                        &si,                   // Pointer to STARTUPINFO structure
                        &m_processInformation) // Pointer to PROCESS_INFORMATION structure
    )
    {
        throw runtime_error("Can't start process");
    }
#else
    m_stdout = popen2(m_command.c_str(), "r", m_pid);
    if (m_stdout == nullptr)
    {
        throw runtime_error("Can't start process");
    }
#endif
    m_task = async(launch::async, [this]
                   {
                       readData();
                       return close();
                   });
}

int OsProcess::waitForData(const chrono::milliseconds& timeout) const
{
#ifdef _WIN32
    chrono::milliseconds sleepTime = 10ms;
    chrono::milliseconds totalWait = 0ms;
    while (totalWait < timeout)
    {
        DWORD bytesRead = 0;
        DWORD bytesAvailable = 0;
        DWORD bytesLeftThisMessage = 0;
        if (!PeekNamedPipe(m_stdout, nullptr, BufferSize, &bytesRead, &bytesAvailable, &bytesLeftThisMessage))
        {
            return -1;
        }

        if (bytesAvailable > 0)
        {
            return static_cast<int>(bytesAvailable);
        }

        if (WaitForSingleObject(m_processInformation.hProcess, static_cast<DWORD>(sleepTime.count())) == WAIT_OBJECT_0)
        {
            m_terminated = true;
            break;
        }

        totalWait += sleepTime;
    }
    return 0;
#else
    int              bytesAvailable = 0;
    array<pollfd, 1> fds {};
    auto             fd = fileno(m_stdout);
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    switch (poll(fds.data(), 1, static_cast<int>(timeout.count())))
    {
        case 0:
            return 0;
        case 1:
            if (ioctl(fd, FIONREAD, &bytesAvailable) < 0)
            {
                return -1;
            }
            if (bytesAvailable == 0)
            {
                // EOF
                return -1;
            }

            return bytesAvailable;
        default:
            return -1;
    }
#endif
}

void OsProcess::readData() const
{
    array<char, BufferSize> buffer {};

    while (!m_terminated
#ifndef _WIN32
           && !feof(m_stdout)
#endif
    )
    {
        auto bytesAvailable = waitForData(500ms);
        if (bytesAvailable == -1)
        {
            break;
        }
        if (bytesAvailable == 0)
        {
            continue;
        }

#ifdef _WIN32
        DWORD readSize = bytesAvailable > BufferSize ? BufferSize : bytesAvailable;
        if (!ReadFile(m_stdout, buffer.data(), readSize, &readSize, nullptr))
        {
            break;
        }
#else
        size_t readSize = static_cast<size_t>(bytesAvailable) > BufferSize ? BufferSize : bytesAvailable;
        if (fread(buffer.data(), readSize, 1, m_stdout) == 0)
        {
            break;
        }
#endif
        if (m_onData)
        {
            m_onData(String(buffer.data(), readSize));
        }
    }
}

int OsProcess::wait()
{
    return m_task.get();
}

int OsProcess::wait_for(const chrono::milliseconds& timeout)
{
    if (m_task.wait_for(timeout) == std::future_status::ready)
    {
        return m_task.get();
    }
    return -1;
}

void OsProcess::kill(int signal)
{
    lock_guard lock(m_mutex);
    m_terminated = true;
#ifndef _WIN32
    auto rc = ::kill(m_pid, signal);
    if (rc != 0)
    {
        throw SystemException("Can't kill process");
    }
#endif
}

int OsProcess::close()
{
    lock_guard lock(m_mutex);

    m_terminated = true;
    auto exitCode = 0;

#ifdef _WIN32
    if (m_stdout)
    {
        CloseHandle(m_stdout);
        CloseHandle(m_stdin);
        m_stdin = nullptr;
        m_stdout = nullptr;
    }
    DWORD dwExitCode = 0;
    GetExitCodeProcess(m_processInformation.hProcess, &dwExitCode);
    exitCode = static_cast<int>(dwExitCode);
#else
    if (m_stdout)
    {
        exitCode = pclose2(m_stdout, m_pid);
        m_stdout = nullptr;
    }
#endif
    return exitCode;
}

#ifdef _WIN32
String OsProcess::getErrorMessage(DWORD lastError)
{
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 nullptr, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR) &messageBuffer, 0, nullptr);

    //Copy the error message into a String.
    String message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif

namespace {

#ifndef _WIN32

/**
 * @brief Get a copy of environment strings.
 * @return Environment variables as vector of "VARNAME=VARVALUE".
 */
Strings getEnvironment()
{
    Strings env;
    auto*   envData = popen("env", "r");
    if (envData != nullptr)
    {
        array<char, 1024> buffer {};
        while (!feof(envData))
        {
            if (fgets(buffer.data(), buffer.size() - 1, envData))
            {
                String line(buffer.data());
                env.push_back(line.trim());
            }
        }
        pclose(envData);
    }
    return env;
}

/**
 * @brief Parse the command to command and arguments.
 * @remarks Enquoted arguments are returned as single strings.
 * @param command               Command to execute.
 * @return Command and arguments as a string vector.
 */
Strings commandToArguments(const String& command)
{
    Strings                        args;
    static const RegularExpression matchArguments(R"(("[^"]+"|\S+))", "g");
    auto                           matches = matchArguments.m(command);
    for (const auto& match: matches.groups())
    {
        const auto& value = match.value;
        if ((value.startsWith("\"") && value.endsWith("\"")) ||
            (value.startsWith("'") && value.endsWith("'")))
        {
            args.push_back(value.substr(1, value.size() - 2));
        }
        else
        {
            args.push_back(value);
        }
    }
    return args;
}

#define READ 0
#define WRITE 1

/**
 * @brief Executes a command in a subprocess, connecting the process's input or output to a pipe.
 * @param command               Command to execute.
 * @param type                  "r" to read from the subprocess's output, "w" to write to the subprocess's input.
 * @param pid                   Reference to an integer where the subprocess's PID will be stored.
 * @return A file pointer connected to the subprocess's input or output, depending on the type parameter.
 * @throws SystemException If pipe creation, process forking, or command execution fails.
 */
FILE* popen2(const string& command, const string& type, int& pid)
{
    pid_t child_pid {0};
    int   fd[2] {};
    if (pipe(fd) != 0)
    {
        throw SystemException("Can't create pipe");
    }

    if ((child_pid = fork()) == -1)
    {
        throw SystemException("Can't start the process");
    }

    if (child_pid == 0)
    {
        // child process
        if (type == "r")
        {
            close(fd[READ]);    //Close the READ end of the pipe since the child's fd is write-only
            dup2(fd[WRITE], 1); //Redirect stdout to pipe
        }
        else
        {
            close(fd[WRITE]);  //Close the WRITE end of the pipe since the child's fd is read-only
            dup2(fd[READ], 0); //Redirect stdin to pipe
        }

        setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh

        auto argStrings = commandToArguments(command);
        auto envStrings = getEnvironment();

        vector<char*> args;
        for (auto& arg: argStrings)
        {
            args.push_back(arg.data());
        }
        args.push_back(nullptr);

        vector<char*> envs;
        for (auto& env: envStrings)
        {
            envs.push_back(env.data());
        }
        envs.push_back(nullptr);

        auto rc = execvpe(args[0], args.data(), (char* const*) envs.data());
        if (rc != 0)
        {
            throw SystemException("Can't execute command");
        }

        //system(command.c_str());
        exit(0);
    }

    // Parent process

    if (type == "r")
    {
        close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
    }
    else
    {
        close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
    }

    pid = child_pid;

    if (type == "r")
    {
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

/**
 * @brief Closes a file pointer associated with a subprocess and waits for the process to terminate.
 * @param fp                    File pointer to the stream opened by `popen2`.
 * @param pid                   Process ID of the subprocess to wait for.
 * @return The termination status of the subprocess, or -1 if an error occurs.
 */
int pclose2(FILE* fp, pid_t pid)
{
    int stat;

    fclose(fp);
    while (waitpid(pid, &stat, 0) == -1)
    {
        if (errno != EINTR)
        {
            stat = -1;
            break;
        }
    }

    return stat;
}
#endif
} // namespace
