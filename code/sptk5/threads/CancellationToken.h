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

#include "sptk5/cutils"

namespace sptk {

/**
 * @brief Simple binary semaphore used to control threads
 */
class CancellationToken
{
public:
    /**
     * @brief Reset state to not cancelled
     */
    void reset()
    {
        std::scoped_lock lock(m_mutex);
        m_cancelled = false;
    }

    /**
     * @brief Set state to cancelled
     */
    void cancel()
    {
        std::scoped_lock lock(m_mutex);
        m_cancelled = true;
    }

    /**
     * @brief Check state
     * @return true if cancelled
     */
    bool isCancelled() const
    {
        std::scoped_lock lock(m_mutex);
        return m_cancelled;
    }

private:
    mutable std::mutex m_mutex; ///< Mutex that protects state
    bool m_cancelled {false};
};

} // namespace sptk
