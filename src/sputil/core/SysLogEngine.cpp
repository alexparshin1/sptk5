/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SysLogEngine.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/SysLogEngine.h>
#include <sstream>

using namespace std;
using namespace sptk;

#ifndef _WIN32
    static int      m_objectCounter(0);
    static bool     m_logOpened(false);
#else
    #include <events.w32/events.h>
    static string   m_moduleFileName;
    static bool     m_registrySet(false);
#endif

/* Unix facilities
 { "auth", LOG_AUTH },
 { "authpriv", LOG_AUTHPRIV },
 { "cron", LOG_CRON },
 { "daemon", LOG_DAEMON },
 { "ftp", LOG_FTP },
 { "kern", LOG_KERN },
 { "lpr", LOG_LPR },
 { "mail", LOG_MAIL },
 { "news", LOG_NEWS },
 { "syslog", LOG_SYSLOG },
 { "user", LOG_USER },
 { "uucp", LOG_UUCP },
 */

SysLogEngine::SysLogEngine(string _programName, uint32_t facilities)
: m_facilities(facilities)
{
#ifndef _WIN32
    m_objectCounter++;
#else
    m_logHandle = 0;
#endif
    programName(_programName);
}

void SysLogEngine::saveMessage(const DateTime& date, const char* message, uint32_t sz, LogPriority priority)
{
    SYNCHRONIZED_CODE;
    if (m_options & LO_ENABLE) {
#ifndef _WIN32
        if (!m_logOpened)
            openlog(m_programName.c_str(), LOG_NOWAIT, LOG_USER | LOG_INFO);
        syslog(int(m_facilities | priority), "[%s] %s", priorityName(priority).c_str(), message);
#else
        if (!m_logHandle) {
            OSVERSIONINFO version;
            version.dwOSVersionInfoSize = sizeof(version);
            if (!GetVersionEx(&version))
                throw Exception("Can't determine Windows version");
            if (version.dwPlatformId != VER_PLATFORM_WIN32_NT)
                throw Exception("EventLog is only implemented on NT-based Windows");
            m_logHandle = RegisterEventSource(NULL, m_programName.c_str());
        }
        if (!m_logHandle)
            throw Exception("Can't open Application Event Log");

        unsigned eventType;
        switch (priority) {
            case LOG_EMERG:
            case LOG_ALERT:
            case LOG_CRIT:
            case LOG_ERR:
                eventType = EVENTLOG_ERROR_TYPE;
                break;
            case LOG_WARNING:
                eventType = EVENTLOG_WARNING_TYPE;
                break;
            default:
                eventType = EVENTLOG_INFORMATION_TYPE;
                break;
        }

        //const char *messageStrings[] = { message, NULL };
        LPCTSTR messageStrings[]= {TEXT(message)};

        if (!ReportEvent(
                        m_logHandle,    // handle returned by RegisterEventSource
                        eventType,		// event type to log
                        0,				// event category
						MSG_INFO_1,		// event identifier
                        NULL,			// user security identifier (optional)
                        1,				// number of strings to merge with message
                        0,				// size of binary data, in bytes
                        messageStrings,	// array of strings to merge with message
                        NULL			// address of binary data
                ))
        {
            throw Exception("Can't write an event to Application Event Log ");
        }
#endif
    }

    if (m_options & LO_STDOUT) {
        string messagePrefix;
        if (m_options & LO_DATE)
            messagePrefix += date.dateString() + " ";

        if (m_options & LO_TIME)
            messagePrefix += date.timeString(true) + " ";

        if (m_options & LO_PRIORITY)
            messagePrefix += "[" + priorityName(priority) + "] ";

        cout << messagePrefix + message + "\n";
    }
}

SysLogEngine::~SysLogEngine()
{
#ifndef _WIN32
    m_objectCounter--;
    if (m_logOpened && m_objectCounter < 1)
        closelog();
#else
    if (m_logHandle)
        CloseEventLog(m_logHandle);
#endif
}

void SysLogEngine::programName(string progName)
{
    m_programName = progName;
#ifndef _WIN32
    m_logOpened = false;
    closelog();
#else
    char exe_path[_MAX_PATH];
    GetModuleFileName(0, exe_path,_MAX_PATH);
    m_moduleFileName = string(exe_path);

	std::string value;
	if (!m_registrySet) {
        string key_path = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\SPTK Event Provider";

		HKEY key;

		DWORD last_error = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			key_path.c_str(),
			0,
			0,
			REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE,
			0,
			&key,
			0);

		if (ERROR_SUCCESS == last_error)
		{
			DWORD last_error;
			const DWORD types_supported = EVENTLOG_ERROR_TYPE |
				EVENTLOG_WARNING_TYPE |
				EVENTLOG_INFORMATION_TYPE;

			last_error = RegSetValueEx(key,
				"EventMessageFile",
				0,
				REG_SZ,
				(BYTE*)exe_path,
				(DWORD)strlen(exe_path));

			if (ERROR_SUCCESS == last_error)
			{
				last_error = RegSetValueEx(key,
					"TypesSupported",
					0,
					REG_DWORD,
					(LPBYTE)&types_supported,
					sizeof(types_supported));
			}

			if (ERROR_SUCCESS != last_error)
			{
				std::cerr << "Failed to install source values: "
					<< last_error << "\n";
			}

			RegCloseKey(key);
		}
		else
		{
			std::cerr << "Failed to install source: " << last_error << "\n";
		}

        m_registrySet = true;
    }
#endif
}
