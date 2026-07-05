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
#include "sptk5/net/RedisCommand.h"
#include "sptk5/net/SocketReader.h"
#include "sptk5/net/TCPSocket.h"
#include "sptk5/threads/SynchronizedQueue.h"

#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

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

 * Only the limited set of Redis methods is implemented.
 * The class is thread-safe, but the transaction control statements are not.
 * If (within the same connection) transaction is started from one thread, all threads accessing
 * the connection are affected.
 */
class SP_EXPORT RedisConnect final
{
public:
    using KeysAndValues = std::unordered_map<std::string, Variant>;

    /**
     * @brief Callback delivering the result of an asynchronous operation.
     * @tparam T Result type of the corresponding synchronous method.
     */
    template<typename T>
    using ResultCallback = std::function<void(const T&)>;

    /**
     * @brief Callback signaling completion of an asynchronous operation that has no result.
     */
    using CompletionCallback = std::function<void()>;

    /**
     * @brief Callback invoked when an asynchronous operation fails.
     * @details Receives the exception that caused the failure. Since a failed asynchronous operation
     *          does not invoke its result/completion callback, this is the only way to observe such
     *          failures (e.g. for logging).
     */
    using ErrorCallback = std::function<void(const Exception&)>;

    /**
     * @brief Constructor
     */
    RedisConnect()
        : m_socket(std::make_shared<TCPSocket>())
    {
    }

    /**
     * @brief Destructor. Stops the asynchronous worker thread, dropping any queued operations.
     */
    ~RedisConnect();

    RedisConnect(const RedisConnect&) = delete;
    RedisConnect& operator=(const RedisConnect&) = delete;

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
     * @brief Flushes the Redis database.
     * @remarks The database contents will be cleared.
     */
    void flush();

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
     * @note If the newKey already exists, it will be overwritten.
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
     * @name Asynchronous operations
     * @brief Non-blocking counterparts of the data methods above.
     *
     * Each *Async method queues the operation onto a single background worker thread and returns
     * immediately. The result is delivered to the supplied callback when the operation completes;
     * for void operations the callback (if provided) signals completion. Operations are executed
     * in the order they were queued. If the underlying operation throws, the result/completion
     * callback is not invoked; the failure is instead reported to the handler registered with
     * setAsyncErrorHandler(), if any. Connection, disconnection and transaction control have no
     * asynchronous form.
     * @{
     */
    void getValueAsync(const std::string& key, ResultCallback<Variant> callback);
    void getValuesAsync(const std::vector<std::string>& keys, ResultCallback<KeysAndValues> callback);
    void setValueAsync(const std::string& key, const Variant& value, CompletionCallback callback = {});
    void setValuesAsync(const KeysAndValues& keysAndValues, CompletionCallback callback = {});
    void setHashValueAsync(const std::string& hash, const std::string& key, const Variant& value, CompletionCallback callback = {});
    void setHashValuesAsync(const std::string& hash, const KeysAndValues& keysAndValues, CompletionCallback callback = {});
    void getHashKeysAsync(const std::string& hashName, ResultCallback<std::vector<std::string>> callback);
    void getHashValueAsync(const std::string& hash, const std::string& key, ResultCallback<Variant> callback);
    void getHashValuesAsync(const std::string& hash, const std::vector<std::string>& keys, ResultCallback<KeysAndValues> callback);
    void getHashValuesAsync(const std::string& hash, ResultCallback<KeysAndValues> callback);
    void deleteHashKeysAsync(const std::string& hash, const std::vector<std::string>& keys, CompletionCallback callback = {});
    void scanAsync(const std::string& pattern, size_t limit, ResultCallback<std::vector<std::string>> callback);
    void deleteKeysAsync(const std::vector<std::string>& keys, ResultCallback<size_t> callback);
    void incrementKeyAsync(const std::string& key, ResultCallback<int64_t> callback);
    void addSetMembersAsync(const std::string& key, const std::vector<std::string>& members, ResultCallback<size_t> callback);
    void getSetMembersAsync(const std::string& key, ResultCallback<std::vector<std::string>> callback);
    void isSetMemberAsync(const std::string& key, const std::string& member, ResultCallback<bool> callback);
    void deleteSetMembersAsync(const std::string& key, const std::vector<std::string>& members, ResultCallback<size_t> callback);
    void renameKeyAsync(const std::string& oldKey, const std::string& newKey, CompletionCallback callback = {});
    void renameKeyIfExistsAsync(const std::string& oldKey, const std::string& newKey, ResultCallback<bool> callback);

    /**
     * @brief Registers a handler invoked when an asynchronous operation fails.
     * @details The handler is called on the worker thread, outside the connection lock, with the
     *          exception that caused the failure. Passing an empty handler clears it. Thread-safe;
     *          may be called at any time, though it is normally set once before issuing asynchronous
     *          operations.
     * @param handler Error handler, or empty to disable error reporting.
     */
    void setAsyncErrorHandler(ErrorCallback handler);

    /**
     * @brief Waits until all queued asynchronous operations have completed.
     * @details Blocks the calling thread until the worker has finished every operation queued so far,
     *          including their callbacks. Returns immediately if nothing is pending.
     * @note Must not be called from within an asynchronous callback, as that would deadlock.
     */
    void waitForAsyncCompletion();

    /**
     * @brief Waits until all queued asynchronous operations have completed, or the timeout elapses.
     * @param timeout Maximum time to wait.
     * @return True if all operations completed, false if the timeout elapsed first.
     * @note Must not be called from within an asynchronous callback, as that would deadlock.
     */
    [[nodiscard]] bool waitForAsyncCompletion(std::chrono::milliseconds timeout);
    /** @} */

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

    /**
     * @brief Executes Redis command and returns results.
     * @param command Redis command elements.
     * @param results Redis command output.
     * @param cursor Optional Redis cursor for SCAN-like commands.
     */
    void executeCommand(const RedisCommand& command, std::vector<Variant>& results, Variant* cursor = nullptr);

    /**
     * @brief Get Redis connection information.
     * @return Redis connection information.
     */
    std::string toString() const;

private:
    mutable std::mutex            m_mutex;                 ///< Mutex for thread safety.
    std::shared_ptr<TCPSocket>    m_socket;                ///< Underlying socket.
    std::unique_ptr<SocketReader> m_reader;                ///< Socket reader.
    Buffer                        m_sendBuffer;            ///< Send command buffer.
    Buffer                        m_readBuffer;            ///< Read buffer.
    bool                          m_inTransaction {false}; ///< If true then transaction is started.

    /**
     * @brief A single queued asynchronous operation.
     *
     * A task is either @e pipelined or @e self-contained.
     * A pipelined task carries a single Redis @c command and an @c onReply handler, so the worker
     * can batch its request with others.
     * A self-contained task carries a @c selfContained callable that performs its own request/response
     * I/O, used for collection or multi-round-trip operations such as SCAN that cannot be expressed
     * as one pipelined command.
     */
    struct AsyncTask
    {
        std::optional<RedisCommand>                command;       ///< Pipelined request; empty for self-contained tasks.
        std::function<void(std::vector<Variant>&)> onReply;       ///< Handles the pipelined reply.
        std::function<void()>                      selfContained; ///< Performs its own I/O; set => not pipelined.
    };

    /// Maximum number of pipelined requests sent before reading their replies.
    static constexpr size_t MaxPipelineBatch = 256;

    SynchronizedQueue<AsyncTask> m_taskQueue;         ///< Queue of pending asynchronous operations.
    std::jthread                 m_worker;            ///< Worker thread executing queued operations.
    std::once_flag               m_workerStarted;     ///< Guards lazy worker thread startup.
    mutable std::mutex           m_asyncMutex;        ///< Guards the pending operation counter and error handler.
    std::condition_variable      m_asyncCondition;    ///< Signaled when a queued operation completes.
    size_t                       m_pendingTasks {0};  ///< Number of queued operations not yet completed.
    ErrorCallback                m_asyncErrorHandler; ///< Invoked when an asynchronous operation fails.

    /**
     * @brief Lazily starts the asynchronous worker thread on first use.
     */
    void startWorker();

    /**
     * @brief Queues a self-contained task (performs its own I/O) for execution on the worker thread.
     * @param task Task to execute.
     */
    void enqueue(std::function<void()> task);

    /**
     * @brief Queues a single-command operation that the worker may pipeline with other queued commands.
     * @param command Request to send.
     * @param onReply Handler invoked with the reply; not called if the command fails.
     */
    void enqueueCommand(RedisCommand command, std::function<void(std::vector<Variant>&)> onReply);

    /**
     * @brief Executes a batch of queued tasks, pipelining consecutive single-command operations.
     * @param batch Tasks popped from the queue, in submission order.
     */
    void runBatch(const std::vector<AsyncTask>& batch);

    /**
     * @brief Sends a run of pipelined requests in one write, reads their replies, and dispatches callbacks.
     * @param batch   The batch being processed.
     * @param indices Indices into @p batch of the consecutive pipelined tasks to flush, in order.
     */
    void flushPipeline(const std::vector<AsyncTask>& batch, const std::vector<size_t>& indices);

    /**
     * @brief Marks a queued task as completed and wakes any waiters.
     */
    void taskCompleted();

    /**
     * @brief Reports a failed asynchronous operation to the registered error handler, if any.
     * @details Invoked on the worker thread, outside the connection lock. A throwing handler is
     *          ignored so it cannot disrupt the worker.
     * @param error Exception that caused the failure.
     */
    void reportAsyncError(const Exception& error) const;

    /**
     * @brief Appends a Redis command to the send buffer without writing it to the socket.
     * @param command Redis command elements.
     */
    void appendRequest(const RedisCommand& command);

    /**
     * @brief Sends Redis command.
     * @param command Redis command elements.
     */
    void sendRequest(const RedisCommand& command);

    /**
     * @brief Reads a line from Redis.
     * @return A line from Redis.
     */
    void readLine();

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
};

using SRedisConnect = std::shared_ptr<RedisConnect>;

} // namespace sptk
