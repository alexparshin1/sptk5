/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-16                                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/OsProcess.h>
#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/wait.h>
#endif

using namespace std;
using namespace sptk;

#ifndef _WIN32
namespace {
FILE* popen2(const string& command, string_view type, int& pid);
int   pclose2(FILE* fp, pid_t pid);
} // namespace
#endif

OsProcess::OsProcess(string command, std::function<void(const string&)> onData)
    : m_command(std::move(command))
    , m_onData(std::move(onData))
{
}

OsProcess::~OsProcess()
{
    close();
    if (m_task.joinable())
    {
        m_task.join();
    }
}

void OsProcess::start()
{
    m_terminated = false;

    const scoped_lock lock(m_mutex);

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

    string commandStr = m_command;

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
        throw Exception("Can't start process");
    }
#else
    m_stdout = popen2(m_command, "r", m_pid);
    if (m_stdout == nullptr)
    {
        throw Exception("Can't start process");
    }
#endif
    m_task = jthread([this]
                     {
                         readData();
                         return close();
                     });
}

int OsProcess::waitForData(const chrono::milliseconds& timeout)
{
#ifdef _WIN32
    static constexpr chrono::milliseconds sleepTime = 10ms;
    chrono::milliseconds                  totalWait = 0ms;
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
    auto bytesAvailable = 0;
    int  fd;
    {
        const scoped_lock lock(m_mutex);
        if (m_stdout == nullptr)
        {
            return -1;
        }
        fd = fileno(m_stdout);
    }

    array<pollfd, 1> fds {};
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
                m_terminated = true;
                return -1;
            }

            return bytesAvailable;
        default:
            return -1;
    }
#endif
}

bool OsProcess::isEof() const
{
#ifndef _WIN32
    const scoped_lock lock(m_mutex);
    return m_stdout != nullptr && feof(m_stdout);
#else
    return m_stdout == nullptr;
#endif
}

void OsProcess::readData()
{
    m_buffer.fill(0);

    while (!m_terminated && !isEof())
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
        if (!ReadFile(m_stdout, m_buffer.data(), readSize, &readSize, nullptr))
        {
            break;
        }
#else
        const auto readSize = static_cast<size_t>(bytesAvailable) > BufferSize ? BufferSize : bytesAvailable;
        if (readSize > 0)
        {
            const scoped_lock lock(m_mutex);
            if (m_stdout == nullptr ||
                fread(m_buffer.data(), readSize, 1, m_stdout) == 0)
            {
                break;
            }
        }
#endif
        if (m_onData)
        {
            m_onData(String(m_buffer.data(), readSize));
        }
    }
}

int OsProcess::wait()
{
    if (m_task.joinable())
    {
        m_task.join();
    }
    return m_exitCode;
}

void OsProcess::kill()
{
    const scoped_lock lock(m_mutex);

    m_terminated = true;
#ifdef _WIN32
    if (TerminateProcess(m_processInformation.hProcess, 0) == 0)
    {
        //throw SystemException("Can't kill process");
    }
#else
    if (m_pid == 0)
    {
        throw SystemException("Can't kill process: pid is 0");
    }

    if (auto rc = ::kill(m_pid, SIGKILL);
        rc != 0)
    {
        throw SystemException("Can't kill process");
    }
#endif
}

int OsProcess::close()
{
    FileHandle out;

    {
        const scoped_lock lock(m_mutex);

        if (m_stdout == nullptr)
        {
            return m_exitCode;
        }
        // Claim the handle before closing it: close() can be called concurrently from the
        // reader task and the destructor, and both would otherwise fclose the same FILE*.
        out = m_stdout;
        m_stdout = nullptr;
    }

    m_terminated = true;

    auto exitCode = 0;

#ifdef _WIN32
    WaitForSingleObject(m_processInformation.hProcess, 10000);

    DWORD dwExitCode = 0;
    GetExitCodeProcess(m_processInformation.hProcess, &dwExitCode);
    exitCode = static_cast<int>(dwExitCode);

    CloseHandle(m_processInformation.hProcess);
    CloseHandle(m_processInformation.hThread);
    m_processInformation.hProcess = nullptr;
    m_processInformation.hThread = nullptr;

    if (out)
    {
        CloseHandle(out);
        CloseHandle(m_stdin);
    }
    m_stdin = nullptr;
#else
    if (out)
    {
        const auto status = pclose2(out, m_pid);
        if (WIFEXITED(status))
        {
            exitCode = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            exitCode = WTERMSIG(status);
        }
        else if (WIFSTOPPED(status))
        {
            exitCode = WSTOPSIG(status);
        }
        else
        {
            exitCode = -1;
        }
    }
#endif

    const scoped_lock lock(m_mutex);
    m_exitCode = exitCode;

    return m_exitCode;
}

#ifdef _WIN32
string OsProcess::getErrorMessage(DWORD lastError)
{
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 nullptr, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

    //Copy the error message into a String.
    string message(messageBuffer, size);

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
    if (auto* envData = popen("env", "r"))
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
        if ((value.starts_with('\"') && value.ends_with('\"')) ||
            (value.starts_with('\'') && value.ends_with('\'')))
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

constexpr auto READ = 0;
constexpr auto WRITE = 1;

/**
 * @brief Executes a command in a subprocess, connecting the process's input or output to a pipe.
 * @param command               Command to execute.
 * @param type                  "r" to read from the subprocess's output, "w" to write to the subprocess's input.
 * @param pid                   Reference to an integer where the subprocess's PID will be stored.
 * @return A file pointer connected to the subprocess's input or output, depending on the type parameter.
 * @throws SystemException If pipe creation, process forking, or command execution fails.
 */
FILE* popen2(const string& command, const string_view type, int& pid)
{
    pid_t         child_pid {0};
    array<int, 2> fd {};
    if (pipe(fd.data()) != 0)
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

        if (const auto rc = execvpe(args[0], args.data(), (char* const*) envs.data());
            rc != 0)
        {
            throw SystemException("Can't execute command");
        }

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
int pclose2(FILE* fp, const pid_t pid)
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
