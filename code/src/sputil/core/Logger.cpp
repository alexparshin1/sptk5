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

#include <sptk5/LogEngine.h>

using namespace std;
using namespace sptk;

Logger::Message::Message(LogPriority priority, String message)
    : priority(priority)
    , message(std::move(message))
{
}

Logger::Logger(LogEngine& destination, String prefix)
    : m_destination(destination)
    , m_prefix(std::move(prefix))
{
}

void Logger::log(LogPriority priority, const String& message)
{
    auto msg = make_unique<Message>(priority, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::debug(const String& message)
{
    auto msg = make_unique<Message>(LogPriority::DEBUG, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::info(const String& message)
{
    auto msg = make_unique<Message>(LogPriority::INFO, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::notice(const String& message)
{
    auto msg = make_unique<Message>(LogPriority::NOTICE, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::warning(const String& message)
{
    auto msg = make_unique<Message>(LogPriority::WARNING, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::error(const String& message)
{
    auto msg = make_unique<Message>(LogPriority::ERR, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::critical(const String& message)
{
    auto msg = make_unique<Message>(LogPriority::CRITICAL, m_prefix + message);
    m_destination.log(std::move(msg));
}
