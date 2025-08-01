/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Logger.cpp - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/LogEngine.h>

using namespace std;
using namespace sptk;

Logger::Message::Message(LogPriority priority, const String& message)
: timestamp(DateTime::Now()), priority(priority), message(message)
{}

Logger::Logger(LogEngine& destination, const String& prefix)
: m_destination(destination), m_prefix(prefix)
{
}

void Logger::log(LogPriority priority, const String& message)
{
    m_destination.log(new Message(priority, m_prefix + message));
}

void Logger::debug(const String& message)
{
    m_destination.log(new Message(LP_DEBUG, m_prefix + message));
}

void Logger::info(const String& message)
{
    m_destination.log(new Message(LP_INFO, m_prefix + message));
}

void Logger::notice(const String& message)
{
    m_destination.log(new Message(LP_NOTICE, m_prefix + message));
}

void Logger::warning(const String& message)
{
    m_destination.log(new Message(LP_WARNING, m_prefix + message));
}

void Logger::error(const String& message)
{
    m_destination.log(new Message(LP_ERROR, m_prefix + message));
}

void Logger::critical(const String& message)
{
    m_destination.log(new Message(LP_CRITICAL, m_prefix + message));
}
