/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/threads/Thread.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

class Runable;

/**
 * @brief Thread event interface
 */
class SP_EXPORT ThreadEvent
{
public:
    /**
     * @brief Thread event type
     */
    enum class Type
        : uint8_t
    {
        THREAD_STARTED,     ///< Thread started event
        THREAD_FINISHED,    ///< Thread finished event
        RUNABLE_STARTED,    ///< Runable started
        RUNABLE_FINISHED,   ///< Runable finished
        IDLE_TIMEOUT        ///< Thread was idle longer than defined idle timeout
    };

    /**
     * @brief Thread event callback function
     *
     * In order to receive thread events, event receiver
     * should be derived from this class.
     * @param thread            Thread where event occured
     * @param eventType         Thread event type
     * @param runable           Related runable (if any)
     */
    virtual void threadEvent(Thread* thread, Type eventType, Runable* runable) = 0;

    /**
     * @brief Destructor
     */
    virtual ~ThreadEvent() = default;
};

/**
 * @}
 */
}
