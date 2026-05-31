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

namespace {
void bufferAppendCount(Buffer& buffer, size_t value)
{
    buffer.checkSize(buffer.size() + 24);
    char* start = reinterpret_cast<char*>(buffer.data()) + buffer.size();
    auto* end = std::to_chars(start, start + 24, value).ptr;
    *end = 0;
    buffer.bytes(reinterpret_cast<uint8_t*>(end) - buffer.data());
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

    m_socket->host(Host(host.c_str(), port));
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
        cursor = scan(pattern, cursor, iterationResults, limit);
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

    const RedisCommand command("MULTI");
    vector<Variant>    results;

    executeCommand(command, results);
}

vector<Variant> RedisConnect::commitTransaction()
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("EXEC");
    vector<Variant>    results;

    executeCommand(command, results);

    return results;
}

void RedisConnect::rollbackTransaction()
{
    scoped_lock lock(m_mutex);

    const RedisCommand command("DISCARD");

    vector<Variant> results;

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

void RedisConnect::sendRequest(const RedisCommand& command) const
{
    Buffer header("*", 1);
    bufferAppendCount(header, command.count());
    header.append("\r\n", 2);

    m_socket->write(header);
    m_socket->write(command);
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
    if (m_reader->readLine(m_readLineBuffer) == 0)
    {
        if (!m_reader->readyToRead(10s))
        {
            throw RedisConnectException("Server read timeout");
        }
        m_reader->readLine(m_readLineBuffer);
    }

    // Remove \r from the end if present
    if (const auto lastCharPos = m_readLineBuffer.size() - 1;
        !m_readLineBuffer.empty() && m_readLineBuffer[lastCharPos] == '\r')
    {
        m_readLineBuffer.bytes(lastCharPos);
        m_readLineBuffer[lastCharPos] = 0;
    }
}

void RedisConnect::readResponse(std::vector<Variant>& results, Variant* cursor)
{
    readLine();
    if (m_readLineBuffer.empty())
    {
        throw RedisConnectException("Empty response");
    }

    const auto             type = m_readLineBuffer[0];
    const std::string_view payload {m_readLineBuffer.c_str() + 1, m_readLineBuffer.size() - 1};

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

        case '$': { // Bulk String
            int64_t len {0};
            std::from_chars(payload.data(), payload.data() + payload.size(), len);
            if (len == -1)
            {
                results.emplace_back(); // Null
                return;
            }
            const auto readLength = len + 2;
            m_readLineBuffer.checkSize(readLength);
            m_reader->read(m_readLineBuffer, readLength);         // Also read \r\n
            m_readLineBuffer.bytes(m_readLineBuffer.bytes() - 2); // Cut off \r\n
            if (cursor)
            {
                *cursor = m_readLineBuffer;
            }
            else
            {
                results.emplace_back(m_readLineBuffer);
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
