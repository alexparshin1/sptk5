/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

LogEngine::LogEngine(const String&)
{
    m_saveMessageThread = jthread([this]()
                                  {
                                      try
                                      {
                                          threadFunction();
                                      }
                                      catch (const Exception& exception)
                                      {
                                          CERR(exception.what());
                                      }
                                  });
}

LogEngine::~LogEngine()
{
    {
        const lock_guard lock(m_mutex);
        m_terminated = true;
    }
    m_messages.wakeup();
    if (m_saveMessageThread.joinable())
    {
        m_saveMessageThread.join();
    }
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
    const lock_guard lock(m_mutex);
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
    const lock_guard lock(m_mutex);
    return m_options.contains(option);
}

String LogEngine::priorityName(LogPriority prt)
{
    switch (prt)
    {
        using enum LogPriority;
        case Debug:
            return "DEBUG";
        case Info:
            return "Info";
        case Notice:
            return "NOTICE";
        case Warning:
            return "WARNING";
        case Error:
            return "ERROR";
        case Critical:
            return "Critical";
        case Alert:
            return "Alert";
        default:
            return "Panic";
    }
}

LogPriority LogEngine::priorityFromName(const String& prt)
{
    static const Strings priorityNames("DEBUG|INFO|NOTICE|WARNING|ERROR|CRITICAL|ALERT|PANIC", "|");

    switch (priorityNames.indexOf(prt.toUpperCase()))
    {
        using enum LogPriority;
        case 0:
            return Debug;
        case 1:
            return Info;
        case 2:
            return Notice;
        case 3:
            return Warning;
        case 4:
            return Error;
        case 5:
            return Critical;
        case 6:
            return Alert;
        case 7:
            return Panic;
        default:
            return Debug;
    }
}

void LogEngine::log(Logger::UMessage&& message)
{
    if (terminated())
    {
        return;
    }

    if (m_minPriority >= message->priority)
    {
        m_messages.push_back(std::move(message));
    }
}

void LogEngine::threadFunction()
{
    while (!terminated())
    {
        if (processNextMessage() == ProcessResult::Error)
        {
            break;
        }
    }

    try
    {
        while (processNextMessage() == ProcessResult::Ok) {}
        close();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << std::endl);
    }
}


LogEngine::ProcessResult LogEngine::processNextMessage()
{
    Logger::UMessage message = nullptr;
    if (!m_messages.pop_front(message, 100ms))
    {
        flush();
        return ProcessResult::NoMoreMessages;
    }

    while (!saveMessage(*message))
    {
        if (terminated())
        {
            return ProcessResult::Error;
        }
        this_thread::sleep_for(1s);
    }

    if (option(Option::STDOUT))
    {
        string messagePrefix;
        if (option(Option::DATE))
        {
            messagePrefix += message->timestamp.dateString() + " ";
        }

        if (option(Option::TIME))
        {
            const auto printAccuracy = option(Option::MILLISECONDS) ? DateTime::PrintAccuracy::MILLISECONDS : DateTime::PrintAccuracy::SECONDS;
            messagePrefix += message->timestamp.timeString(DateTime::PF_RFC_DATE, printAccuracy) + " ";
        }

        if (option(Option::PRIORITY))
        {
            messagePrefix += "[" + priorityName(message->priority) + "] ";
        }

        if (message->priority <= LogPriority::Error)
        {
            CERR(messagePrefix.c_str() << message->message.c_str());
        }
        else
        {
            COUT(messagePrefix.c_str() << message->message.c_str());
        }
    }
    return ProcessResult::Ok;
}

void LogEngine::terminate()
{
    m_terminated = true;

    m_messages.wakeup();
    if (m_saveMessageThread.joinable())
    {
        m_saveMessageThread.join();
    }
}
