/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-30                                             ║
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
#include "sptk5/Exception.h"
#include "sptk5/Printer.h"

using namespace std;
using namespace sptk;

vector<Variant> RedisConnect::connect(const string& host, int port,
                                      const string& username, const string& password, const string& clientName)
{
    m_socket->host(Host(host.c_str(), static_cast<uint16_t>(port)));
    m_socket->open();
    m_reader = make_unique<SocketReader>(m_socket);

    // Hello command in inline format.
    string helloCommand = "HELLO 3";

    if (!username.empty() && !password.empty())
    {
        helloCommand += " AUTH " + username + " " + password;
    }

    if (!clientName.empty())
    {
        helloCommand += " SETNAME " + clientName;
    }

    m_socket->write(helloCommand + "\r\n");

    vector<Variant> results;
    readResponse(results);

    return results;
}

bool RedisConnect::isConnected() const
{
    return m_socket->active();
}

void RedisConnect::disconnect()
{
    if (m_socket->active())
    {
        m_socket->close();
    }
    m_reader.reset();
}

void RedisConnect::set(const std::string& key, const Variant& value) const
{
    vector<string_view> commandWords;
    commandWords.emplace_back("SET");
    commandWords.push_back(key);

    vector<Variant> results;

    // Note: commandWords uses string_view, so local vars need to be used before going out of scope.
    switch (value.dataType())
    {
        using enum VariantDataType;
        case VAR_BOOL: {
            const string s = value.asBool() ? "true" : "false";
            commandWords.emplace_back(s.c_str(), s.length());
            executeCommand(commandWords, results);
            break;
        }

        case VAR_INT:
        case VAR_INT64:
        case VAR_FLOAT: {
            const auto s = value.asString();
            commandWords.emplace_back(s.c_str(), s.length());
            executeCommand(commandWords, results);
            break;
        }

        case VAR_DATE: {
            const auto s = value.asDate().dateString();
            commandWords.emplace_back(s.c_str(), s.length());
            executeCommand(commandWords, results);
            break;
        }

        case VAR_DATE_TIME: {
            const auto s = value.asDateTime().isoDateTimeString();
            commandWords.emplace_back(s.c_str(), s.length());
            executeCommand(commandWords, results);
            break;
        }

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER: {
            commandWords.emplace_back(value.getString(), value.dataSize());
            executeCommand(commandWords, results);
            break;
        }

        case VAR_NONE:
            commandWords.emplace_back("_", 1);
            executeCommand(commandWords, results);
            break;

        default:
            throw Exception("Redis: Unsupported variant type");
    }
}

std::vector<Variant> RedisConnect::scan(const std::string& pattern, size_t limit) const
{
    std::vector<Variant> results;

    size_t cursor = 0;
    do
    {
        std::vector<Variant> iterationResults;
        cursor = scan(pattern, cursor, iterationResults, 1000000);
        results.reserve(results.size() + iterationResults.size());
        ranges::copy(iterationResults, back_inserter(results));
    } while (cursor != 0 && results.size() < limit);

    if (results.size() > limit)
    {
        results.resize(limit);
    }

    return results;
}

size_t RedisConnect::remove(const std::vector<std::string>& keys) const
{
    vector<Variant>     results;
    vector<string_view> commandWords = {"DEL"};
    for (const auto& key: keys)
    {
        commandWords.emplace_back(key.c_str(), key.length());
    }
    executeCommand(commandWords, results);
    const auto keysRemoved = results[0].asInteger();
    return keysRemoved;
}

size_t RedisConnect::scan(const std::string& pattern, const size_t cursor, std::vector<Variant>& matchedKeys, size_t limit) const
{
    const auto          cursorStr = to_string(cursor);
    const auto          countStr = to_string(limit);
    vector<string_view> commandWords = {"SCAN", cursorStr, "MATCH", pattern};
    if (limit != 0)
    {
        commandWords.emplace_back("COUNT");
        commandWords.push_back(countStr);
    }

    executeCommand(commandWords, matchedKeys);

    if (!matchedKeys.empty())
    {
        const auto newCursor = matchedKeys[0].asInteger();
        matchedKeys.erase(matchedKeys.begin());
        return newCursor;
    }

    return 0;
}

Variant RedisConnect::get(const std::string& key) const
{
    const vector<string_view> commandWords {"GET", key};
    vector<Variant>           results;
    executeCommand(commandWords, results);
    if (results.empty())
    {
        return {};
    }

    return results[0];
}

std::vector<Variant> RedisConnect::mget(const std::vector<std::string>& keys) const
{
    vector<string_view> commandWords {"MGET"};
    commandWords.reserve(keys.size() + 1);
    for (const auto& key: keys)
    {
        commandWords.emplace_back(key.c_str(), key.length());
    }
    vector<Variant> results;
    executeCommand(commandWords, results);
    return results;
}

void RedisConnect::executeCommand(const vector<string_view>& commandElements, vector<Variant>& results) const
{
    if (!m_socket->active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    if (commandElements.empty())
    {
        throw Exception("Redis: Empty command data");
    }

    size_t expectedLength = 0;
    for (const auto& commandElement: commandElements)
    {
        expectedLength += commandElement.size() + 10;
    }

    Buffer buffer(expectedLength);
    buffer.append(format("*{}", commandElements.size()));

    for (const auto& commandElement: commandElements)
    {
        buffer.append(format("\r\n${}\r\n", commandElement.size()));
        buffer.append(reinterpret_cast<const uint8_t*>(commandElement.data()), commandElement.size());
    }
    buffer.append(reinterpret_cast<const uint8_t*>("\r\n"), 2);
    m_socket->write(buffer.data(), buffer.bytes());

    readResponse(results);
}

String RedisConnect::readLine() const
{
    String line;
    if (m_reader->readLine(line) == 0)
    {
        if (!m_reader->readyToRead(10s))
        {
            throw Exception("Redis: Server read timeout");
        }
        m_reader->readLine(line);
    }

    // Remove \r from the end if present
    if (!line.empty() && line.back() == '\r')
    {
        line.resize(line.size() - 1);
    }

    return line;
}

void RedisConnect::readResponse(vector<Variant>& results) const
{
    const auto line = readLine();
    if (line.empty())
    {
        throw Exception("Redis: Empty response");
    }

    const auto        type = line[0];
    const string_view payload {line.c_str() + 1, line.size() - 1};

    switch (type)
    {
        case '+': // Simple String
            results.emplace_back(payload);
            return;

        case '-': // Error
            throw Exception("Redis error: " + string(payload));

        case ':': // Integer
            results.emplace_back(atoll(payload.data()), 0u);
            return;

        case '$': { // Bulk String
            const auto len = atol(payload.data());
            if (len == -1)
            {
                results.emplace_back(); // Null
                return;
            }
            Buffer buffer;
            Buffer trailing;
            m_reader->read(buffer, len + 2);  // Also read \r\n
            buffer.bytes(buffer.bytes() - 2); // Cut off \r\n
            results.emplace_back(buffer);
            return;
        }
        case '*': { // Array
            const auto count = atol(payload.data());
            if (count == -1)
            {
                results.emplace_back();
                return;
            }
            // For read the array
            for (auto i = 0; i < count; ++i)
            {
                readResponse(results);
            }
            return;
        }
        case '_':                   // Null (RESP3)
            results.emplace_back(); // Null
            return;
        case '#': // Boolean (RESP3)
            results.emplace_back(payload == "t");
            return;
        case ',': // Double (RESP3)
            results.emplace_back(strtod(payload.data(), nullptr));
            return;
        case '%': { // Map (RESP3)
            const auto count = atol(payload.data());
            for (auto i = 0; i < count; ++i)
            {
                vector<Variant> mapValues;
                readResponse(mapValues); // Key
                readResponse(mapValues); // Value
                string mapValue;
                switch (mapValues.size())
                {
                    case 1:
                        mapValue = format("[{}]:", mapValues[0].asString().c_str());
                        break;
                    case 2:
                        mapValue = format("[{}]: {}", mapValues[0].asString().c_str(), mapValues[1].asString().c_str());
                        break;
                    default:
                        break;
                }
                if (!mapValue.empty())
                {
                    results.emplace_back(mapValue);
                }
            }
            return;
        }
        default:
            throw Exception("Redis: Unknown response type: " + string(1, type));
    }
}
