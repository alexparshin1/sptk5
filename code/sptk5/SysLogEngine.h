/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#ifndef _WIN32
#include <syslog.h>
#else
#include <windows.h>
#include <winsock2.h>
#endif

#include <sptk5/LogEngine.h>

namespace sptk {

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * A log stored in the system log.
 *
 * On *nix , the log is sent to *nix syslog daemon.
 * On Windows NT/2000+/XP the log is sent to Event Log (Application).
 * On Windows 95/98/ME the system log isn't supported..
 * The facility method allows to define - which system log is used
 */
class SP_EXPORT SysLogEngine
    : public LogEngine
{
public:
    /**
     * Stores or sends log message to actual destination
     *
     * This method should be overwritten by the actual log implementation
     * @param message           Log message
     */
    void saveMessage(const Logger::UMessage& message) override;

    /**
     * Constructor
     *
     * Creates a new log object based on the syslog facility (or facilities).
     * For Windows, parameter facilities is ignored and messages are stored
     * into Application event log.
     * The program name is optional. It is set for all the SysLogEngine objects at once.
     * If set, it appears in the log as a message prefix. Every time the program
     * name is changed, the log is closed to be re-opened on next message.
     * @param programName       Program name
     * @param facilities        Log facility or a set of facilities.
     */
    SysLogEngine(const String& programName, uint32_t facilities = LOG_USER);

    /**
     * Destructor
     *
     * Destructs the log object, closes the log descriptor, releases all the allocated resources
     */
    ~SysLogEngine() override;

    /**
     * Get log engine options
     * @param options           Log engine output options
     * @param programName       Log engine program name
     * @param facilities        Log engine facilities
     */
    void getOptions(std::set<Option>& options, String& programName, uint32_t& facilities) const;

private:
#ifdef _WIN32
    std::atomic<HANDLE> m_logHandle {0}; ///< The handle of the log file
    static bool m_registrySet;           ///< Is registry set?
#endif

    static std::mutex m_syslogMutex;
    static std::atomic_bool m_logOpened;

    uint32_t m_facilities; ///< List of facilities allows to define one or more system logs where messages would be sent
    String m_programName;  ///< Application name

    void programName(const String& progName);

    void setupEventSource() const;
};
/**
 * @}
 */
} // namespace sptk
