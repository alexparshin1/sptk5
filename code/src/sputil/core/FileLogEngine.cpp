/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-11                                             ║
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

#include <sptk5/FileLogEngine.h>

#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

FileLogEngine::~FileLogEngine()
{
    terminate();
}

bool FileLogEngine::saveMessage(const Logger::Message& message)
{
    const auto options = this->options();

    const lock_guard lock(masterLock());

    if (options.contains(Option::ENABLE))
    {
        if (!m_fileStream.is_open())
        {
            m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::app);
            if (!m_fileStream.is_open())
            {
                CERR("Can't append or create log file '" << m_fileName.string() << "'");
                return false;
            }
        }

        if (options.contains(Option::DATE))
        {
            m_fileStream << message.timestamp.dateString() << " ";
        }

        if (options.contains(Option::TIME))
        {
            const auto printAccuracy = options.contains(Option::MILLISECONDS) ? DateTime::PrintAccuracy::MILLISECONDS : DateTime::PrintAccuracy::SECONDS;
            m_fileStream << message.timestamp.timeString(0, printAccuracy) << " ";
        }

        if (options.contains(Option::PRIORITY))
        {
            m_fileStream << "[" << priorityName(message.priority) << "] ";
        }

        m_fileStream << message.message << '\n';

        if (!m_fileStream.good())
        {
            m_fileStream.close();
            CERR(format("Can't write to file '{}'", m_fileName.string()));
            return false;
        }
    }
    return true;
}

FileLogEngine::FileLogEngine(const filesystem::path& fileName, const bool append)
    : LogEngine("FileLogEngine")
    , m_fileName(fileName)
    , m_fileStream(fileName.c_str(), ios_base::out | (append ? ios_base::app : ios_base::trunc))
{
    if (!m_fileStream.is_open())
    {
        throw Exception(format("Can't open log file '{}'", fileName.string()));
    }
}

void FileLogEngine::flush()
{
    const lock_guard lock(masterLock());
    m_fileStream.flush();
}

void FileLogEngine::reset()
{
    const lock_guard lock(masterLock());

    if (m_fileStream.is_open())
    {
        m_fileStream.close();
    }

    if (m_fileName.empty())
    {
        throw Exception("File name isn't defined");
    }

    m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::trunc);
    if (!m_fileStream.is_open())
    {
        throw Exception("Can't open log file '" + m_fileName.string() + "'");
    }

    LogEngine::reset();
}

// The name a log is set aside under: the original with ".YYYYMMDD.HHMM" appended, and a counter
// after that if something is already there. Two rotations within the same minute are unusual but
// entirely possible - a size-triggered rotation of a busy log, or simply a second call - and
// quietly writing over the first one would throw away the very thing being kept.
static filesystem::path archiveName(const filesystem::path& logFile)
{
    short year = 0;
    short month = 0;
    short day = 0;
    short weekDay = 0;
    short yearDay = 0;
    short hour = 0;
    short minute = 0;
    short second = 0;
    short millisecond = 0;

    const auto now = DateTime::Now();
    now.decodeDate(&year, &month, &day, &weekDay, &yearDay);
    now.decodeTime(&hour, &minute, &second, &millisecond);

    const auto stamp = format("{:04}{:02}{:02}.{:02}{:02}", year, month, day, hour, minute);

    auto candidate = logFile;
    candidate += "." + stamp;
    for (unsigned attempt = 1; exists(candidate); ++attempt)
    {
        candidate = logFile;
        candidate += format(".{}-{}", stamp, attempt);
    }
    return candidate;
}

filesystem::path FileLogEngine::rotate()
{
    const lock_guard lock(masterLock());

    if (m_fileName.empty())
    {
        throw Exception("File name isn't defined");
    }

    if (m_fileStream.is_open())
    {
        m_fileStream.flush();
        m_fileStream.close();
    }

    // Nothing written yet, so nothing to keep. The empty file that the constructor made is left
    // alone rather than set aside, which would leave a directory of empty timestamped files behind
    // a service that restarts often.
    error_code errorCode;
    if (!exists(m_fileName, errorCode) || file_size(m_fileName, errorCode) == 0)
    {
        m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::app);
        return {};
    }

    const auto archived = archiveName(m_fileName);
    rename(m_fileName, archived, errorCode);

    // Truncating: after a successful rename the name is free, and if the rename failed the file is
    // still the live log and must be appended to, not emptied.
    const auto mode = errorCode ? (ofstream::out | ofstream::app) : (ofstream::out | ofstream::trunc);
    m_fileStream.open(m_fileName.c_str(), mode);
    if (!m_fileStream.is_open())
    {
        throw Exception(format("Can't open log file '{}'", m_fileName.string()));
    }

    if (errorCode)
    {
        CERR(format("Can't rename log file '{}' to '{}': {}. The log was not rotated.",
                          m_fileName.string(), archived.string(), errorCode.message()));
        return {};
    }

    return archived;
}

void FileLogEngine::close()
{
    const lock_guard lock(masterLock());
    if (m_fileStream.is_open())
    {
        m_fileStream.flush();
        m_fileStream.close();
    }
}
