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

#include <sptk5/Printer.h>
#include <sptk5/threads/TimerEvents.h>

namespace sptk {

void TimerEvents::add(DateTime::time_point timestamp, const std::shared_ptr<TimerEvent>& event)
{
    std::lock_guard lock(m_mutex);
    if (const auto iterator = m_events.emplace(timestamp, event);
        iterator == m_events.begin())
    {
        m_semaphore.release();
    }
}

STimerEvent TimerEvents::next()
{
    auto event = front();
    if (!event)
    {
        if (m_semaphore.try_acquire_for(std::chrono::milliseconds(100)))
        {
            // Wait interrupted
            event = front();
        }
    }

    if (!event)
    {
        return {};
    }

    auto when = event->when();
    if (when < std::chrono::system_clock::now())
    {
        std::lock_guard lock(m_mutex);
        event = m_events.begin()->second;
        m_events.erase(m_events.begin());
        return event;
    }

    DateTime whenDateTime(when);
    if (m_semaphore.try_acquire_until(when))
    {
        // Wait interrupted
        return {};
    }

    std::lock_guard lock(m_mutex);
    if (!m_events.empty())
    {
        event = m_events.begin()->second;
        m_events.erase(m_events.begin());
    }
    return event;
}

void TimerEvents::clear()
{
    std::lock_guard lock(m_mutex);
    m_events.clear();
}

bool TimerEvents::empty() const
{
    std::lock_guard lock(m_mutex);
    return m_events.empty();
}

void TimerEvents::wakeUp()
{
    m_semaphore.release();
}

STimerEvent TimerEvents::front()
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

} // namespace sptk
