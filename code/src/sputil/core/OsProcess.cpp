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
#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/poll.h>
#endif

using namespace std;
using namespace sptk;

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
    if (!CreateProcessA(nullptr,          // No module name (use command line)
                        m_command.data(), // Command line
                        nullptr,          // Process handle not inheritable
                        nullptr,          // Thread handle not inheritable
                        true,             // Set handle inheritance
                        0,                // No creation flags
                        nullptr,          // Use parent's environment block
                        nullptr,          // Use parent's starting directory
                        &si,              // Pointer to STARTUPINFO structure
                        &m_processInformation) // Pointer to PROCESS_INFORMATION structure
    )
    {
        throw runtime_error("Can't start process");
    }
#else
    m_stdout = popen(m_command.c_str(), "r");
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

int OsProcess::waitForData(std::chrono::milliseconds timeout)
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
    auto result = poll(fds.data(), 1, (int) timeout.count());
    switch (result)
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

void OsProcess::readData()
{
    array<char, BufferSize> buffer {};

    while (!m_terminated)
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
        size_t readSize = (size_t) bytesAvailable > BufferSize ? BufferSize : bytesAvailable;
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

int OsProcess::wait_for(std::chrono::milliseconds timeout)
{
    if (m_task.wait_for(timeout) == std::future_status::ready)
    {
        return m_task.get();
    }
    return -1;
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
        exitCode = pclose(m_stdout);
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
