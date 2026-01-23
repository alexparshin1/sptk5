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

#include <sptk5/threads/TimerEvents.h>

using namespace std;

namespace sptk {

void TimerEvents::add(const std::shared_ptr<TimerEvent>& event)
{
    auto when = event->mcs_since_epoch();

    std::unique_lock lock(m_mutex);
    if (const auto iterator = m_events.emplace(when, event);
        iterator == m_events.begin())
    {
        m_semaphore.release();
    }
}

STimerEvent TimerEvents::next()
{
    auto event = front();
    if (!event && m_semaphore.try_acquire_for(std::chrono::milliseconds(100)))
    {
        // Wait interrupted
        event = front();
    }

    if (!event)
    {
        return {};
    }

    const auto when = event->when();
    if (when < std::chrono::system_clock::now())
    {
        std::unique_lock lock(m_mutex);
        event = m_events.begin()->second;
        m_events.erase(m_events.begin());
        return event;
    }

    if (m_semaphore.try_acquire_until(when))
    {
        // Wait interrupted
        return {};
    }

    std::unique_lock lock(m_mutex);
    if (!m_events.empty())
    {
        event = m_events.begin()->second;
        m_events.erase(m_events.begin());
    }
    return event;
}

void TimerEvents::clear()
{
    std::unique_lock lock(m_mutex);
    m_events.clear();
}

bool TimerEvents::empty() const
{
    std::shared_lock lock(m_mutex);
    return m_events.empty();
}

void TimerEvents::terminate()
{
    std::unique_lock lock(m_mutex);
    m_terminated = true;
    m_semaphore.release();
}

bool TimerEvents::terminated() const
{
    std::shared_lock lock(m_mutex);
    return m_terminated;
}

STimerEvent TimerEvents::front()
{
    std::unique_lock lock(m_mutex);

    if (m_terminated)
    {
        return {};
    }

    while (!m_events.empty())
    {
        if (const auto iterator = m_events.begin();
            iterator->second->cancelled())
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
