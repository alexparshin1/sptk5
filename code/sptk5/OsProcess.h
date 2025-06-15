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

#pragma once

#include <sptk5/cutils>

#include <functional>
#include <future>

namespace sptk {
/**
	 * @brief Asynchronously execute the OS process, optionally capturing its output to callback function
	 */
class SP_EXPORT OsProcess
{
public:
    /**
     * @brief Constructor
     * @param command           Command to execute
     * @param onData            Optional callback function called upon process output
     */
    explicit OsProcess(String command, std::function<void(const String&)> onData = nullptr);

    /**
     * @brief Destructor
     */
    ~OsProcess();

    /**
     * @brief Asynchronous start of the process
     */
    void start();

    /**
     * @brief Wait until the process exits
     * @return process exit code
     */
    int wait();

    /**
     * @brief Wait until the process exits
     * @param timeout           Maximum time to wait for process exit
     * @return process exit code
     */
    int wait_for(const std::chrono::milliseconds& timeout);

    /**
     * @brief Kill process
     */
    void kill();

    int close();

private:
    static constexpr size_t BufferSize = 16384; ///< Read buffer size
#ifdef _WIN32
    using FileHandle = HANDLE;
#else
    using FileHandle = FILE*;
#endif
    std::mutex                               m_mutex;              ///< Mutex that protects internal data
    String                                   m_command;            ///< Process command
    std::function<void(const sptk::String&)> m_onData;             ///< Optional callback function called on process output
    FileHandle                               m_stdout {};          ///< Process stdout
    std::future<int>                         m_task;               ///< Process execution task
    std::atomic_bool                         m_terminated {false}; ///< Process terminate flag
    int                                      m_pid {0};            ///< Process id
#ifdef _WIN32
    FileHandle          m_stdin {};                       ///< Process stdin
    static sptk::String getErrorMessage(DWORD lastError); ///< Get error message
    PROCESS_INFORMATION m_processInformation {};          ///< Process information (Windows only)
#endif
    int  waitForData(const std::chrono::milliseconds& timeout); ///< Wait for process output
    void readData();                                            ///< Read process output
};

using SOsProcess = std::shared_ptr<OsProcess>;

} // namespace sptk
