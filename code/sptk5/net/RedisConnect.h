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
    std::vector<Variant> connect(const std::string& host, uint16_t port = 6379,
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
    [[nodiscard]] Variant getValue(const std::string& key);

    /**
     * @brief Get values for the keys.
     * @param keys Key values.
     * @return Variant values.
     */
    [[nodiscard]] KeysAndValues getValues(const std::vector<std::string>& keys);

    /**
     * @brief Sets the key-value pair in Redis.
     * @param key Key value.
     * @param value Value.
     */
    void setValue(const std::string& key, const Variant& value);

    /**
     * @brief Sets the multiple key-value pair in Redis.
     * @param keysAndValues Keys and corresponding values.
     */
    void setValues(const KeysAndValues& keysAndValues);

    /**
     * @brief Sets the multiple key-value pair in Redis.
     * @param hash Hash name.
     * @param key Key in the hash.
     * @param value Value for the key.
     */
    void setHashValue(const std::string& hash, const std::string& key, const Variant& value);

    /**
     * @brief Sets the multiple key-value pair in Redis.
     * @param hash Hash name.
     * @param keysAndValues Keys and corresponding values of hash elements.
     */
    void setHashValues(const std::string& hash, const KeysAndValues& keysAndValues);

    /**
     * @brief Gets list of the hash keys.
     * @param hashName Hash name (key).
     * @return List of keys of the hash.
     */
    [[nodiscard]] std::vector<std::string> getHashKeys(const std::string& hashName);

    /**
     * @brief Gets hash key's value.
     * @param hash Hash name.
     * @param key Key.
     * @return Key value, the Variant is null if not found.
     */
    [[nodiscard]] Variant getHashValue(const std::string& hash, const std::string& key);

    /**
     * @brief Gets hash keys and values for the list of keys.
     * @param hash Hash name.
     * @param keys Keys of the hash values.
     * @return Keys and values matching the passed keys of the hash.
     */
    [[nodiscard]] KeysAndValues getHashValues(const std::string& hash, const std::vector<std::string>& keys);

    /**
     * @brief Gets keys and values of all the keys from the hash.
     * @param hash Hash name.
     * @return Keys and values matching the passed keys of the hash.
     */
    [[nodiscard]] KeysAndValues getHashValues(const std::string& hash);

    /**
     * @brief Removes list of keys from the hash.
     * @param hash Hash name.
     * @param keys Keys from the hash.
     */
    void deleteHashKeys(const std::string& hash, const std::vector<std::string>& keys);

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
    [[nodiscard]] size_t deleteKeys(const std::vector<std::string>& keys);

    /**
     * @brief Increment the key.
     * @param key The key to increment.
     * @return The new key value.
     */
    [[nodiscard]] int64_t incrementKey(const std::string& key);

    /**
     * @brief Adds one or more members to a set.
     * @param key Set key.
     * @param members Members to add.
     * @return Number of members actually added (excluding already-present ones).
     */
    size_t addSetMembers(const std::string& key, const std::vector<std::string>& members);

    /**
     * @brief Returns all members of a set.
     * @param key Set key.
     * @return All members of the set.
     */
    [[nodiscard]] std::vector<std::string> getSetMembers(const std::string& key);

    /**
     * @brief Tests whether a value is a member of a set.
     * @param key Set key.
     * @param member Value to test.
     * @return True if the member exists in the set.
     */
    [[nodiscard]] bool isSetMember(const std::string& key, const std::string& member);

    /**
     * @brief Removes one or more members from a set.
     * @param key Set key.
     * @param members Members to remove.
     * @return Number of members actually removed.
     */
    size_t deleteSetMembers(const std::string& key, const std::vector<std::string>& members);

    /**
     * @brief Rename a key.
     * @param oldKey The current key name.
     * @param newKey The new key name.
     * @throws RedisConnectException if the old key does not exist.
     * @note If newKey already exists, it will be overwritten.
     */
    void renameKey(const std::string& oldKey, const std::string& newKey);

    /**
     * @brief Rename a key only if the new key does not exist.
     * @param oldKey The current key name.
     * @param newKey The new key name.
     * @return True if the key was renamed, false if newKey already exists.
     * @throws RedisConnectException if the old key does not exist.
     */
    [[nodiscard]] bool renameKeyIfExists(const std::string& oldKey, const std::string& newKey);

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
    std::vector<Variant> commitTransaction();

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
     */
    void sendRequest(const Command& command);

    /**
     * @brief Sends Redis command.
     * @param command Redis command elements.
     * @param results Redis command output.
     */
    void executeCommand(const Command& command, std::vector<Variant>& results, Variant* cursor = nullptr);

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
    void readResponse(std::vector<Variant>& results, Variant* cursor = nullptr);

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
