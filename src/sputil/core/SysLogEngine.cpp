/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SysLogEngine.cpp - description                         ║
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

#include <sptk5/SysLogEngine.h>

using namespace std;
using namespace sptk;

static SharedMutex	    syslogMutex;
static atomic_bool      m_logOpened(false);
#ifndef _WIN32
    static atomic_int   m_objectCounter(0);
#else
    #include <events.w32/event_provider.h>
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

SysLogEngine::SysLogEngine(const string& _programName, uint32_t facilities)
: LogEngine("SysLogEngine"),
  m_facilities(facilities)
{
#ifndef _WIN32
    m_objectCounter++;
#else
    m_logHandle = 0;
#endif
    programName(_programName);
}

void SysLogEngine::saveMessage(const Logger::Message* message)
{
    uint32_t    options;
    string      programName;
    uint32_t    facilities;

    {
        SharedLock(syslogMutex);
        options = (uint32_t) m_options;
        programName = m_programName;
        facilities = m_facilities;
    }

    if (options & LO_ENABLE) {
#ifndef _WIN32
        UniqueLock(m_mutex);
        if (!m_logOpened)
            openlog(programName.c_str(), LOG_NOWAIT, LOG_USER | LOG_INFO);
        syslog(int(facilities | message->priority), "[%s] %s", priorityName(message->priority).c_str(), message->message.c_str());
#else
        if (m_logHandle.load() == nullptr) {
            OSVERSIONINFO version;
            version.dwOSVersionInfoSize = sizeof(version);
            if (!GetVersionEx(&version))
                throw Exception("Can't determine Windows version");
            if (version.dwPlatformId != VER_PLATFORM_WIN32_NT)
                throw Exception("EventLog is only implemented on NT-based Windows");
            m_logHandle = RegisterEventSource(NULL, programName.c_str());
        }
        if (m_logHandle.load() == nullptr)
            throw Exception("Can't open Application Event Log");

        WORD eventType;
        switch (message->priority) {
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
        LPCTSTR messageStrings[]= {TEXT(message->message.c_str())};

        if (!ReportEvent(
                        m_logHandle,    // handle returned by RegisterEventSource
                        eventType,		// event type to log
                        SPTK_MSG_CATEGORY, // event category
                        MSG_TEXT,		// event identifier
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
}

SysLogEngine::~SysLogEngine()
{
#ifndef _WIN32
	bool needToClose = false;
	{
		UniqueLock(syslogMutex);
		m_objectCounter--;
		if (m_logOpened && m_objectCounter < 1)
			needToClose = true;
	}

	if (needToClose)
        closelog();
#else
    if (m_logHandle)
        CloseEventLog(m_logHandle);
#endif
}

void SysLogEngine::setupEventSource()
{
    UniqueLock(syslogMutex);
#ifndef _WIN32
    m_logOpened = false;
    closelog();
#else
    char *buffer = new char[_MAX_PATH];
    GetModuleFileName(0, buffer, _MAX_PATH);
    string moduleFileName = buffer;

    string keyName = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + m_programName;

    HKEY keyHandle;
    if (RegCreateKey(HKEY_LOCAL_MACHINE, keyName.c_str(), &keyHandle) != ERROR_SUCCESS)
        throw runtime_error("Can't create registry key HKEY_LOCAL_MACHINE '" + keyName + "'");

    unsigned long len = _MAX_PATH;
    unsigned long vtype = REG_EXPAND_SZ;
    int rc = RegQueryValueEx(keyHandle, "EventMessageFile", 0, &vtype, (BYTE*)buffer, &len);
    if (rc != ERROR_SUCCESS) {

        struct ValueData {
            const char* name;
            const char* strValue;
            DWORD       intValue;
        } valueData[5] = {
            { "CategoryCount", NULL, 3 },
            { "CategoryMessageFile", moduleFileName.c_str(), 0 },
            { "EventMessageFile", moduleFileName.c_str(), 0 },
            { "ParameterMessageFile", moduleFileName.c_str(), 0 },
            { "TypesSupported", NULL, 7 }
        };

        for (int i = 0; i < 5; i++) {
            CONST BYTE * value;
            DWORD valueSize;
            DWORD valueType;
            if (valueData[i].strValue == NULL) {
                // DWORD value
                value = (CONST BYTE *) &(valueData[i].intValue);
                valueSize = sizeof(valueData[i].intValue);
                valueType = REG_DWORD;
            }
            else {
                // String value
                value = (CONST BYTE *) valueData[i].strValue;
                valueSize = (DWORD)strlen(valueData[i].strValue) + 1;
                valueType = REG_EXPAND_SZ;
            }
            rc = RegSetValueEx(
                keyHandle,          // handle to key to set value for
                valueData[i].name,  // name of the value to set
                0,                  // reserved
                valueType,          // flag for value type
                value,              // address of value data
                valueSize           // size of value data
            );

            if (rc != ERROR_SUCCESS) {
                stringstream error;
                error << "Can't set registry key HKEY_LOCAL_MACHINE '" << keyName << "' ";
                error << "value '" << valueData[i].name << "' to ";
                if (valueData[i].strValue == NULL)
                    error << "REG_DWORD " << valueData[i].intValue;
                else
                    error << "REG_SZ " << valueData[i].strValue;
                throw runtime_error(error.str());
            }
        }
    }
    RegCloseKey(keyHandle);
#endif
}

void SysLogEngine::programName(const string& progName)
{
    m_programName = progName;
    setupEventSource();
}
