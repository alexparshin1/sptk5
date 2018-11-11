/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SysLogEngine.h - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CSYSLOG_H__
#define __CSYSLOG_H__

#ifndef _WIN32
#include <syslog.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include <sptk5/LogEngine.h>

namespace sptk
{

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * @brief A log stored in the system log.
 *
 * On *nix , the log is sent to *nix syslog daemon.
 * On Windows NT/2000+/XP the log is sent to Event Log (Application).
 * On Windows 95/98/ME the system log isn't supported..
 * The facility method allows to define - which system log is used
 */
class SP_EXPORT SysLogEngine: public LogEngine
{
#ifdef _WIN32
    /**
     * (Windows) The handle of the log file
     */
    std::atomic<HANDLE> m_logHandle;

#endif

    /**
     * List of facilities allows to define one or more system logs where messages would be sent
     */
    uint32_t            m_facilities;

    std::string         m_programName;

    void programName(const std::string& progName);
    void setupEventSource();
public:
    /**
     * @brief Stores or sends log message to actual destination
     *
     * This method should be overwritten by the actual log implementation
     * @param message           Log message
     */
    virtual void saveMessage(const Logger::Message* message) override;
public:
    /**
     * @brief Constructor
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
    SysLogEngine(const std::string& programName = "", uint32_t facilities = LOG_USER);

    /**
     * @brief Destructor
     *
     * Destructs the log object, closes the log descriptor, releases all the allocated resources
     */
    virtual ~SysLogEngine();
};
/**
 * @}
 */
}
#endif
