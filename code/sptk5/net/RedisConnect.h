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

#include <list>
#include <memory>
#include <mutex>
#include <string>

namespace sptk {

/**
 * @brief Redis-specific exception.
 */
class RedisConnectException : public Exception
{
public:
    /**
     * @brief Constructor.
     * @param message Error message.
     */
    RedisConnectException(const std::string& message)
        : Exception(message)
    {
    }
};

/**
 * @brief Redis Client.
 * @remarks Only the limited set of Redis methods is implemented.
 * @remarks This class is thread-safe.
 */
class SP_EXPORT RedisConnect final
{
public:
    using Command = std::vector<std::string>;
    using KeysAndValues = std::unordered_map<std::string, Variant>;

    /**
     * @brief Constructor
     */
    RedisConnect()
        : m_socket(std::make_shared<TCPSocket>())
    {
    }

    /**
     * @brief Connects to Redis server.
     * @param host Redis host.
     * @param port Redis port.
     * @param username Optional username.
     * @param password Optional password.
     * @param clientName Optional client name.
     * @return Server information.
     */
    std::vector<Variant> connect(const std::string& host, int port = 6379,
                                 const std::string& username = "", const std::string& password = "",
                                 const std::string& clientName = "");

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
     * @brief Get a value for the key.
     * @param key Key value.
     * @return Variant value.
     */
    [[nodiscard]] Variant get(const std::string& key);

    /**
     * @brief Get values for the keys.
     * @param keys Key values.
     * @return Variant values.
     */
    [[nodiscard]] KeysAndValues mget(const std::vector<std::string>& keys);
    void                        sendRequest(const Command& command);

    /**
     * @brief Sets the key-value pair in Redis.
     * @param key Key value.
     * @param value Value.
     */
    void set(const std::string& key, const Variant& value);

    /**
     * @brief Sets the multiple key-value pair in Redis.
     * @param keysAndValues Keys and corresponding values.
     */
    void mset(const KeysAndValues& keysAndValues);

    /**
     * @brief Sets the multiple key-value pair in Redis.
     * @param hash Hash name.
     * @param key Key in the hash.
     * @param value Value for the key.
     */
    void hset(const std::string& hash, const std::string& key, const Variant& value);

    /**
     * @brief Sets the multiple key-value pair in Redis.
     * @param hash Hash name.
     * @param keysAndValues Keys and corresponding values of hash elements.
     */
    void hset(const std::string& hash, const KeysAndValues& keysAndValues);

    /**
     * @brief Gets list of the hash keys.
     * @param hashName Hash name (key).
     * @return List of keys of the hash.
     */
    [[nodiscard]] std::vector<std::string> hkeys(const std::string& hashName);

    /**
     * @brief Gets hash key's value.
     * @param hash Hash name.
     * @param key Key.
     * @return Key value, the Variant is null if not found.
     */
    [[nodiscard]] Variant hget(const std::string& hash, const std::string& key);

    /**
     * @brief Gets hash keys and values for the list of keys.
     * @param hash Hash name.
     * @param keys Keys of the hash values.
     * @return Keys and values matching the passed keys of the hash.
     */
    [[nodiscard]] KeysAndValues hmget(const std::string& hash, const std::vector<std::string>& keys);

    /**
     * @brief Gets keys and values of all the keys from the hash.
     * @param hash Hash name.
     * @return Keys and values matching the passed keys of the hash.
     */
    [[nodiscard]] KeysAndValues hgetall(const std::string& hash);

    /**
     * @brief Removes list of keys from the hash.
     * @param hash Hash name.
     * @param keys Keys from the hash.
     */
    void hdel(const std::string& hash, const std::vector<std::string>& keys);

    /**
     * @brief Find keys matching the pattern.
     * The scan should start from cursor = 0 and stop after returned cursor is also 0.
     * @param pattern Pattern to match keys.
     * @param limit Match limit.
     * @return Matched keys.
     */
    [[nodiscard]] std::vector<std::string> scan(const std::string& pattern, size_t limit);

    /**
     * @brief Remove keys.
     * @param keys The keys to remove.
     * @return The number of the removed keys.
     */
    [[nodiscard]] size_t remove(const std::vector<std::string>& keys);

    /**
     * @brief Increment the key.
     * @param key The key to increment.
     * @return The new key value.
     */
    [[nodiscard]] int64_t incr(const std::string& key);

    /**
     * @brief Rename a key.
     * @param oldKey The current key name.
     * @param newKey The new key name.
     * @throws RedisConnectException if the old key does not exist.
     * @note If newKey already exists, it will be overwritten.
     */
    void rename(const std::string& oldKey, const std::string& newKey);

    /**
     * @brief Rename a key only if the new key does not exist.
     * @param oldKey The current key name.
     * @param newKey The new key name.
     * @return True if the key was renamed, false if newKey already exists.
     * @throws RedisConnectException if the old key does not exist.
     */
    [[nodiscard]] bool renameNX(const std::string& oldKey, const std::string& newKey);

    /**
     * @brief Begin a transaction block.
     * @details Marks the start of a transaction. Subsequent commands will be queued
     *          and executed atomically when commitTransaction() is called.
     *          Corresponds to Redis MULTI command.
     * @throws RedisConnectException if already in a transaction or not connected.
     */
    void beginTransaction();

    /**
     * @brief Commit and execute all queued commands in a transaction.
     * @details Executes all commands queued since beginTransaction() atomically.
     *          Corresponds to Redis EXEC command.
     * @return Results from all executed commands.
     * @throws RedisConnectException if not in a transaction or not connected.
     */
    [[nodiscard]] std::vector<Variant> commitTransaction();

    /**
     * @brief Discard all queued commands in a transaction.
     * @details Discards all commands queued since beginTransaction() without executing them.
     *          Corresponds to Redis DISCARD command.
     * @throws RedisConnectException if not in a transaction or not connected.
     */
    void rollbackTransaction();

private:
    mutable std::mutex            m_mutex;          ///< Mutex for thread safety.
    std::shared_ptr<TCPSocket>    m_socket;         ///< Underlying socket.
    std::unique_ptr<SocketReader> m_reader;         ///< Socket reader.
    Buffer                        m_sendBuffer;     ///< Read line buffer.
    Buffer                        m_readLineBuffer; ///< Read line buffer.

    /**
     * @brief Sends Redis command.
     * @param command Redis command elements.
     * @param results Redis command output.
     */
    void executeCommand(const Command& command, std::vector<Variant>& results, Variant* cursor = nullptr)
    {
        if (!m_socket->active())
        {
            throw RedisConnectException("Not connected");
        }

        if (command.empty())
        {
            throw RedisConnectException("Empty command data");
        }

        sendRequest(command);

        readResponse(results, cursor);
    }

    /**
     * @brief Reads a line from Redis.
     * @return A line from Redis.
     */
    [[nodiscard]] const Buffer& readLine();

    /**
     * @brief Reads a response from Redis.
     * @param results           Output results.
     * @param cursor            Optional output cursor for commands like SCAN.
     * @return Response as Variant.
     */
    void readResponse(std::vector<Variant>& results, Variant* cursor = nullptr)
    {
        const auto& line = readLine();
        if (line.empty())
        {
            throw RedisConnectException("Empty response");
        }

        const auto             type = line[0];
        const std::string_view payload {line.c_str() + 1, line.size() - 1};

        switch (type)
        {
            case '+': // Simple String
                results.emplace_back(payload);
                return;

            case '-': // Error
                throw RedisConnectException(std::string(payload));

            case ':': // Integer
                results.emplace_back(strtoll(payload.data(), nullptr, 10), 0u);
                return;

            case '$': { // Bulk String
                int64_t len;
                std::from_chars(payload.data(), payload.data() + payload.size(), len);
                if (len == -1)
                {
                    results.emplace_back(); // Null
                    return;
                }
                const auto readLength = len + 2;
                Buffer     buffer(readLength);
                m_reader->read(buffer, readLength); // Also read \r\n
                buffer.bytes(buffer.bytes() - 2);   // Cut off \r\n
                if (cursor)
                {
                    *cursor = buffer;
                }
                else
                {
                    results.emplace_back(std::move(buffer));
                }
                return;
            }
            case '*': { // Array
                int64_t count;
                std::from_chars(payload.data(), payload.data() + payload.size(), count);
                if (count == -1)
                {
                    results.emplace_back();
                    return;
                }
                // For read the array
                for (auto i = 0; i < count; ++i)
                {
                    readResponse(results, cursor);
                    cursor = nullptr;
                }
                return;
            }
            case '_':                   // Null (RESP3)
                results.emplace_back(); // Null
                return;
            case '#': // Boolean (RESP3)
                results.emplace_back(payload == "t");
                return;
            case ',': { // Double (RESP3)
                double value;
                std::from_chars(payload.data(), payload.data() + payload.size(), value);
                results.emplace_back(strtod(payload.data(), nullptr));
                return;
            }
            case '%': { // Map (RESP3)
                int64_t count;
                std::from_chars(payload.data(), payload.data() + payload.size(), count);
                for (auto i = 0; i < count; ++i)
                {
                    readResponse(results); // Key
                    readResponse(results); // Value
                }
                return;
            }
            default:
                throw RedisConnectException("Unknown response type: " + std::string(1, type));
        }
    }

    /**
     * @brief Find keys matching the pattern.
     * The scan should start from cursor = 0 and stop after returned cursor is also 0.
     * @param pattern Pattern to match keys.
     * @param cursor Cursor.
     * @param matchedKeys Output values.
     * @param limit Match limit.
     * @return Cursor.
     */
    size_t scan(const std::string& pattern, size_t cursor, std::vector<Variant>& matchedKeys, size_t limit);

    /**
     * @brief Append value to the command.
     * @param value Value to append.
     */
    static std::string serialize(const Variant& value);
};

using SRedisConnect = std::shared_ptr<RedisConnect>;

} // namespace sptk
