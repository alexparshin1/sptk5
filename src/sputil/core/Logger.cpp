/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Logger.cpp - description                               ║
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

#include <sptk5/Logger.h>

using namespace std;
using namespace sptk;

CLogStreamBuf::CLogStreamBuf()
{
    m_parent = nullptr;
    m_bytes = 0;
    m_size = 1024;
    m_buffer = (char *) malloc(m_size);
    m_priority = LP_NOTICE;
    m_date = DateTime::Now();
}

streambuf::int_type CLogStreamBuf::overflow(streambuf::int_type c)
{
    //SYNCHRONIZED_CODE;
    
    bool bufferOverflow = m_bytes > m_size - 2;
    bool lineBreak = c <= 13;
    
    if (lineBreak || bufferOverflow) {
        if (m_bytes != 0) {
            m_buffer[m_bytes] = 0;
            if (m_parent != nullptr) {
                if (m_priority <= m_parent->m_destination.minPriority())
                    m_parent->saveMessage(m_date, m_buffer, m_bytes, m_priority);
                if (!bufferOverflow) {
                    m_priority = m_parent->m_destination.defaultPriority();
                    m_date = DateTime::Now();
                }
            }
            m_bytes = 0;
        }
    }

    if (!lineBreak) {
        if (m_bytes == 0)
            m_date = DateTime::Now();
        m_buffer[m_bytes] = (char) c;
        m_bytes++;
    }

    return traits_type::not_eof(c);
}
//==========================================================================================

Logger::Logger(LogEngine& destination)
: _ios(nullptr), _ostream((m_buffer = new CLogStreamBuf)), m_destination(destination)
{
    m_buffer->parent(this);
}

Logger::~Logger()
{
    flush();
    delete m_buffer;
}

void Logger::saveMessage(const DateTime& date, const char* message, uint32_t sz, LogPriority priority)
{
    m_destination.saveMessage(date, message, sz, priority);
}

SP_EXPORT Logger& sptk::operator <<(Logger& stream, LogPriority priority)
{
    if (stream.fail() || stream.bad())
        stream.clear();
    stream.messagePriority(priority);
    return stream;
}
