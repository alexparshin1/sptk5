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
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

LogEngine::LogEngine(const String&)
{
    m_saveMessageThread = jthread([this](stop_token stopToken) {
        try
        {
            threadFunction();
        }
        catch (const Exception& exception)
        {
            CERR(exception.what() << endl);
        }
    });
}

LogEngine::~LogEngine()
{
    shutdown();
}

void LogEngine::shutdown()
{
    if (!terminated())
    {
        terminate();
    }
}

void LogEngine::option(Option option, bool flag)
{
    const std::scoped_lock lock(m_mutex);
    if (flag)
    {
        m_options.insert(option);
    }
    else
    {
        m_options.erase(option);
    }
}

bool LogEngine::option(Option option) const
{
    const std::scoped_lock lock(m_mutex);
    return m_options.contains(option);
}

String LogEngine::priorityName(LogPriority prt)
{
    switch (prt)
    {
        case LogPriority::DEBUG:
            return "DEBUG";
        case LogPriority::INFO:
            return "INFO";
        case LogPriority::NOTICE:
            return "NOTICE";
        case LogPriority::WARNING:
            return "WARNING";
        case LogPriority::ERR:
            return "ERROR";
        case LogPriority::CRITICAL:
            return "CRITICAL";
        case LogPriority::ALERT:
            return "ALERT";
        default:
            return "PANIC";
    }
}

LogPriority LogEngine::priorityFromName(const String& prt)
{
    static const Strings priorityNames("DEBUG|INFO|NOTICE|WARNING|ERROR|CRITICAL|ALERT|PANIC", "|");

    switch (priorityNames.indexOf(prt.toUpperCase()))
    {
        case 0:
            return LogPriority::DEBUG;
        case 1:
            return LogPriority::INFO;
        case 2:
            return LogPriority::NOTICE;
        case 3:
            return LogPriority::WARNING;
        case 4:
            return LogPriority::ERR;
        case 5:
            return LogPriority::CRITICAL;
        case 6:
            return LogPriority::ALERT;
        case 7:
            return LogPriority::PANIC;
        default:
            return LogPriority::DEBUG;
    }
}

void LogEngine::log(const Logger::UMessage& message)
{
    if (terminated())
    {
        return;
    }

    if (m_minPriority >= message->priority)
    {
        m_messages.push(message);
    }
}

void LogEngine::threadFunction()
{
    const chrono::seconds timeout(1);
    while (!terminated())
    {

        Logger::UMessage message = nullptr;
        if (!m_messages.pop(message, timeout))
        {
            continue;
        }

        saveMessage(message);

        if (option(Option::STDOUT))
        {
            string messagePrefix;
            if (option(Option::DATE))
            {
                messagePrefix += message->timestamp.dateString() + " ";
            }

            if (option(Option::TIME))
            {
                auto printAccuracy = option(Option::MILLISECONDS) ? DateTime::PrintAccuracy::MILLISECONDS : DateTime::PrintAccuracy::SECONDS;
                messagePrefix += message->timestamp.timeString(true, printAccuracy) + " ";
            }

            if (option(Option::PRIORITY))
            {
                messagePrefix += "[" + priorityName(message->priority) + "] ";
            }

            FILE* dest = stdout;
            if (message->priority <= LogPriority::ERR)
            {
                dest = stderr;
            }
            fprintf(dest, "%s%s\n", messagePrefix.c_str(), message->message.c_str());
            fflush(dest);
        }
    }

    try
    {
        close();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << std::endl);
    }
}
