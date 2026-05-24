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
#include <list>

using namespace std;
using namespace sptk;

vector<Variant> RedisConnect::connect(const string& host, int port,
                                      const string& username, const string& password, const string& clientName)
{
    scoped_lock lock(m_mutex);

    if (m_socket->active())
    {
        throw RedisConnectException("Already connected, please disconnect, first.");
    }

    m_socket->host(Host(host.c_str(), static_cast<uint16_t>(port)));
    m_socket->open();
    m_reader = make_unique<SocketReader>(m_socket);

    vector<string> commandWords {"HELLO", "3"};

    if (!username.empty() && !password.empty())
    {
        commandWords.emplace_back("AUTH");
        commandWords.emplace_back(username);
        commandWords.emplace_back(password);
    }

    if (!clientName.empty())
    {
        commandWords.emplace_back("SETNAME");
        commandWords.emplace_back(clientName);
    }

    vector<Variant> results;
    executeCommand(commandWords, results);

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

void RedisConnect::set(const string& key, const Variant& value)
{
    scoped_lock lock(m_mutex);

    Command command {"SET", key};
    command.push_back(serialize(value));

    vector<Variant> results;
    executeCommand(command, results);
}

string RedisConnect::serialize(const Variant& value)
{
    switch (value.dataType())
    {
        using enum VariantDataType;
        case VAR_BOOL:
            return value.asBool() ? "true" : "false";

        case VAR_INT:
        case VAR_INT64:
        case VAR_FLOAT:
            return value.asString();

        case VAR_DATE:
            return format("{:%F}", value.asDate().timePoint());

        case VAR_DATE_TIME:
            return format("{:%F %T}", value.asDate().timePoint());

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return {value.getString(), value.dataSize()};

        case VAR_NONE:
            return {"_", 1};

        default:
            throw RedisConnectException("Unsupported variant type");
    }
}

void RedisConnect::mset(const KeysAndValues& keysAndValues)
{
    if (keysAndValues.empty())
    {
        return;
    }

    scoped_lock lock(m_mutex);

    Command command;
    command.emplace_back("MSET");
    command.reserve(keysAndValues.size() * 2 + 1);

    for (const auto& [key, value]: keysAndValues)
    {
        command.push_back(key);
        command.push_back(serialize(value));
    }

    vector<Variant> results;
    executeCommand(command, results);
}

void RedisConnect::hset(const string& hash, const string& key, const Variant& value)
{
    scoped_lock lock(m_mutex);

    Command command {"HSET", hash};

    command.push_back(key);
    command.push_back(serialize(value));

    vector<Variant> results;
    executeCommand(command, results);
}

vector<string> RedisConnect::scan(const string& pattern, size_t limit)
{
    scoped_lock lock(m_mutex);

    vector<string> results;

    size_t cursor = 0;
    do
    {
        list<Variant> iterationResults;
        cursor = scan(pattern, cursor, iterationResults, limit);
        results.reserve(results.size() + iterationResults.size());
        ranges::transform(iterationResults, back_inserter(results), [](const auto& result)
                          {
                              return result.asString();
                          });
    } while (cursor != 0 && results.size() < limit);

    if (results.size() > limit)
    {
        results.resize(limit);
    }

    return results;
}

size_t RedisConnect::remove(const vector<string>& keys)
{
    if (keys.empty())
    {
        return 0;
    }

    scoped_lock lock(m_mutex);

    vector<Variant> results;
    vector<string>  commandWords = {"DEL"};

    commandWords.reserve(keys.size() + 1);
    ranges::copy(keys, back_inserter(commandWords));

    executeCommand(commandWords, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from DEL command");
    }

    const auto keysRemoved = results[0].asInteger();
    return keysRemoved;
}

int64_t RedisConnect::incr(const string& key)
{
    scoped_lock lock(m_mutex);

    const vector<string> commandWords = {"INCR", key};
    vector<Variant>      results;

    executeCommand(commandWords, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from INCR command");
    }

    return results[0].asInt64();
}

void RedisConnect::rename(const string& oldKey, const string& newKey)
{
    scoped_lock lock(m_mutex);

    const vector<string> commandWords = {"RENAME", oldKey, newKey};
    vector<Variant>      results;

    executeCommand(commandWords, results);
}

bool RedisConnect::renameNX(const string& oldKey, const string& newKey)
{
    scoped_lock lock(m_mutex);

    const vector<string> commandWords = {"RENAMENX", oldKey, newKey};
    vector<Variant>      results;

    executeCommand(commandWords, results);

    if (results.empty())
    {
        throw RedisConnectException("Unexpected empty response from RENAMENX command");
    }

    return results[0].asInteger() == 1;
}

void RedisConnect::beginTransaction()
{
    scoped_lock lock(m_mutex);

    const Command   command = {"MULTI"};
    vector<Variant> results;

    executeCommand(command, results);
}

vector<Variant> RedisConnect::commitTransaction()
{
    scoped_lock lock(m_mutex);

    const Command   command = {"EXEC"};
    vector<Variant> results;

    executeCommand(command, results);

    return results;
}

void RedisConnect::rollbackTransaction()
{
    scoped_lock lock(m_mutex);

    const Command   command = {"DISCARD"};
    vector<Variant> results;

    executeCommand(command, results);
}

void RedisConnect::hset(const string& hash, const KeysAndValues& keysAndValues)
{
    if (keysAndValues.empty())
    {
        return;
    }

    scoped_lock lock(m_mutex);

    Command command {"HSET", hash};
    command.reserve(keysAndValues.size() * 2 + 2);

    for (const auto& [key, value]: keysAndValues)
    {
        command.push_back(key);
        command.push_back(serialize(value));
    }

    vector<Variant> results;
    executeCommand(command, results);
}

vector<string> RedisConnect::hkeys(const string& hashName)
{
    scoped_lock lock(m_mutex);

    const vector<string> commandWords {"HKEYS", hashName};
    vector<Variant>      results;
    executeCommand(commandWords, results);

    vector<string> keys;
    keys.reserve(results.size());
    ranges::transform(results, back_inserter(keys), [](const Variant& v)
                      {
                          return v.asString();
                      });
    return keys;
}

RedisConnect::KeysAndValues RedisConnect::hmget(const string& hash, const vector<string>& keys)
{
    scoped_lock lock(m_mutex);

    vector<string> commandWords {"HMGET", hash};
    commandWords.reserve(keys.size() + 2);
    ranges::copy(keys, back_inserter(commandWords));

    vector<Variant> results;
    KeysAndValues   output;
    output.reserve(keys.size());

    executeCommand(commandWords, results);

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

RedisConnect::KeysAndValues RedisConnect::hgetall(const string& hash)
{
    scoped_lock lock(m_mutex);

    const vector<string> commandWords {"HGETALL", hash};
    vector<Variant>      results;
    executeCommand(commandWords, results);

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

void RedisConnect::hdel(const string& hash, const vector<string>& keys)
{
    scoped_lock lock(m_mutex);

    vector<string> commandWords {"HDEL", hash};
    commandWords.reserve(keys.size() + 2);
    ranges::copy(keys, back_inserter(commandWords));

    vector<Variant> results;
    executeCommand(commandWords, results);
}

size_t RedisConnect::scan(const string& pattern, const size_t cursor, list<Variant>& matchedKeys, size_t limit)
{
    const auto     cursorStr = to_string(cursor);
    const auto     countStr = to_string(limit);
    vector<string> commandWords = {"SCAN", cursorStr, "MATCH", pattern};
    if (limit != 0)
    {
        commandWords.emplace_back("COUNT");
        commandWords.push_back(countStr);
    }

    executeCommand(commandWords, matchedKeys);

    if (matchedKeys.empty())
    {
        throw RedisConnectException("Unexpected empty response from SCAN command");
    }

    const auto newCursor = matchedKeys.front().asInteger();
    matchedKeys.erase(matchedKeys.begin());
    return newCursor;
}

Variant RedisConnect::get(const string& key)
{
    scoped_lock lock(m_mutex);

    const vector<string> commandWords {"GET", key};
    vector<Variant>      results;
    executeCommand(commandWords, results);
    if (results.empty())
    {
        return {};
    }

    return results[0];
}

RedisConnect::KeysAndValues RedisConnect::mget(const vector<string>& keys)
{
    scoped_lock lock(m_mutex);

    vector<string> commandWords {"MGET"};
    commandWords.reserve(keys.size() + 1);
    ranges::copy(keys, back_inserter(commandWords));

    vector<Variant> results;
    KeysAndValues   output;

    executeCommand(commandWords, results);

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

void RedisConnect::sendRequest(const Command& command) const
{
    size_t expectedLength = 10; // New line chars and number of elements as a string.
    for (const auto& commandElement: command)
    {
        expectedLength += commandElement.size() + 10;
    }

    Buffer buffer(expectedLength);
    buffer.append('*');
    buffer.append(to_string(command.size()));

    for (const auto& commandElement: command)
    {
        buffer.append("\r\n$", 3);
        buffer.append(to_string(commandElement.size()));
        buffer.append("\r\n", 2);
        buffer.append(reinterpret_cast<const uint8_t*>(commandElement.data()), commandElement.size());
    }
    buffer.append("\r\n", 2);
    m_socket->write(buffer.data(), buffer.bytes());
}

String RedisConnect::readLine()
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
    if (!m_readLineBuffer.empty() && m_readLineBuffer.back() == '\r')
    {
        m_readLineBuffer.resize(m_readLineBuffer.size() - 1);
    }

    return m_readLineBuffer;
}
