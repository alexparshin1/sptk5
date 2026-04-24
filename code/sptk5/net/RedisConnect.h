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
#include <memory>
#include <string>

namespace sptk {
/**
 * @brief Redis Client.
 */
class SP_EXPORT RedisConnect
{
public:
    /**
     * @brief Constructor
     */
    RedisConnect()
        : m_socket(std::make_shared<TCPSocket>())
    {
    }

    /**
     * @brief Destructor
     */
    virtual ~RedisConnect() = default;

    /**
     * @brief Connects to Redis server.
     * @param host Redis host.
     * @param port Redis port.
     * @return Server information.
     */
    std::vector<Variant> connect(const std::string& host, int port);

    /**
     * @brief Check if the connection is active.
     * @return Connection state.
     */
    [[nodiscard]] bool isConnected() const;

    /**
     * @brief Disconnects from Redis server.
     */
    void disconnect();

    /**
     * @brief Get a variant value for the key.
     * @param key Key value.
     * @return Variant value.
     */
    [[nodiscard]] Variant get(const std::string& key) const;

    /**
     * @brief Get a binary value for the key.
     * @param key Key value.
     * @return Binary value.
     */
    [[nodiscard]] Buffer getBinary(const std::string& key) const;

    /**
     * @brief Sets the key-value pair in Redis.
     * @param key Key value.
     * @param value Value.
     */
    void set(const std::string& key, const Variant& value) const;

    /**
     * @brief Sets the key-value pair in Redis.
     * @param key Key value.
     * @param value Value.
     */
    void setBinary(const std::string& key, const Buffer& value) const;

    /**
     * @brief Find keys matching the pattern.
     * The scan should start from cursor = 0 and stop after returned cursor is also 0.
     * @param pattern Pattern to match keys.
     * @param cursor Cursor.
     * @param matchedKeys Output values.
     * @param limit Match limit.
     * @return Cursor.
     */
    size_t scan(const std::string& pattern, size_t cursor, std::vector<Variant>& matchedKeys, size_t limit) const;

private:
    std::shared_ptr<TCPSocket>    m_socket; ///< Underlying socket
    std::unique_ptr<SocketReader> m_reader; ///< Socket reader

    /**
     * @sends Redis command.
     * @param commandElements Redis command elements.
     */
    void sendCommand(const std::vector<std::string_view>& commandElements) const;

    /**
     * @brief Reads a line from Redis.
     * @return A line from Redis.
     */
    [[nodiscard]] std::string readLine() const;

    /**
     * @brief Reads a response from Redis.
     * @return Response as Variant.
     */
    void readResponse(std::vector<Variant>& results) const;
};

} // namespace sptk
