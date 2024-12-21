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

#include <sptk5/DateTime.h>
#include <sptk5/threads/TimerEvent.h>

#include <map>
#include <semaphore>

namespace sptk {

/**
 * Thread-safe event map
 */
class SP_EXPORT TimerEvents
{
public:
    /**
     * @brief Add event
     * @param event         Event
     */
    void add( const std::shared_ptr<TimerEvent>& event);

    STimerEvent next();

    void clear();

    bool empty() const;

    void wakeUp();

private:
    using EventMap = std::multimap<long, std::shared_ptr<TimerEvent>>;

    mutable std::mutex m_mutex;  ///< Mutex that protects access to events collection
    EventMap           m_events; ///< Events collection
    std::counting_semaphore<0x7FFFFFFF> m_semaphore {0};

    STimerEvent front();
};

} // namespace sptk
