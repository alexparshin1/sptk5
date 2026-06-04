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

#include "sptk5/Variant.h"
#include "sptk5/net/SocketReader.h"
#include "sptk5/net/TCPSocket.h"

#include <string>

namespace sptk {

/**
 * @brief Redis command.
 * @remarks A Redis command object contains command arguments as Redis strings.
 */
class RedisCommand final : public Buffer
{
public:
    /**
     * @brief Constructor.
     * @param command Redis command (string).
     * @param mode Redis command modifier or just a first argument.
     */
    RedisCommand(std::string_view command, std::string_view mode = "");

    /**
     * @brief Add argument.
     * @param argument Argument to add to command.
     */
    void emplace_back(std::string_view argument);

    /**
     * @brief Add argument.
     * @param argument Argument to add to command.
     */
    void emplace_back(const char* argument)
    {
        emplace_back(std::string_view(argument));
    }

    /**
     * @brief Add arguments.
     * @param arguments Arguments to add to command.
     */
    void emplace_back(const std::vector<std::string>& arguments);

    /**
     * @brief Add argument.
     * @param argument Argument to add to command.
     */
    void emplace_back(const Variant& argument);

    /**
     * @brief Get argument count.
     * @return argument count.
     */
    [[nodiscard]] size_t count() const;

private:
    size_t m_count {0}; ///< Argument count.
};

} // namespace sptk
