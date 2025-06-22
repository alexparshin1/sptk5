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

#include <sptk5/StreamLogEngine.h>

using namespace std;
using namespace sptk;

StreamLogEngine::~StreamLogEngine()
{
    terminate();
}

void StreamLogEngine::saveMessage(const Logger::Message& message)
{
    if (terminated())
    {
        return;
    }

    const auto options = this->options();

    const lock_guard lock(masterLock());

    if (options.contains(Option::ENABLE))
    {
        if (options.contains(Option::DATE))
        {
            m_logStream << message.timestamp.dateString() << " ";
        }

        if (options.contains(Option::TIME))
        {
            const auto printAccuracy = options.contains(Option::MILLISECONDS) ? DateTime::PrintAccuracy::MILLISECONDS : DateTime::PrintAccuracy::SECONDS;
            m_logStream << message.timestamp.timeString(true, printAccuracy) << " ";
        }

        if (options.contains(Option::PRIORITY))
        {
            m_logStream << "[" << priorityName(message.priority) << "] ";
        }

        m_logStream << message.message << '\n';

        if (m_logStream.bad())
        {
            CERR("Can't write to stream");
        }
    }
}

StreamLogEngine::StreamLogEngine(ostream& outputStream)
    : LogEngine("StreamLogEngine")
    , m_logStream(outputStream)
{
}
