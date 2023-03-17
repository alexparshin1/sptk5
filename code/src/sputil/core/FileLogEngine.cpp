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

#include <sptk5/FileLogEngine.h>
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

FileLogEngine::~FileLogEngine()
{
    terminate();
}

void FileLogEngine::saveMessage(const Logger::Message& message)
{
    const auto options = this->options();

    const scoped_lock lock(masterLock());

    if (options.contains(Option::ENABLE) && !terminated())
    {
        if (!m_fileStream.is_open())
        {
            m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::app);
            if (!m_fileStream.is_open())
            {
                throw Exception("Can't append or create log file '" + m_fileName.string() + "'");
            }
        }

        if (options.contains(Option::DATE))
        {
            m_fileStream << message.timestamp.dateString() << " ";
        }

        if (options.contains(Option::TIME))
        {
            auto printAccuracy = options.contains(Option::MILLISECONDS) ? DateTime::PrintAccuracy::MILLISECONDS : DateTime::PrintAccuracy::SECONDS;
            m_fileStream << message.timestamp.timeString(true, printAccuracy) << " ";
        }

        if (options.contains(Option::PRIORITY))
        {
            m_fileStream << "[" << priorityName(message.priority) << "] ";
        }

        m_fileStream << message.message << endl;

        if (m_fileStream.bad() && !terminated())
        {
            CERR("Can't write to file " << m_fileName.c_str() << endl);
        }
    }
}

FileLogEngine::FileLogEngine(const filesystem::path& fileName)
    : LogEngine("FileLogEngine")
    , m_fileName(fileName)
    , m_fileStream(fileName.c_str())
{
}

void FileLogEngine::reset()
{
    const scoped_lock lock(masterLock());

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
}

void FileLogEngine::close()
{
    const scoped_lock lock(masterLock());
    if (m_fileStream.is_open())
    {
        m_fileStream.flush();
        m_fileStream.close();
    }
}
