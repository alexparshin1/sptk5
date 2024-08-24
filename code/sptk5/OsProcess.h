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

#include <filesystem>
#include <functional>
#include <future>
#include <vector>

namespace sptk {

class OsProcess
{
public:
    /**
     * @brief Constructor
     * @param command           Command to execute
     */
    OsProcess(sptk::String command, std::function<void(const sptk::String&)> onData = nullptr);

    /**
     * @brief Destructor
     */
    ~OsProcess();

    void start();
    int  wait();
    int  wait_for(std::chrono::milliseconds timeout);

private:
    static constexpr size_t BufferSize = 1024;
#ifdef _WIN32
    using FileHandle = HANDLE;
#else
    using FileHandle = FILE*;
#endif
    std::mutex                               m_mutex;
    sptk::String                             m_command;
    std::function<void(const sptk::String&)> m_onData;
    FileHandle                               m_stdout {};
    FileHandle                               m_stdin {};
    std::future<int>                         m_task;
    std::atomic_bool                         m_terminated {false};
#ifdef _WIN32
    static sptk::String getErrorMessage(DWORD lastError);
    PROCESS_INFORMATION m_ProcessInformation {};
#endif
    int  waitForData(std::chrono::milliseconds timeout);
    void readData();
    int  close();
};

using SOsProcess = std::shared_ptr<OsProcess>;

} // namespace sptk
