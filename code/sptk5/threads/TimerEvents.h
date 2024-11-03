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

#pragma once
#include <sptk5/threads/TimerEvent.h>

#include <map>
#include <sptk5/DateTime.h>

namespace sptk {

/**
 * Thread-safe event map
 */
class SP_EXPORT TimerEvents
{
public:
    /**
     * @brief Add event
     * @param timestamp     Event timestamp
     * @param event         Event
     * @param isFront       Out parameter: is event inserted at the front of the events
     */
    void add(DateTime::time_point timestamp, const std::shared_ptr<TimerEvent>& event, bool& isFront)
    {
        std::lock_guard lock(m_mutex);
        const auto      iterator = m_events.emplace(timestamp, event);
        isFront = iterator == m_events.begin();
    }

    std::shared_ptr<TimerEvent> front()
    {
        std::lock_guard lock(m_mutex);
        while (!m_events.empty())
        {
            const auto iterator = m_events.begin();
            if (iterator->second->cancelled())
            {
                m_events.erase(iterator);
            }
            else
            {
                return m_events.begin()->second;
            }
        }
        return {};
    }

    void popFront()
    {
        std::lock_guard lock(m_mutex);
        if (m_events.empty())
        {
            return;
        }
        auto iterator = m_events.begin();
        m_events.erase(iterator);
    }

    void clear()
    {
        std::lock_guard lock(m_mutex);
        m_events.clear();
    }

private:
    using EventMap = std::multimap<DateTime::time_point, std::shared_ptr<TimerEvent>>;

    mutable std::mutex m_mutex; ///< Mutex that protects access to events collection
    EventMap           m_events; ///< Events collection
};

} // sptk