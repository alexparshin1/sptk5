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

#include "TimerEvents.h"
#include "sptk5/threads/Thread.h"

namespace sptk {
/**
  * @brief Base class for Timer and IntervalTimer internal threads
  */
class SP_EXPORT TimerThread
    : public sptk::Thread
{
public:
    /**
   * @brief Constructor
   * @param threadName        Thread name
   */
    TimerThread();

    /**
     * @brief Destructor
     */
    ~TimerThread() override;

    /**
   * @brief Schedule an event
   * @param event             Event
   */
    void schedule(const STimerEvent& event);

    void clear();

    /**
   * @brief Terminate thread
   */
    void terminate() override;

protected:
    /**
   * @brief Thread function
   */
    void threadFunction() override;

private:
    TimerEvents m_scheduledEvents; ///< Scheduled events
};
} // namespace sptk
