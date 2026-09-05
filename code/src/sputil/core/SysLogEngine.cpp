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

#include <sptk5/SysLogEngine.h>

#include <sptk5/Exception.h>

#ifdef _WIN32
#include <events.w32/event_provider.h>
#include <array>
#include <memory>
#include <sstream> // libc++
#endif

using namespace std;
using namespace sptk;

std::mutex  SysLogEngine::m_syslogMutex;
atomic_bool SysLogEngine::m_logOpened(false);

#ifdef _WIN32
namespace {
/**
 * Any address within this shared library, used to find the module that carries
 * the event message table resource.
 */
const char moduleAnchor {0};

/**
 * Highest syslog facility code (LOG_LOCAL7), and the number of event categories
 * defined for it in events.w32/event_provider.mc.
 */
constexpr uint32_t maxSyslogFacility = LOG_LOCAL7 >> 3;
constexpr DWORD    syslogFacilityCount = maxSyslogFacility + 1;

// The category is computed from the facility rather than looked up, so pin the
// message table to the facility codes it's generated from.
static_assert(SPTK_CATEGORY_KERN == (LOG_KERN >> 3) + 1);
static_assert(SPTK_CATEGORY_USER == (LOG_USER >> 3) + 1);
static_assert(SPTK_CATEGORY_AUTH == (LOG_AUTH >> 3) + 1);
static_assert(SPTK_CATEGORY_LOCAL7 == (LOG_LOCAL7 >> 3) + 1);
} // namespace
#endif

SysLogEngine::SysLogEngine(const String& _programName, const uint32_t facilities)
    : LogEngine("SysLogEngine")
    , m_facilities(facilities)
{
    programName(_programName);
}

bool SysLogEngine::saveMessage(const Logger::Message& message)
{
    set<Option> options;
    String      programName;
    uint32_t    facilities {0};

    getOptions(options, programName, facilities);

    if (options.contains(Option::ENABLE))
    {
#ifndef _WIN32
        const scoped_lock lock(m_syslogMutex);
        if (!m_logOpened)
        {
            openlog(programName.c_str(), LOG_NOWAIT, static_cast<int>(facilities));
            m_logOpened = true;
        }
        // openlog() registers one default facility for the whole process, but syslog() takes
        // facility|priority, so each engine's own facility is applied per message.
        syslog(static_cast<int>(facilities) | static_cast<int>(message.priority), "[%s] %s",
               priorityName(message.priority).c_str(), message.message.c_str());
#else
        if (m_logHandle.load() == nullptr)
        {
            OSVERSIONINFO version;
            version.dwOSVersionInfoSize = sizeof(version);
            if (!GetVersionEx(&version))
            {
                throw Exception("Can't determine Windows version");
            }
            if (version.dwPlatformId != VER_PLATFORM_WIN32_NT)
            {
                throw Exception("EventLog is only implemented on NT-based Windows");
            }
            m_logHandle = RegisterEventSource(nullptr, programName.c_str());
        }
        if (m_logHandle.load() == nullptr)
        {
            throw Exception("Can't open Application Event Log");
        }

        // Event categories mirror the syslog facility codes (see events.w32/event_provider.mc):
        // category N is facility N-1, so the same facility groups messages the same way on
        // every platform. Category 0 tells Event Viewer that the event has no category.
        const uint32_t facilityCode = facilities >> 3;
        const auto     category = facilityCode <= maxSyslogFacility ? static_cast<WORD>(facilityCode + 1) : WORD {0};

        WORD eventType;
        switch ((int) message.priority)
        {
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
        LPCTSTR messageStrings[] = {TEXT(message.message.c_str())};

        if (!ReportEvent(
                m_logHandle,       // handle returned by RegisterEventSource
                eventType,         // event type to log
                category,          // event category
                MSG_TEXT,          // event identifier
                NULL,              // user security identifier (optional)
                1,                 // number of strings to merge with message
                0,                 // size of binary data, in bytes
                messageStrings,    // array of strings to merge with message
                NULL               // address of binary data
                ))
        {
            throw Exception("Can't write an event to Application Event Log ");
        }
#endif
    }
    return true;
}

void SysLogEngine::getOptions(set<Option>& options, String& programName, uint32_t& facilities) const
{
    const scoped_lock lock(m_syslogMutex);
    options = this->options();
    programName = m_programName;
    facilities = m_facilities;
}

SysLogEngine::~SysLogEngine()
{
#ifdef _WIN32
    if (m_logHandle)
        CloseEventLog(m_logHandle);
#endif
    shutdown();
}

void SysLogEngine::setupEventSource() const
{
    const scoped_lock lock(m_syslogMutex);
#ifndef _WIN32
    m_logOpened = false;
    closelog();
#else
    // Event Viewer formats a message by loading the message table resource from the module
    // named in EventMessageFile. That table (events.w32/event_provider.rc) is linked into
    // this library, so register the library itself rather than the host executable -
    // applications using SPTK don't have to embed the resource in their own binaries.
    HMODULE thisModule {nullptr};
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(&moduleAnchor), &thisModule))
    {
        throw Exception("Can't determine SPTK module handle");
    }

    // One extra byte keeps the buffer NUL terminated even for a registry value that isn't
    constexpr DWORD                 pathBufferSize = MAX_PATH;
    array<char, pathBufferSize + 1> buffer {};

    const auto moduleFileNameLength = GetModuleFileName(thisModule, buffer.data(), pathBufferSize);
    if (moduleFileNameLength == 0 || moduleFileNameLength >= pathBufferSize)
    {
        throw Exception("Can't determine SPTK module file name");
    }
    const String moduleFileName(buffer.data(), moduleFileNameLength);

    const String keyName = R"(SYSTEM\CurrentControlSet\Services\EventLog\Application\)" + m_programName;

    HKEY keyHandle {nullptr};
    auto rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE, keyName.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE,
                             KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &keyHandle, nullptr);
    if (rc == ERROR_ACCESS_DENIED)
    {
        // Registering an event source is a machine-wide operation that requires administrator
        // rights. Without it messages still reach the Application log, they just aren't
        // formatted by Event Viewer, so this isn't worth failing the application over.
        return;
    }
    if (rc != ERROR_SUCCESS)
    {
        throw Exception("Can't create registry key HKEY_LOCAL_MACHINE '" + keyName + "'");
    }

    const auto closeKey = [](HKEY handle)
    {
        RegCloseKey(handle);
    };
    const unique_ptr<remove_pointer_t<HKEY>, decltype(closeKey)> key(keyHandle, closeKey);

    // Re-register whenever the recorded module path is missing or stale, so that moving or
    // upgrading the library doesn't leave the event source pointing at the old file.
    DWORD valueSize = pathBufferSize;
    DWORD valueType = REG_EXPAND_SZ;
    rc = RegQueryValueEx(keyHandle, "EventMessageFile", nullptr, &valueType, reinterpret_cast<BYTE*>(buffer.data()), &valueSize);
    if (rc == ERROR_SUCCESS && String(buffer.data()) == moduleFileName)
    {
        return;
    }

    struct ValueData
    {
        const char* name;
        const char* strValue;
        DWORD       intValue;
    };

    const array<ValueData, 5> valueData {
        ValueData {"CategoryCount", nullptr, syslogFacilityCount},
        ValueData {"CategoryMessageFile", moduleFileName.c_str(), 0},
        ValueData {"EventMessageFile", moduleFileName.c_str(), 0},
        ValueData {"ParameterMessageFile", moduleFileName.c_str(), 0},
        ValueData {"TypesSupported", nullptr, 7}};

    for (const auto& item: valueData)
    {
        const BYTE* value;
        if (item.strValue == nullptr)
        {
            // DWORD value
            value = reinterpret_cast<const BYTE*>(&item.intValue);
            valueSize = sizeof(item.intValue);
            valueType = REG_DWORD;
        }
        else
        {
            // String value
            value = reinterpret_cast<const BYTE*>(item.strValue);
            valueSize = static_cast<DWORD>(strlen(item.strValue)) + 1;
            valueType = REG_EXPAND_SZ;
        }

        rc = RegSetValueEx(
            keyHandle,  // handle to key to set value for
            item.name,  // name of the value to set
            0,          // reserved
            valueType,  // flag for value type
            value,      // address of value data
            valueSize   // size of value data
        );

        if (rc != ERROR_SUCCESS)
        {
            stringstream error;
            error << "Can't set registry key HKEY_LOCAL_MACHINE '" << keyName << "' ";
            error << "value '" << item.name << "' to ";
            if (item.strValue == nullptr)
            {
                error << "REG_DWORD " << item.intValue;
            }
            else
            {
                error << "REG_EXPAND_SZ " << item.strValue;
            }
            throw Exception(error.str());
        }
    }
#endif
}

void SysLogEngine::programName(const String& progName)
{
    m_programName = progName;
    setupEventSource();
}