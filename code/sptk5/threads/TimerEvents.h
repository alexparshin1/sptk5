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

#pragma once

#include <sptk5/threads/TimerEvent.h>

#include <map>
#include <semaphore>
#include <shared_mutex>

namespace sptk {
/**
 * @brief Thread-safe event map.
 */
class SP_EXPORT TimerEvents
{
public:
    /**
     * @brief Add event.
     * @param event         Event.
     */
    void add(const std::shared_ptr<TimerEvent>& event);

    /**
     * @brief Get the next event.
     * @return Next event.
     */
    STimerEvent next();

    /**
     * @brief Clear the event map.
     */
    void clear();

    /**
     * @brief Check if the event map is empty.
     * @return true if the event map is empty.
     */
    bool empty() const;

    /**
     * @brief Terminate the event map processing.
     */
    void terminate();

    /**
     * @return true if the event map is terminated.
     */
    bool terminated() const;

private:
    using EventMap = std::multimap<int64_t, std::shared_ptr<TimerEvent>>;

    mutable std::shared_mutex           m_mutex;  ///< Mutex that protects access to the event collection.
    EventMap                            m_events; ///< Events collection.
    std::counting_semaphore<0x7FFFFFFF> m_semaphore {0};
    bool                                m_terminated {false};

    STimerEvent front();
};

} // namespace sptk
