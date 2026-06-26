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

#include <sptk5/Strings.h>
#include <sptk5/sptk.h>

#include <set>

namespace sptk {

/**
* @brief Log information details.
 *
 * Define information about server activities that should be logged.
 */
class SP_EXPORT LogDetails
{
public:
    /**
     * @brief Log details constants.
     */
    enum class MessageDetail : uint8_t
    {
        SERIAL_ID,
        SOURCE_IP,
        REQUEST_NAME,
        REQUEST_DURATION,
        REQUEST_DATA,
        RESPONSE_DATA,
        THREAD_POOLING
    };

    using MessageDetails = std::set<MessageDetail>;

    /**
     * @brief Default constructor.
     */
    LogDetails() = default;

    /**
     * @brief Constructor.
     * @param details           Log details.
     */
    explicit LogDetails(MessageDetails details)
        : m_details(std::move(details))
    {
    }

    /**
     * @brief Constructor.
     * @param details           Log details as the lower case strings.
     */
    explicit LogDetails(const Strings& details);

    /**
     * @brief Constructor.
     * @param details           Log details.
     */
    LogDetails(std::initializer_list<MessageDetail> details)
    {
        for (auto detail: details)
        {
            m_details.insert(detail);
        }
    }

    [[nodiscard]] String toString(const String& delimiter = ",") const;

    /**
     * @brief Query log details.
     * @param detail            Log detail.
     * @return true if log detail is set.
     */
    [[nodiscard]] bool has(MessageDetail detail) const
    {
        return m_details.contains(detail);
    }

    [[nodiscard]] bool empty() const
    {
        return m_details.empty();
    }

private:
    MessageDetails                               m_details; ///< Log details set.
    static const std::map<String, MessageDetail> detailNames;
};
} // namespace sptk
