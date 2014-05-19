/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSysLog.cpp  -  description
                             -------------------
    begin                : Tue Jan 31 2006
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.

    This module creation was sponsored by Total Knowledge (http://www.total-knowledge.com).
    Author thanks the developers of CPPSERV project (http://www.total-knowledge.com/progs/cppserv)
    for defining the requirements for this class.
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/CSysLog.h>

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

static string       m_programName;

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

CSysLog::CSysLog(uint32_t facilities) :
        m_facilities(facilities)
{
#ifndef _WIN32
    m_objectCounter++;
#else

    m_logHandle = 0;
#endif
}

void CSysLog::saveMessage(CDateTime date, const char *message, uint32_t sz, CLogPriority priority) THROWS_EXCEPTIONS
{
    SYNCHRONIZED_CODE;
    if (m_options & CLO_ENABLE) {
#ifndef _WIN32
        if (!m_logOpened)
            openlog(m_programName.c_str(), LOG_NOWAIT, LOG_USER | LOG_INFO);
        syslog(int(m_facilities | priority), "[%s] %s", priorityName(priority).c_str(), message);
#else

        if (!m_logHandle) {
            OSVERSIONINFO version;
            version.dwOSVersionInfoSize = sizeof(version);
            if (!GetVersionEx(&version))
            throw CException("Can't determine Windows version");
            if (version.dwPlatformId != VER_PLATFORM_WIN32_NT)
            throw CException("EventLog is only implemented on NT-based Windows");
            m_logHandle = RegisterEventSource(0,m_programName.c_str());
        }
        if (!m_logHandle)
        throw CException("Can't open Application Event Log");

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
                        eventType,// event type to log
                        0,// event category
                        SPTK_MESSAGE,// event identifier
                        0,// user security identifier (optional)
                        1,// number of strings to merge with message
                        sz,// size of binary data, in bytes
                        messageStrings,// array of strings to merge with message
                        0// address of binary data
                )) {
            throw CException("Can't write an event to Application Event Log ");
        }
#endif
    }

    if (m_options & CLO_STDOUT) {
        if (m_options & CLO_DATE)
            cout << date.dateString() << " ";

        if (m_options & CLO_TIME)
            cout << date.timeString(true) << " ";

        if (m_options & CLO_PRIORITY)
            cout << "[" << priorityName(priority) << "] ";

        cout << message << endl;
    }
}

CSysLog::~CSysLog()
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

void CSysLog::programName(string progName)
{
    m_programName = progName;
#ifndef _WIN32

    m_logOpened = false;
    closelog();
#else

    char *buffer = new char[_MAX_PATH];
    GetModuleFileName(0,buffer,_MAX_PATH);
    m_moduleFileName = buffer;

    if (!m_registrySet) {
        string keyName = "SYSTEM\\ControlSet001\\Services\\EventLog\\Application\\"+progName;

        HKEY keyHandle;
        if (RegCreateKey(
                        HKEY_LOCAL_MACHINE,
                        ("SYSTEM\\ControlSet001\\Services\\EventLog\\Application\\"+progName).c_str(),
                        &keyHandle) != ERROR_SUCCESS)
        throw CException("Can't open registry (HKEY_LOCAL_MACHINE) for write");

        unsigned long len = _MAX_PATH;
        unsigned long vtype = REG_EXPAND_SZ;
        if (RegQueryValueEx(
                        keyHandle,       // handle to key to query
                        "EventMessageFile",
                        0,
                        &vtype,
                        (BYTE *)buffer,// buffer for returned string
                        &len// receives size of returned string
                ) != ERROR_SUCCESS)
        throw CException("Can't open registry (HKEY_LOCAL_MACHINE) for write");

        if (strcmp(buffer,m_moduleFileName.c_str()) != 0) {

            if (RegSetValueEx(
                            keyHandle,          // handle to key to set value for
                            "EventMessageFile",// name of the value to set
                            0,// reserved
                            REG_EXPAND_SZ,// flag for value type
                            (CONST BYTE *)m_moduleFileName.c_str(),// address of value data
                            DWORD(m_moduleFileName.length()+1)// size of value data
                    ) != ERROR_SUCCESS)
            throw CException("Can't open registry (HKEY_LOCAL_MACHINE) for write");

            unsigned typesSupported = 7;
            if (RegSetValueEx(
                            keyHandle,// handle to key to set value for
                            "TypesSupported",// name of the value to set
                            0,// reserved
                            REG_DWORD,// flag for value type
                            (CONST BYTE *)&typesSupported,// address of value data
                            sizeof(typesSupported)// size of value data
                    ) != ERROR_SUCCESS)
            throw CException("Can't open registry (HKEY_LOCAL_MACHINE) for write");
            RegCloseKey(keyHandle);
        }
        m_registrySet = true;
    }
#endif
}
