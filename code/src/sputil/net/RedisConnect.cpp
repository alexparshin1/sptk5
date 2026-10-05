/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "sptk5/net/RedisConnect.h"

#include <algorithm>
#include "sptk5/Base64.h"
#include "sptk5/Printer.h"

#ifndef _WIN32
#include <cstdlib>
#include <netinet/tcp.h>
#endif

using namespace std;
using namespace sptk;

namespace {

// Wakes a thread blocked in recv() on the socket. close() cannot do it: it takes the socket's lock
// exclusively, and a full-duplex recv() holds that lock shared for as long as it blocks.
void shutdownSocket(const TCPSocket& socket)
{
    if (const auto fd = socket.fd(); fd != INVALID_SOCKET)
    {
#ifdef _WIN32
        ::shutdown(fd, SD_BOTH);
#else
        ::shutdown(fd, SHUT_RDWR);
#endif
    }
}

/**
 * @brief The database a connection's path names, as in redis://host:6379/2.
 *
 * A path that is a number is the database the connection selects; any other path is, as it always
 * was, the name the connection gives itself.
 *
 * @return the database number, or -1 when the path is not one.
 */
int databaseOf(const string& path)
{
    const string_view number = !path.empty() && path.front() == '/' ? string_view(path).substr(1) : string_view(path);
    if (number.empty() || number.size() > 5 || !ranges::all_of(number, [](const char c) { return isdigit(static_cast<unsigned char>(c)) != 0; }))
    {
        return -1;
    }
    return stoi(string(number));
}

/// The HELLO a connection opens with, naming the client unless the path names a database instead.
RedisCommand helloCommand(const string& username, const string& password, const string& path)
{
    RedisCommand hello("HELLO", "3");
    if (!username.empty() && !password.empty())
    {
        hello.emplace_back("AUTH");
        hello.emplace_back(username);
        hello.emplace_back(password);
    }
    if (!path.empty() && databaseOf(path) < 0)
    {
        hello.emplace_back("SETNAME");
        hello.emplace_back(path);
    }
    return hello;
}

} // namespace

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

    vector<Variant> results;
    executeCommand(helloCommand(username, password, clientName), results);

    // Every command after this one runs in the selected database - which is what lets several users
    // of one server, test runs among them, keep apart, FLUSHDB included.
    if (const auto database = databaseOf(clientName); database >= 0)
    {
        vector<Variant> selected;
        executeCommand(RedisCommand("SELECT", to_string(database)), selected);
    }

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
    {
        scoped_lock lock(m_mutex);
        if (m_socket->active())
        {
            m_socket->close();
        }
        m_reader.reset();
    }

    // Only woken here, not joined: this may run on the reader's own thread, from a callback. The
    // reader fails what is still unanswered and exits; the writer replaces the connection on its
    // next use, if there is anything to connect to by then.
    const scoped_lock lock(m_asyncSocketMutex);
    if (m_asyncSocket)
    {
        shutdownSocket(*m_asyncSocket);
    }
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
    appendRequest(m_sendBuffer, command);
}

void RedisConnect::appendRequest(Buffer& buffer, const RedisCommand& command)
{
    buffer.append(24, "*{}\r\n", command.count());
    buffer.append(command);
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
            // strtod rather than std::from_chars: libc++ has no floating-point from_chars and
            // deletes the overload outright, so that call does not compile against it - the
            // integral ones above are fine. The accepted format is the same, and nothing here
            // changes the global locale, so the decimal point is the same too.
            //
            // Initialised, unlike before: from_chars leaves its output untouched when the text
            // does not parse, and the value was read either way.
            const std::string text(payload);
            const double      value = std::strtod(text.c_str(), nullptr);
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
        m_workerStop = true;
        m_taskQueue.wakeup();
        m_inFlightChanged.notify_all();
        m_worker.join();
    }
    stopAsyncConnection();
}

void RedisConnect::startWorker()
{
    call_once(m_workerStarted, [this]
              {
                  m_worker = JoiningThread([this]
                                     {
                                         vector<AsyncTask> batch;
                                         while (!m_workerStop)
                                         {
                                             if (m_taskQueue.pop_front(batch, MaxPipelineBatch, 100ms))
                                             {
                                                 runBatch(batch);
                                             }
                                         }
                                     });
              });
}

void RedisConnect::runBatch(vector<AsyncTask>& batch)
{
    for (auto& task: batch)
    {
        if (task.selfContained)
        {
            // A self-contained task does its own round trip on m_socket. Everything submitted before
            // it has to be answered first, so that operations still happen in submission order (see
            // the asyncOperationsAreOrdered test).
            waitForInFlight(0);
            try
            {
                task.selfContained();
            }
            catch (const Exception& e)
            {
                // The underlying operation failed; its callback is intentionally not invoked. The
                // failure is reported to the error handler instead.
                reportAsyncError(e);
            }
            taskCompleted();
            continue;
        }

        if (!ensureAsyncConnection())
        {
            reportAsyncError(RedisConnectException("Not connected"));
            taskCompleted();
            continue;
        }

        bool accepted = false;
        {
            unique_lock lock(m_inFlightMutex);
            if (m_inFlight.size() >= MaxInFlight)
            {
                // Bounded, so that a stalled Redis costs memory up to a limit, not without one.
                lock.unlock();
                waitForInFlight(MaxInFlight - 1);
                lock.lock();
            }
            // Registered before the request is sent, so the reader always finds the handler of
            // the reply it has just parsed. Refused once the connection has failed: the reader has
            // already failed everything registered, and nothing would answer this one.
            if (!m_asyncBroken)
            {
                m_inFlight.push_back({std::move(task.onReply), true});
                accepted = true;
            }
        }

        if (accepted)
        {
            appendRequest(m_asyncSendBuffer, *task.command);
        }
        else
        {
            reportAsyncError(RedisConnectException("Redis connection lost"));
            taskCompleted();
        }
    }

    sendAsyncRequests();
}

bool RedisConnect::ensureAsyncConnection()
{
    {
        const scoped_lock lock(m_inFlightMutex);
        if (m_asyncSocket && !m_asyncBroken)
        {
            return true;
        }
    }

    // The previous connection failed, or there never was one.
    stopAsyncConnection();

    URL redisUrl;
    {
        const scoped_lock lock(m_mutex);
        if (!m_socket->active())
        {
            return false;
        }
        redisUrl = m_redisUrl;
    }

    try
    {
        const auto& [host, port] = redisUrl.hostAndPort();
        auto socket = make_shared<TCPSocket>();
        socket->host(Host(host, port));
        socket->open();
        socket->setOption(IPPROTO_TCP, TCP_NODELAY, 1);

        // The same handshake connect() makes, answered first on this connection - the database
        // included, or the asynchronous writes would land in database 0.
        const auto hello = helloCommand(redisUrl.username(), redisUrl.password(), redisUrl.path());
        const auto database = databaseOf(redisUrl.path());

        {
            const scoped_lock lock(m_inFlightMutex);
            m_asyncBroken = false;
            m_inFlight.push_back({{}, false});
            if (database >= 0)
            {
                m_inFlight.push_back({{}, false});
            }
        }
        {
            const scoped_lock lock(m_asyncSocketMutex);
            m_asyncSocket = socket;
        }
        m_asyncSendBuffer.bytes(0);
        appendRequest(m_asyncSendBuffer, hello);
        if (database >= 0)
        {
            appendRequest(m_asyncSendBuffer, RedisCommand("SELECT", to_string(database)));
        }
        m_asyncReader = JoiningThread([this, socket]
                                      {
                                          readReplies(socket);
                                      });
        return true;
    }
    catch (const Exception& e)
    {
        reportAsyncError(e);
        const scoped_lock lock(m_inFlightMutex);
        m_inFlight.clear();
        return false;
    }
}

void RedisConnect::stopAsyncConnection()
{
    shared_ptr<TCPSocket> socket;
    {
        const scoped_lock lock(m_asyncSocketMutex);
        socket = std::move(m_asyncSocket);
    }
    if (socket)
    {
        shutdownSocket(*socket);
    }
    m_asyncReader.join();
    if (socket)
    {
        socket->close();
    }
    m_asyncSendBuffer.bytes(0);
}

void RedisConnect::sendAsyncRequests()
{
    if (m_asyncSendBuffer.empty())
    {
        return;
    }

    shared_ptr<TCPSocket> socket;
    {
        const scoped_lock lock(m_asyncSocketMutex);
        socket = m_asyncSocket;
    }

    try
    {
        if (!socket)
        {
            throw RedisConnectException("Not connected");
        }
        socket->write(m_asyncSendBuffer);
    }
    catch (const Exception&)
    {
        // The reader owns failing what is unanswered, including these: it finds the connection
        // shut down and fails every command registered for it.
        if (socket)
        {
            shutdownSocket(*socket);
        }
    }
    m_asyncSendBuffer.bytes(0);
}

void RedisConnect::waitForInFlight(const size_t maxRemaining)
{
    sendAsyncRequests();

    unique_lock lock(m_inFlightMutex);
    m_inFlightChanged.wait(lock, [this, maxRemaining]
                           {
                               return m_inFlight.size() <= maxRemaining || m_asyncBroken || m_workerStop;
                           });
}

void RedisConnect::readReplies(const shared_ptr<TCPSocket>& socket)
{
    constexpr size_t readSize = 64 * 1024;

    Buffer          stream;
    size_t          parsed = 0;
    string          failure = "Redis connection closed";
    vector<Variant> reply;
    string          error;

    struct Parsed
    {
        vector<Variant> reply;
        string          error;
    };
    vector<Parsed> complete;

    try
    {
        while (true)
        {
            stream.reserve(stream.bytes() + readSize);
            const auto received = socket->read(stream.data() + stream.bytes(), readSize);
            if (received == 0)
            {
                break;
            }
            stream.bytes(stream.bytes() + received);

            const string_view data(stream.c_str(), stream.bytes());
            while (true)
            {
                reply.clear();
                error.clear();
                if (!parseReply(data, parsed, reply, error))
                {
                    break;
                }
                complete.push_back({std::move(reply), std::move(error)});
            }

            if (!complete.empty())
            {
                // One lock for everything this read completed, not one per reply.
                vector<InFlightCommand> commands;
                commands.reserve(complete.size());
                {
                    const scoped_lock lock(m_inFlightMutex);
                    if (m_inFlight.size() < complete.size())
                    {
                        throw RedisConnectException("Redis sent a reply to no request");
                    }
                    for (size_t i = 0; i < complete.size(); ++i)
                    {
                        commands.push_back(std::move(m_inFlight.front()));
                        m_inFlight.pop_front();
                    }
                }
                m_inFlightChanged.notify_all();

                for (size_t i = 0; i < complete.size(); ++i)
                {
                    auto& command = commands[i];
                    if (!complete[i].error.empty())
                    {
                        reportAsyncError(RedisConnectException(complete[i].error));
                    }
                    else if (command.onReply)
                    {
                        try
                        {
                            command.onReply(complete[i].reply);
                        }
                        catch (const Exception&)
                        {
                            // A failing callback must not prevent completion bookkeeping for this or later tasks.
                        }
                    }
                    if (command.counted)
                    {
                        taskCompleted();
                    }
                }
                complete.clear();
            }

            // Keep only the start of a reply that has not fully arrived yet.
            if (parsed != 0)
            {
                const auto remaining = stream.bytes() - parsed;
                memmove(stream.data(), stream.data() + parsed, remaining);
                stream.bytes(remaining);
                parsed = 0;
            }
        }
    }
    catch (const Exception& e)
    {
        failure = e.what();
    }

    failInFlight(failure);
}

void RedisConnect::failInFlight(const string& reason)
{
    deque<InFlightCommand> failed;
    {
        const scoped_lock lock(m_inFlightMutex);
        m_asyncBroken = true;
        failed.swap(m_inFlight);
    }
    m_inFlightChanged.notify_all();

    for (const auto& command: failed)
    {
        // Nobody is waiting for these once the object is being destroyed; the destructor drops
        // queued operations, and reporting each one would only fill the log on shutdown.
        if (!m_workerStop)
        {
            reportAsyncError(RedisConnectException(reason));
        }
        if (command.counted)
        {
            taskCompleted();
        }
    }
}

bool RedisConnect::parseReply(const string_view data, size_t& position, vector<Variant>& results, string& error)
{
    const auto lineEnd = data.find("\r\n", position);
    if (lineEnd == string_view::npos)
    {
        return false;
    }

    const auto       type = data[position];
    const string_view payload = data.substr(position + 1, lineEnd - position - 1);
    auto             next = lineEnd + 2;

    const auto readCount = [&payload]
    {
        int64_t count {0};
        from_chars(payload.data(), payload.data() + payload.size(), count);
        return count;
    };

    switch (type)
    {
        case '+': // Simple String
            results.emplace_back(payload);
            break;

        case '-': // Error
            error = payload;
            break;

        case ':': {
            // Integer - int, as readResponse() delivers it
            int value {0};
            from_chars(payload.data(), payload.data() + payload.size(), value);
            results.emplace_back(value);
            break;
        }

        case '=':   // Verbatim String (RESP3)
        case '$':   // Bulk String
        case '!': { // Blob Error (RESP3)
            const auto length = readCount();
            if (length == -1)
            {
                results.emplace_back(); // Null
                break;
            }
            if (data.size() < next + static_cast<size_t>(length) + 2)
            {
                return false;
            }
            auto value = data.substr(next, static_cast<size_t>(length));
            next += static_cast<size_t>(length) + 2;
            if (type == '!')
            {
                error = value.empty() ? "Redis error" : string(value);
                break;
            }
            // See readResponse(): the "txt:" style format marker of a verbatim string is dropped.
            if (constexpr size_t formatMarkerLength = 4;
                type == '=' && value.size() >= formatMarkerLength && value[3] == ':')
            {
                value.remove_prefix(formatMarkerLength);
            }
            results.emplace_back(Buffer(value.data(), value.size()));
            break;
        }

        case '*':   // Array
        case '~': { // Set (RESP3)
            const auto count = readCount();
            if (count == -1)
            {
                results.emplace_back();
                break;
            }
            for (int64_t i = 0; i < count; ++i)
            {
                if (!parseReply(data, next, results, error))
                {
                    return false;
                }
            }
            break;
        }

        case '%': { // Map (RESP3)
            const auto count = readCount();
            for (int64_t i = 0; i < count * 2; ++i)
            {
                if (!parseReply(data, next, results, error))
                {
                    return false;
                }
            }
            break;
        }

        case '(': // Big number (RESP3), delivered as text - it may not fit an integer
            results.emplace_back(string(payload));
            break;

        case '_': // Null (RESP3)
            results.emplace_back();
            break;

        case '#': // Boolean (RESP3)
            results.emplace_back(payload == "t");
            break;

        case ',': // Double (RESP3) - strtod for the reason given in readResponse()
            results.emplace_back(strtod(string(payload).c_str(), nullptr));
            break;

        default:
            // The stream cannot be realigned after this; the reader fails the connection.
            throw RedisConnectException("Unknown response type: " + string(1, type));
    }

    position = next;
    return true;
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

namespace {

/// Reply handler for a command that answers with one integer - a count - as DEL, SADD and SREM do.
function<void(vector<Variant>&)> countReply(const char* commandName, RedisConnect::ResultCallback<size_t> callback)
{
    return [commandName, callback = std::move(callback)](const vector<Variant>& results)
    {
        if (results.empty())
        {
            throw RedisConnectException(string("Unexpected empty response from ") + commandName + " command");
        }
        if (callback)
        {
            callback(static_cast<size_t>(results[0].asInt64()));
        }
    };
}

} // namespace

void RedisConnect::deleteKeysAsync(const vector<string>& keys, ResultCallback<size_t> callback)
{
    if (keys.empty())
    {
        // Nothing to send, but the answer still comes from the worker, in queue order, as it did.
        enqueue([callback = std::move(callback)]
                {
                    if (callback)
                    {
                        callback(0);
                    }
                });
        return;
    }

    // Pipelined, not self-contained: a self-contained task waits for everything in flight and then
    // makes a round trip of its own, which stalls every command queued behind it - and with
    // appendfsync always, each such round trip also waits for the disk.
    RedisCommand command("DEL");
    command.emplace_back(keys);
    enqueueCommand(std::move(command), countReply("DEL", std::move(callback)));
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
    if (members.empty())
    {
        enqueue([callback = std::move(callback)]
                {
                    if (callback)
                    {
                        callback(0);
                    }
                });
        return;
    }

    // Pipelined - see deleteKeysAsync().
    RedisCommand command("SADD", key);
    command.emplace_back(members);
    enqueueCommand(std::move(command), countReply("SADD", std::move(callback)));
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
    if (members.empty())
    {
        enqueue([callback = std::move(callback)]
                {
                    if (callback)
                    {
                        callback(0);
                    }
                });
        return;
    }

    // Pipelined - see deleteKeysAsync().
    RedisCommand command("SREM", key);
    command.emplace_back(members);
    enqueueCommand(std::move(command), countReply("SREM", std::move(callback)));
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
