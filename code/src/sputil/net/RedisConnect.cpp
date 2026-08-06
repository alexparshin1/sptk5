/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-05-10                                             ║
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

#include "sptk5/net/RedisConnect.h"
#include "sptk5/Base64.h"
#include "sptk5/Printer.h"

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace std;
using namespace sptk;

vector<Variant> RedisConnect::connect(const string& host, const uint16_t port,
                                      const string& username, const string& password, const string& clientName)
{
    scoped_lock lock(m_mutex);

    if (m_socket->active())
    {
        throw RedisConnectException("Already connected, please disconnect, first.");
    }

    m_redisUrl = URL("redis", host, port, username, password, clientName);

    m_socket->host(Host(host, port));
    m_socket->open();
    m_socket->setOption(IPPROTO_TCP, TCP_NODELAY, 1);
    m_reader = make_unique<SocketReader>(m_socket);

    RedisCommand command("HELLO", "3");

    if (!username.empty() && !password.empty())
    {
        command.emplace_back("AUTH");
        command.emplace_back(username);
        command.emplace_back(password);
    }

    if (!clientName.empty())
    {
        command.emplace_back("SETNAME");
        command.emplace_back(clientName);
    }

    vector<Variant> results;
    executeCommand(command, results);

    return results;
}

std::vector<Variant> RedisConnect::connect(const URL& connectURL)
{
    const auto& [host, port] = connectURL.hostAndPort();
    return connect(host, port, connectURL.username(), connectURL.password(), connectURL.path());
}

bool RedisConnect::isConnected() const
{
    scoped_lock lock(m_mutex);
    return m_socket->active();
}

void RedisConnect::disconnect()
{
    scoped_lock lock(m_mutex);
    if (m_socket->active())
    {
        m_socket->close();
    }
    m_reader.reset();
}

void RedisConnect::flush()
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("FLUSHDB");

    vector<Variant> results;
    executeCommand(command, results);
}

void RedisConnect::setValue(const string& key, const Variant& value)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("SET", key);
    command.emplace_back(value);

    vector<Variant> results;
    executeCommand(command, results);
}

void RedisConnect::setValues(const KeysAndValues& keysAndValues)
{
    if (keysAndValues.empty())
    {
        return;
    }

    scoped_lock lock(m_mutex);

    RedisCommand command("MSET");
    for (const auto& [key, value]: keysAndValues)
    {
        command.emplace_back(key);
        command.emplace_back(value);
    }

    vector<Variant> results;
    executeCommand(command, results);
}

void RedisConnect::setHashValue(const string& hash, const string& key, const Variant& value)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("HSET", hash);

    command.emplace_back(key);
    command.emplace_back(value);

    vector<Variant> results;
    executeCommand(command, results);
}

vector<string> RedisConnect::scan(const string& pattern, const size_t limit)
{
    scoped_lock lock(m_mutex);

    vector<string> results;

    size_t cursor = 0;
    do
    {
        vector<Variant> iterationResults;
        cursor = scan(pattern, cursor, iterationResults, 1000);
        if (!iterationResults.empty())
        {
            results.reserve(results.size() + iterationResults.size());
            ranges::transform(iterationResults, back_inserter(results), [](const auto& result)
                              {
                                  return result.asString();
                              });
        }
    } while (cursor != 0 && results.size() < limit);

    if (results.size() > limit)
    {
        results.resize(limit);
    }

    return results;
}

size_t RedisConnect::deleteKeys(const vector<string>& keys)
{
    if (keys.empty())
    {
        return 0;
    }

    scoped_lock lock(m_mutex);

    vector<Variant> results;
    RedisCommand    command("DEL");

    for (const auto& key: keys)
    {
        command.emplace_back(key);
    }

    executeCommand(command, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from DEL command");
    }

    const auto keysRemoved = results[0].asInteger();
    return keysRemoved;
}

int64_t RedisConnect::incrementKey(const string& key)
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("INCR", key);
    vector<Variant>    results;

    executeCommand(command, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from INCR command");
    }

    return results[0].asInt64();
}

void RedisConnect::renameKey(const string& oldKey, const string& newKey)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("RENAME");
    command.emplace_back(oldKey);
    command.emplace_back(newKey);

    vector<Variant> results;
    executeCommand(command, results);
}

bool RedisConnect::renameKeyIfExists(const string& oldKey, const string& newKey)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("RENAMENX");
    command.emplace_back(oldKey);
    command.emplace_back(newKey);

    vector<Variant> results;
    executeCommand(command, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from RENAMENX command");
    }

    return results[0].asInteger() == 1;
}

void RedisConnect::beginTransaction()
{
    scoped_lock lock(m_mutex);

    if (m_inTransaction)
    {
        throw RedisConnectException("Transaction is already started");
    }

    const RedisCommand command("MULTI");
    vector<Variant>    results;

    executeCommand(command, results);

    m_inTransaction = true;
}

vector<Variant> RedisConnect::commitTransaction()
{
    scoped_lock lock(m_mutex);

    if (!m_inTransaction)
    {
        throw RedisConnectException("Transaction is not started");
    }

    const RedisCommand command("EXEC");
    vector<Variant>    results;

    executeCommand(command, results);

    m_inTransaction = false;

    return results;
}

void RedisConnect::rollbackTransaction()
{
    scoped_lock lock(m_mutex);

    if (!m_inTransaction)
    {
        throw RedisConnectException("Transaction is not started");
    }

    const RedisCommand command("DISCARD");

    vector<Variant> results;

    m_inTransaction = false;

    executeCommand(command, results);
}

void RedisConnect::setHashValues(const string& hash, const KeysAndValues& keysAndValues)
{
    if (keysAndValues.empty())
    {
        return;
    }

    scoped_lock lock(m_mutex);

    RedisCommand command("HSET", hash);

    for (const auto& [key, value]: keysAndValues)
    {
        command.emplace_back(key);
        command.emplace_back(value);
    }

    vector<Variant> results;
    executeCommand(command, results);
}

vector<string> RedisConnect::getHashKeys(const string& hashName)
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("HKEYS", hashName);
    vector<Variant>    results;
    executeCommand(command, results);

    vector<string> keys;
    keys.reserve(results.size());
    ranges::transform(results, back_inserter(keys), [](const Variant& v)
                      {
                          return v.asString();
                      });
    return keys;
}

Variant RedisConnect::getHashValue(const std::string& hash, const std::string& key)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("HGET", hash);
    command.emplace_back(key);

    vector<Variant> results;
    executeCommand(command, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from HGET");
    }

    return results[0];
}

RedisConnect::KeysAndValues RedisConnect::getHashValues(const string& hash, const vector<string>& keys)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("HMGET", hash);
    command.emplace_back(keys);

    vector<Variant> results;
    KeysAndValues   output;
    output.reserve(keys.size());

    executeCommand(command, results);

    if (keys.size() != results.size())
    {
        throw RedisConnectException("Keys and results do not match");
    }

    for (size_t i = 0; i < results.size(); ++i)
    {
        output.try_emplace(keys[i], std::move(results[i]));
    }

    return output;
}

RedisConnect::KeysAndValues RedisConnect::getHashValues(const string& hash)
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("HGETALL", hash);
    vector<Variant>    results;
    executeCommand(command, results);

    if (results.size() % 2 != 0)
    {
        throw RedisConnectException("Unexpected odd number of elements in HGETALL response");
    }

    KeysAndValues output;
    output.reserve(results.size() / 2);
    for (size_t i = 0; i + 1 < results.size(); i += 2)
    {
        output[results[i].asString()] = std::move(results[i + 1]);
    }

    return output;
}

void RedisConnect::deleteHashKeys(const string& hash, const vector<string>& keys)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("HDEL", hash);
    command.emplace_back(keys);

    vector<Variant> results;
    executeCommand(command, results);
}

size_t RedisConnect::addSetMembers(const string& key, const vector<string>& members)
{
    if (members.empty())
    {
        return 0;
    }

    scoped_lock lock(m_mutex);

    RedisCommand command("SADD", key);
    command.emplace_back(members);

    vector<Variant> results;
    executeCommand(command, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from SADD command");
    }

    return static_cast<size_t>(results[0].asInt64());
}

vector<string> RedisConnect::getSetMembers(const string& key)
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("SMEMBERS", key);
    vector<Variant>    results;
    executeCommand(command, results);

    vector<string> members;
    members.reserve(results.size());
    ranges::transform(results, back_inserter(members), [](const Variant& v)
                      {
                          return v.asString();
                      });
    return members;
}

bool RedisConnect::isSetMember(const string& key, const string& member)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("SISMEMBER", key);
    command.emplace_back(member);

    vector<Variant> results;
    executeCommand(command, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from SISMEMBER command");
    }

    return results[0].asInt64() == 1;
}

size_t RedisConnect::deleteSetMembers(const string& key, const vector<string>& members)
{
    if (members.empty())
    {
        return 0;
    }

    scoped_lock lock(m_mutex);

    RedisCommand command("SREM", key);
    command.emplace_back(members);

    vector<Variant> results;
    executeCommand(command, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from SREM command");
    }

    return static_cast<size_t>(results[0].asInt64());
}

size_t RedisConnect::scan(const string& pattern, const size_t cursor, vector<Variant>& matchedKeys, const size_t limit)
{
    const auto cursorStr = to_string(cursor);
    const auto countStr = to_string(limit);

    RedisCommand command("SCAN", cursorStr);
    command.emplace_back("MATCH");
    command.emplace_back(pattern);

    if (limit != 0)
    {
        command.emplace_back("COUNT");
        command.emplace_back(countStr);
    }

    Variant newCursor;
    executeCommand(command, matchedKeys, &newCursor);

    return newCursor.asInt64();
}

std::string RedisConnect::toString() const
{
    scoped_lock lock(m_mutex);
    return m_socket->host().toString();
}

URL RedisConnect::getRedisUrl() const
{
    return m_redisUrl;
}

Variant RedisConnect::getValue(const string& key)
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("GET", key);
    vector<Variant>    results;
    executeCommand(command, results);
    if (results.empty())
    {
        return {};
    }

    return results[0];
}

RedisConnect::KeysAndValues RedisConnect::getValues(const vector<string>& keys)
{
    scoped_lock lock(m_mutex);

    RedisCommand command("MGET");

    for (const auto& key: keys)
    {
        command.emplace_back(key);
    }

    vector<Variant> results;
    KeysAndValues   output;
    output.reserve(keys.size());

    executeCommand(command, results);

    if (keys.size() != results.size())
    {
        throw RedisConnectException("Keys and results do not match");
    }

    for (size_t i = 0; i < results.size(); ++i)
    {
        output.try_emplace(keys[i], std::move(results[i]));
    }

    return output;
}

void RedisConnect::appendRequest(const RedisCommand& command)
{
    m_sendBuffer.append(24, "*{}\r\n", command.count());
    m_sendBuffer.append(command);
}

void RedisConnect::sendRequest(const RedisCommand& command)
{
    m_sendBuffer.bytes(0);
    appendRequest(command);
    m_socket->write(m_sendBuffer);
}

void RedisConnect::executeCommand(const RedisCommand& command, std::vector<Variant>& results, Variant* cursor)
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

void RedisConnect::readLine()
{
    if (m_reader->readLine(m_readBuffer) == 0)
    {
        if (!m_reader->readyToRead(10s))
        {
            throw RedisConnectException("Server read timeout");
        }
        m_reader->readLine(m_readBuffer);
    }

    // Remove \r from the end if present
    if (const auto lastCharPos = m_readBuffer.size() - 1;
        !m_readBuffer.empty() && m_readBuffer[lastCharPos] == '\r')
    {
        m_readBuffer.bytes(lastCharPos);
        m_readBuffer[lastCharPos] = 0;
    }
}

void RedisConnect::readResponse(std::vector<Variant>& results, Variant* cursor)
{
    readLine();
    if (m_readBuffer.empty())
    {
        throw RedisConnectException("Empty response");
    }

    const auto             type = m_readBuffer[0];
    const std::string_view payload {m_readBuffer.c_str() + 1, m_readBuffer.size() - 1};

    switch (type)
    {
        case '+': // Simple String
            results.emplace_back(payload);
            return;

        case '-': // Error
            throw RedisConnectException(std::string(payload));

        case ':': {
            // Integer
            int value {0};
            std::from_chars(payload.data(), payload.data() + payload.size(), value);
            results.emplace_back(value);
            return;
        }

        case '=':   // Verbatim String (RESP3)
        case '$': { // Bulk String
            int64_t len {0};
            std::from_chars(payload.data(), payload.data() + payload.size(), len);
            if (len == -1)
            {
                results.emplace_back(); // Null
                return;
            }
            const auto readLength = len + 2;
            m_readBuffer.reserve(readLength);
            m_reader->read(m_readBuffer, readLength);     // Also read \r\n
            m_readBuffer.bytes(m_readBuffer.bytes() - 2); // Cut off \r\n
            if (type == '=')
            {
                // A verbatim string begins with a three-character format and a colon, such as
                // "txt:" or "mkd:", describing how the text should be presented. Callers want the
                // text, so the marker is dropped here rather than by every one of them.
                constexpr size_t formatMarkerLength = 4;
                if (m_readBuffer.bytes() >= formatMarkerLength && m_readBuffer[3] == ':')
                {
                    memmove(m_readBuffer.data(), m_readBuffer.data() + formatMarkerLength,
                            m_readBuffer.bytes() - formatMarkerLength);
                    m_readBuffer.bytes(m_readBuffer.bytes() - formatMarkerLength);
                }
            }
            if (cursor)
            {
                *cursor = m_readBuffer;
            }
            else
            {
                results.emplace_back(m_readBuffer);
            }
            return;
        }
        case '*':   // Array
        case '~': { // Set (RESP3)
            int64_t count;
            std::from_chars(payload.data(), payload.data() + payload.size(), count);
            if (count == -1)
            {
                results.emplace_back();
                return;
            }
            for (auto i = 0; i < count; ++i)
            {
                readResponse(results, cursor);
                cursor = nullptr;
            }
            return;
        }
        case '!': { // Blob Error (RESP3)
            int64_t len {0};
            std::from_chars(payload.data(), payload.data() + payload.size(), len);
            if (len > 0)
            {
                const auto readLength = len + 2;
                m_readBuffer.reserve(readLength);
                m_reader->read(m_readBuffer, readLength);
                m_readBuffer.bytes(m_readBuffer.bytes() - 2);
                throw RedisConnectException(std::string(m_readBuffer.c_str(), m_readBuffer.bytes()));
            }
            throw RedisConnectException("Redis error");
        }

        case '(': { // Big number (RESP3), delivered as text - it may not fit an integer
            results.emplace_back(std::string(payload));
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
            results.emplace_back(value);
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

RedisConnect::~RedisConnect()
{
    if (m_worker.joinable())
    {
        m_worker.request_stop();
        m_taskQueue.wakeup();
        m_worker.join();
    }
}

void RedisConnect::startWorker()
{
    call_once(m_workerStarted, [this]
              {
                  m_worker = jthread([this](const stop_token& stopToken)
                                     {
                                         vector<AsyncTask> batch;
                                         while (!stopToken.stop_requested())
                                         {
                                             if (m_taskQueue.pop_front(batch, MaxPipelineBatch, 100ms))
                                             {
                                                 runBatch(batch);
                                             }
                                         }
                                     });
              });
}

void RedisConnect::runBatch(const vector<AsyncTask>& batch)
{
    // Pipeline consecutive single-command tasks, but flush before each self-contained task so that
    // operations are still executed in submission order (see asyncOperationsAreOrdered test).
    vector<size_t> pipelined;
    pipelined.reserve(batch.size());

    for (size_t i = 0; i < batch.size(); ++i)
    {
        if (batch[i].selfContained)
        {
            flushPipeline(batch, pipelined);
            pipelined.clear();

            try
            {
                batch[i].selfContained();
            }
            catch (const Exception& e)
            {
                // The underlying operation failed; its callback is intentionally not invoked. The
                // failure is reported to the error handler instead.
                reportAsyncError(e);
            }
            taskCompleted();
        }
        else
        {
            pipelined.push_back(i);
        }
    }

    flushPipeline(batch, pipelined);
}

void RedisConnect::flushPipeline(const vector<AsyncTask>& batch, const vector<size_t>& indices)
{
    if (indices.empty())
    {
        return;
    }

    vector<vector<Variant>> replies(indices.size());
    vector<bool>            succeeded(indices.size(), false);
    vector<string>          errors(indices.size()); ///< Failure message per task; set when !succeeded.

    {
        scoped_lock lock(m_mutex);
        if (m_socket->active())
        {
            // Send every queued request in a single write, then read one reply per request.
            m_sendBuffer.bytes(0);
            for (const auto index: indices)
            {
                appendRequest(*batch[index].command);
            }
            m_socket->write(m_sendBuffer);

            for (size_t k = 0; k < indices.size(); ++k)
            {
                try
                {
                    readResponse(replies[k]);
                    succeeded[k] = true;
                }
                catch (const Exception& e)
                {
                    // An error reply for this command was consumed; keep reading the remaining
                    // replies so the response stream stays aligned with the pipelined requests.
                    errors[k] = e.what();
                }
            }
        }
        else
        {
            for (auto& error: errors)
            {
                error = "Not connected";
            }
        }
    }

    // Invoke callbacks outside the connection lock, then mark each task complete. Completion is
    // signaled per task only after its callback returns, preserving waitForAsyncCompletion semantics.
    for (size_t k = 0; k < indices.size(); ++k)
    {
        if (succeeded[k])
        {
            if (const auto& onReply = batch[indices[k]].onReply)
            {
                try
                {
                    onReply(replies[k]);
                }
                catch (const Exception&)
                {
                    // A failing callback must not prevent completion bookkeeping for this or later tasks.
                }
            }
        }
        else
        {
            reportAsyncError(RedisConnectException(errors[k]));
        }
        taskCompleted();
    }
}

void RedisConnect::enqueue(function<void()> task)
{
    startWorker();
    {
        scoped_lock lock(m_asyncMutex);
        ++m_pendingTasks;
    }
    m_taskQueue.push_back(AsyncTask {.selfContained = std::move(task)});
}

void RedisConnect::enqueueCommand(RedisCommand command, function<void(vector<Variant>&)> onReply)
{
    startWorker();
    {
        scoped_lock lock(m_asyncMutex);
        ++m_pendingTasks;
    }
    m_taskQueue.push_back(AsyncTask {.command = std::move(command), .onReply = std::move(onReply)});
}

void RedisConnect::taskCompleted()
{
    bool allCompleted = false;
    {
        scoped_lock lock(m_asyncMutex);
        allCompleted = (--m_pendingTasks == 0);
    }
    // The only waiter (waitForAsyncCompletion) blocks until the pending count reaches zero,
    // so notifying on every task would just wake it to re-check and re-park, while contending
    // for m_asyncMutex and stalling the worker between operations. Notify only when draining.
    if (allCompleted)
    {
        m_asyncCondition.notify_all();
    }
}

void RedisConnect::setAsyncErrorHandler(ErrorCallback handler)
{
    scoped_lock lock(m_asyncMutex);
    m_asyncErrorHandler = std::move(handler);
}

void RedisConnect::reportAsyncError(const Exception& error) const
{
    // Copy the handler under the lock, then invoke it unlocked: the handler may re-enter the
    // connection (e.g. queue another async operation), which would deadlock on m_asyncMutex.
    ErrorCallback handler;
    {
        scoped_lock lock(m_asyncMutex);
        handler = m_asyncErrorHandler;
    }

    if (handler)
    {
        try
        {
            handler(error);
        }
        catch (...)
        {
            // A failing error handler must not disrupt the worker thread.
        }
    }
}

void RedisConnect::waitForAsyncCompletion()
{
    unique_lock lock(m_asyncMutex);
    m_asyncCondition.wait(lock, [this]
                          {
                              return m_pendingTasks == 0;
                          });
}

bool RedisConnect::waitForAsyncCompletion(const chrono::milliseconds timeout)
{
    unique_lock lock(m_asyncMutex);
    return m_asyncCondition.wait_for(lock, timeout, [this]
                                     {
                                         return m_pendingTasks == 0;
                                     });
}

void RedisConnect::getValueAsync(const string& key, ResultCallback<Variant> callback)
{
    enqueueCommand(RedisCommand("GET", key),
                   [callback = std::move(callback)](const vector<Variant>& results)
                   {
                       if (callback)
                       {
                           callback(results.empty() ? Variant {} : results[0]);
                       }
                   });
}

void RedisConnect::getValuesAsync(const vector<string>& keys, ResultCallback<KeysAndValues> callback)
{
    enqueue([this, keys, callback = std::move(callback)]
            {
                const auto result = getValues(keys);
                if (callback)
                {
                    callback(result);
                }
            });
}

void RedisConnect::setValueAsync(const string& key, const Variant& value, CompletionCallback callback)
{
    RedisCommand command("SET", key);
    command.emplace_back(value);

    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](vector<Variant>&)
                   {
                       if (callback)
                       {
                           callback();
                       }
                   });
}

void RedisConnect::setValuesAsync(const KeysAndValues& keysAndValues, CompletionCallback callback)
{
    enqueue([this, keysAndValues, callback = std::move(callback)]
            {
                setValues(keysAndValues);
                if (callback)
                {
                    callback();
                }
            });
}

void RedisConnect::setHashValueAsync(const string& hash, const string& key, const Variant& value, CompletionCallback callback)
{
    RedisCommand command("HSET", hash);
    command.emplace_back(key);
    command.emplace_back(value);

    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](vector<Variant>&)
                   {
                       if (callback)
                       {
                           callback();
                       }
                   });
}

void RedisConnect::setHashValuesAsync(const string& hash, const KeysAndValues& keysAndValues, CompletionCallback callback)
{
    if (keysAndValues.empty())
    {
        if (callback)
        {
            callback();
        }
        return;
    }

    // Enqueue as a single command rather than a self-contained task, for the same reason as
    // deleteHashKeysAsync: commands are pipelined by the worker, while a self-contained task
    // flushes the pipeline and performs its own synchronous round trip. This one was the last
    // async operation still taking that path, which made it slower than the singular
    // setHashValueAsync despite writing more per call - the opposite of what its name suggests.
    RedisCommand command("HSET", hash);
    for (const auto& [key, value]: keysAndValues)
    {
        command.emplace_back(key);
        command.emplace_back(value);
    }

    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](vector<Variant>&)
                   {
                       if (callback)
                       {
                           callback();
                       }
                   });
}

void RedisConnect::getHashKeysAsync(const string& hashName, ResultCallback<vector<string>> callback)
{
    enqueueCommand(RedisCommand("HKEYS", hashName),
                   [callback = std::move(callback)](vector<Variant>& results)
                   {
                       if (!callback)
                       {
                           return;
                       }
                       vector<string> keys;
                       keys.reserve(results.size());
                       ranges::transform(results, back_inserter(keys), [](const Variant& v)
                                         {
                                             return v.asString();
                                         });
                       callback(keys);
                   });
}

void RedisConnect::getHashValueAsync(const string& hash, const string& key, ResultCallback<Variant> callback)
{
    RedisCommand command("HGET", hash);
    command.emplace_back(key);

    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](const vector<Variant>& results)
                   {
                       if (results.empty())
                       {
                           throw RedisConnectException("Unexpected empty response from HGET");
                       }
                       if (callback)
                       {
                           callback(results[0]);
                       }
                   });
}

void RedisConnect::getHashValuesAsync(const string& hash, const vector<string>& keys, ResultCallback<KeysAndValues> callback)
{
    enqueue([this, hash, keys, callback = std::move(callback)]
            {
                const auto result = getHashValues(hash, keys);
                if (callback)
                {
                    callback(result);
                }
            });
}

void RedisConnect::getHashValuesAsync(const string& hash, ResultCallback<KeysAndValues> callback)
{
    enqueue([this, hash, callback = std::move(callback)]
            {
                const auto result = getHashValues(hash);
                if (callback)
                {
                    callback(result);
                }
            });
}

void RedisConnect::deleteHashKeysAsync(const string& hash, const vector<string>& keys, CompletionCallback callback)
{
    // Enqueue as a single command rather than a self-contained task: commands are pipelined
    // by the worker, while each self-contained task flushes the pipeline and performs its own
    // synchronous round trip, which is dramatically slower for bulk deletes.
    RedisCommand command("HDEL", hash);
    command.emplace_back(keys);
    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](vector<Variant>&)
                   {
                       if (callback)
                       {
                           callback();
                       }
                   });
}

void RedisConnect::scanAsync(const string& pattern, size_t limit, ResultCallback<vector<string>> callback)
{
    enqueue([this, pattern, limit, callback = std::move(callback)]
            {
                const auto result = scan(pattern, limit);
                if (callback)
                {
                    callback(result);
                }
            });
}

void RedisConnect::deleteKeysAsync(const vector<string>& keys, ResultCallback<size_t> callback)
{
    enqueue([this, keys, callback = std::move(callback)]
            {
                const auto result = deleteKeys(keys);
                if (callback)
                {
                    callback(result);
                }
            });
}

void RedisConnect::incrementKeyAsync(const string& key, ResultCallback<int64_t> callback)
{
    enqueueCommand(RedisCommand("INCR", key),
                   [callback = std::move(callback)](const vector<Variant>& results)
                   {
                       if (results.empty())
                       {
                           throw RedisConnectException("Unexpected empty response from INCR command");
                       }
                       if (callback)
                       {
                           callback(results[0].asInt64());
                       }
                   });
}

void RedisConnect::addSetMembersAsync(const string& key, const vector<string>& members, ResultCallback<size_t> callback)
{
    enqueue([this, key, members, callback = std::move(callback)]
            {
                const auto result = addSetMembers(key, members);
                if (callback)
                {
                    callback(result);
                }
            });
}

void RedisConnect::getSetMembersAsync(const string& key, ResultCallback<vector<string>> callback)
{
    enqueueCommand(RedisCommand("SMEMBERS", key),
                   [callback = std::move(callback)](vector<Variant>& results)
                   {
                       if (!callback)
                       {
                           return;
                       }
                       vector<string> members;
                       members.reserve(results.size());
                       ranges::transform(results, back_inserter(members), [](const Variant& v)
                                         {
                                             return v.asString();
                                         });
                       callback(members);
                   });
}

void RedisConnect::isSetMemberAsync(const string& key, const string& member, ResultCallback<bool> callback)
{
    RedisCommand command("SISMEMBER", key);
    command.emplace_back(member);

    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](const vector<Variant>& results)
                   {
                       if (results.empty())
                       {
                           throw RedisConnectException("Unexpected empty response from SISMEMBER command");
                       }
                       if (callback)
                       {
                           callback(results[0].asInt64() == 1);
                       }
                   });
}

void RedisConnect::deleteSetMembersAsync(const string& key, const vector<string>& members, ResultCallback<size_t> callback)
{
    enqueue([this, key, members, callback = std::move(callback)]
            {
                const auto result = deleteSetMembers(key, members);
                if (callback)
                {
                    callback(result);
                }
            });
}

void RedisConnect::renameKeyAsync(const string& oldKey, const string& newKey, CompletionCallback callback)
{
    RedisCommand command("RENAME");
    command.emplace_back(oldKey);
    command.emplace_back(newKey);

    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](vector<Variant>&)
                   {
                       if (callback)
                       {
                           callback();
                       }
                   });
}

void RedisConnect::renameKeyIfExistsAsync(const string& oldKey, const string& newKey, ResultCallback<bool> callback)
{
    RedisCommand command("RENAMENX");
    command.emplace_back(oldKey);
    command.emplace_back(newKey);

    enqueueCommand(std::move(command),
                   [callback = std::move(callback)](const vector<Variant>& results)
                   {
                       if (results.empty())
                       {
                           throw RedisConnectException("Unexpected empty response from RENAMENX command");
                       }
                       if (callback)
                       {
                           callback(results[0].asInteger() == 1);
                       }
                   });
}
