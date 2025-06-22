/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/Logger.h"
#include <sptk5/LogEngine.h>


using namespace std;
using namespace sptk;

Logger::Message::Message(LogPriority priority, String message)
    : priority(priority)
    , message(std::move(message))
{
}

Logger::Logger(LogEngine& destination, std::string_view prefix)
    : m_destination(destination)
    , m_prefix(prefix)
{
}

void Logger::log(LogPriority priority, const String& message) const
{
    auto msg = make_unique<Message>(priority, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::log(LogPriority priority, const OutputString& output) const
{
    auto msg = make_unique<Message>(priority, m_prefix + output());
    m_destination.log(std::move(msg));
}

void Logger::debug(const String& message) const
{
    auto msg = make_unique<Message>(LogPriority::Debug, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::debug(const OutputString& output) const
{
    auto msg = make_unique<Message>(LogPriority::Debug, m_prefix + output());
    m_destination.log(std::move(msg));
}

void Logger::info(const String& message) const
{
    auto msg = make_unique<Message>(LogPriority::Info, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::info(const OutputString& output) const
{
    auto msg = make_unique<Message>(LogPriority::Info, m_prefix + output());
    m_destination.log(std::move(msg));
}

void Logger::notice(const String& message) const
{
    auto msg = make_unique<Message>(LogPriority::Notice, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::notice(const OutputString& output) const
{
    auto msg = make_unique<Message>(LogPriority::Notice, m_prefix + output());
    m_destination.log(std::move(msg));
}

void Logger::warning(const String& message) const
{
    auto msg = make_unique<Message>(LogPriority::Warning, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::warning(const OutputString& output) const
{
    auto msg = make_unique<Message>(LogPriority::Warning, m_prefix + output());
    m_destination.log(std::move(msg));
}

void Logger::error(const String& message) const
{
    auto msg = make_unique<Message>(LogPriority::Error, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::error(const OutputString& output) const
{
    auto msg = make_unique<Message>(LogPriority::Error, m_prefix + output());
    m_destination.log(std::move(msg));
}

void Logger::critical(const String& message) const
{
    auto msg = make_unique<Message>(LogPriority::Critical, m_prefix + message);
    m_destination.log(std::move(msg));
}

void Logger::critical(const OutputString& output) const
{
    auto msg = make_unique<Message>(LogPriority::Critical, m_prefix + output());
    m_destination.log(std::move(msg));
}

bool Logger::has(LogPriority logPriority) const
{
    return m_destination.minPriority() >= logPriority;
}
