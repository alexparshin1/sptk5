/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       LogEngine.cpp - description                            ║
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

#include <sptk5/LogEngine.h>

using namespace std;
using namespace sptk;

LogEngine::LogEngine(const String& logEngineName)
: Thread(logEngineName),
  m_defaultPriority(LP_INFO),
  m_minPriority(LP_INFO),
  m_options(LO_ENABLE | LO_DATE | LO_TIME | LO_PRIORITY)
{
    run();
}

LogEngine::~LogEngine()
{
	terminate();
	join();
}

void LogEngine::option(Option option, bool flag)
{
    if (flag)
        m_options |= option;
    else
        m_options &= ~option;
}

String LogEngine::priorityName(LogPriority prt)
{
    switch (prt) {
    case LP_DEBUG:     return "DEBUG";
    case LP_INFO:      return "INFO";
    case LP_NOTICE:    return "NOTICE";
    case LP_WARNING:   return "WARNING";
    case LP_ERROR:     return "ERROR";
    case LP_CRITICAL:  return "CRITICAL";
    case LP_ALERT:     return "ALERT";
    case LP_PANIC:     return "PANIC";
    }
    return "";
}

LogPriority LogEngine::priorityFromName(const String& prt)
{
    static const Strings priorityNames("DEBUG|INFO|NOTICE|WARNING|ERROR|CRITICAL|ALERT|PANIC", "|");

    switch (priorityNames.indexOf(prt)) {
        case 0: return LP_DEBUG;
        case 1: return LP_INFO;
        case 2: return LP_NOTICE;
        case 3: return LP_WARNING;
        case 4: return LP_ERROR;
        case 5: return LP_CRITICAL;
        case 6: return LP_ALERT;
        case 7: return LP_PANIC;
        default: return LP_DEBUG;
    }
}

void LogEngine::log(Logger::Message* message)
{
    m_messages.push(message);
}

void LogEngine::threadFunction()
{
    chrono::seconds timeout(1);
    while (!terminated()) {
        Logger::Message* message;
        if (m_messages.pop(message, timeout)) {
            saveMessage(message);

			if (m_options & LO_STDOUT) {
				string messagePrefix;
				if (m_options & LO_DATE)
					messagePrefix += message->timestamp.dateString() + " ";

				if (m_options & LO_TIME)
					messagePrefix += message->timestamp.timeString(true) + " ";

				if (m_options & LO_PRIORITY)
					messagePrefix += "[" + priorityName(message->priority) + "] ";

				FILE* dest = stdout;
				if (message->priority <= LP_ERROR)
					dest = stderr;
				fprintf(dest, "%s%s\n", messagePrefix.c_str(), message->message.c_str());
			}
			
			delete message;
        }
    }
}
