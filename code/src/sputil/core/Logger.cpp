/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-15                                             ║
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

#include "sptk5/Logger.h"
#include <sptk5/LogEngine.h>


using namespace std;
using namespace sptk;

Logger::Message::Message(const LogPriority priority, std::string message)
    : priority(priority)
    , message(std::move(message))
{
}

Logger::Logger(LogEngine& destination, const std::string_view prefix)
    : m_destination(destination)
    , m_prefix(prefix)
{
}

void Logger::log(LogPriority priority, const string& message) const
{
    auto msg = make_unique<Message>(priority, format("{}{}", m_prefix, message));
    m_destination.log(std::move(msg));
}

void Logger::log(LogPriority priority, const OutputString& output) const
{
    auto msg = make_unique<Message>(priority, prefix() + output());
    m_destination.log(std::move(msg));
}

void Logger::debug(const string& message) const
{
    auto msg = make_unique<Message>(LogPriority::Debug, prefix() + message);
    m_destination.log(std::move(msg));
}

void Logger::debug(const OutputString& output) const
{
    if (m_destination.minPriority() >= LogPriority::Debug)
    {
        auto msg = make_unique<Message>(LogPriority::Debug, prefix() + output());
        m_destination.log(std::move(msg));
    }
}

void Logger::info(const string& message) const
{
    auto msg = make_unique<Message>(LogPriority::Info, prefix() + message);
    m_destination.log(std::move(msg));
}

void Logger::info(const OutputString& output) const
{
    if (m_destination.minPriority() >= LogPriority::Info)
    {
        auto msg = make_unique<Message>(LogPriority::Info, prefix() + output());
        m_destination.log(std::move(msg));
    }
}

void Logger::notice(const string& message) const
{
    if (m_destination.minPriority() >= LogPriority::Notice)
    {
        auto msg = make_unique<Message>(LogPriority::Notice, prefix() + message);
        m_destination.log(std::move(msg));
    }
}

void Logger::notice(const OutputString& output) const
{
    if (m_destination.minPriority() >= LogPriority::Notice)
    {
        auto msg = make_unique<Message>(LogPriority::Notice, prefix() + output());
        m_destination.log(std::move(msg));
    }
}

void Logger::warning(const string& message) const
{
    if (m_destination.minPriority() >= LogPriority::Warning)
    {
        auto msg = make_unique<Message>(LogPriority::Warning, prefix() + message);
        m_destination.log(std::move(msg));
    }
}

void Logger::warning(const OutputString& output) const
{
    if (m_destination.minPriority() >= LogPriority::Warning)
    {
        auto msg = make_unique<Message>(LogPriority::Warning, prefix() + output());
        m_destination.log(std::move(msg));
    }
}

void Logger::error(const string& message) const
{
    if (m_destination.minPriority() >= LogPriority::Error)
    {
        auto msg = make_unique<Message>(LogPriority::Error, prefix() + message);
        m_destination.log(std::move(msg));
    }
}

void Logger::error(const OutputString& output) const
{
    if (m_destination.minPriority() >= LogPriority::Error)
    {
        auto msg = make_unique<Message>(LogPriority::Error, prefix() + output());
        m_destination.log(std::move(msg));
    }
}

void Logger::critical(const string& message) const
{
    if (m_destination.minPriority() >= LogPriority::Critical)
    {
        auto msg = make_unique<Message>(LogPriority::Critical, prefix() + message);
        m_destination.log(std::move(msg));
    }
}

void Logger::critical(const OutputString& output) const
{
    if (m_destination.minPriority() >= LogPriority::Critical)
    {
        auto msg = make_unique<Message>(LogPriority::Critical, prefix() + output());
        m_destination.log(std::move(msg));
    }
}

bool Logger::has(const LogPriority logPriority) const
{
    return m_destination.minPriority() >= logPriority;
}
