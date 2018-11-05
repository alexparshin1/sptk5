/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       FileLogEngine.cpp - description                        ║
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

#include <sptk5/FileLogEngine.h>

using namespace std;
using namespace sptk;

void FileLogEngine::saveMessage(const Logger::Message* message)
{
    UniqueLock(m_mutex);
    if ((m_options & LO_ENABLE) == LO_ENABLE) {
        if (!m_fileStream.is_open()) {
            m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::app);
            if (!m_fileStream.is_open())
                throw Exception("Can't append or create log file '" + m_fileName + "'", __FILE__, __LINE__);
        }

        if ((m_options & LO_DATE) == LO_DATE)
            m_fileStream << message->timestamp.dateString() << " ";

        if ((m_options & LO_TIME) == LO_TIME)
            m_fileStream << message->timestamp.timeString(true) << " ";

        if ((m_options & LO_PRIORITY) == LO_PRIORITY)
            m_fileStream << "[" << priorityName(message->priority) << "] ";

        m_fileStream << message << endl;
    }

	if (m_fileStream.bad())
        throw Exception("Can't write to log file '" + m_fileName + "'", __FILE__, __LINE__);
}

FileLogEngine::FileLogEngine(const String& fileName)
: LogEngine("FileLogEngine"),
  m_fileName(fileName)
{}

FileLogEngine::~FileLogEngine()
{
    if (m_fileStream.is_open())
        m_fileStream.close();
}

void FileLogEngine::reset()
{
    UniqueLock(m_mutex);
    if (m_fileStream.is_open())
        m_fileStream.close();
    if (m_fileName.empty())
        throw Exception("File name isn't defined", __FILE__, __LINE__);
    m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::trunc);
    if (!m_fileStream.is_open())
        throw Exception("Can't open log file '" + m_fileName + "'", __FILE__, __LINE__);
}
