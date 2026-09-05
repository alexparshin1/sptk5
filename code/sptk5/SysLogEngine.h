/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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

#pragma once

#ifndef _WIN32
#include <syslog.h>
#else
#include <winsock2.h>

#include <windows.h>
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
 * The facility defines which system log is used: it selects the syslog facility
 * on *nix, and the matching Event Log category on Windows, so the same facility
 * groups messages the same way on every platform.
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
    bool saveMessage(const Logger::Message& message) override;

    /**
     * Constructor
     *
     * Creates a new log object based on the syslog facility.
     * For Windows, messages are stored into the Application event log, and the
     * facility selects the event category.
     * The program name is optional. It is set for all the SysLogEngine objects at once.
     * If set, it appears in the log as a message prefix. Every time the program
     * name is changed, the log is closed to be re-opened on next message.
     * @param programName       Program name
     * @param facilities        Log facility, one of LOG_KERN..LOG_LOCAL7.
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
#endif

    static std::mutex       m_syslogMutex;
    static std::atomic_bool m_logOpened;

    uint32_t m_facilities;  ///< Syslog facility defining which system log receives the messages
    String   m_programName; ///< Application name

    void programName(const String& progName);

    void setupEventSource() const;
};
/**
 * @}
 */
} // namespace sptk
