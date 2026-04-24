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

vector<Variant> RedisConnect::connect(const string& host, int port)
{
    m_socket->host(Host(host.c_str(), static_cast<uint16_t>(port)));
    m_socket->open();
    m_reader = make_unique<SocketReader>(m_socket);

    m_socket->write("HELLO 3\r\n");

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
    if (!m_socket->active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    auto setKey = format("SET '{}'", key);

    vector<string_view> commandWords;
    commandWords.emplace_back("SET");
    commandWords.push_back(key);

    // Note: commandWords uses string_view, so local vars need to be used before going out of scope.
    switch (value.dataType())
    {
        using enum VariantDataType;
        case VAR_BOOL: {
            const string s = value.asBool() ? "true" : "false";
            commandWords.emplace_back(s.c_str(), s.length());
            sendCommand(commandWords);
            break;
        }

        case VAR_INT:
        case VAR_INT64:
        case VAR_FLOAT: {
            const auto s = value.asString();
            commandWords.emplace_back(s.c_str(), s.length());
            sendCommand(commandWords);
            break;
        }

        case VAR_DATE: {
            const auto s = value.asDate().dateString();
            commandWords.emplace_back(s.c_str(), s.length());
            sendCommand(commandWords);
            break;
        }

        case VAR_DATE_TIME: {
            const auto s = value.asDateTime().isoDateTimeString();
            commandWords.emplace_back(s.c_str(), s.length());
            sendCommand(commandWords);
            break;
        }

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER: {
            commandWords.emplace_back(value.getString(), value.dataSize());
            sendCommand(commandWords);
            break;
        }

        case VAR_NONE:
            commandWords.emplace_back("_", 1);
            sendCommand(commandWords);
            break;

        default:
            throw Exception("Redis: Unsupported variant type");
    }

    vector<Variant> results;
    readResponse(results);
}

void RedisConnect::setBinary(const std::string& key, const Buffer& value) const
{
    set(key, Variant(value));
}

size_t RedisConnect::scan(const std::string& pattern, size_t cursor, std::vector<Variant>& matchedKeys, size_t limit) const
{
    if (!m_socket->active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    const auto          cursorStr = to_string(cursor);
    const auto          countStr = to_string(limit);
    vector<string_view> commandWords = {"SCAN", cursorStr, "MATCH", pattern};
    if (limit != 0)
    {
        commandWords.emplace_back("COUNT");
        commandWords.push_back(countStr);
    }
    sendCommand(commandWords);

    readResponse(matchedKeys);

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
    if (!m_socket->active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    const vector<string_view> commandWords {"GET", key};
    sendCommand(commandWords);

    vector<Variant> results;
    readResponse(results);

    if (results.empty())
    {
        return {};
    }

    return results[0];
}

Buffer RedisConnect::getBinary(const std::string& key) const
{
    return get(key).asBuffer();
}

void RedisConnect::sendCommand(const vector<string_view>& commandElements) const
{
    if (commandElements.empty())
    {
        throw Exception("Redis: Empty command data");
    }

    Buffer buffer;
    buffer.append(format("*{}\r\n", commandElements.size()));

    for (const auto& commandElement: commandElements)
    {
        buffer.append(format("${}\r\n", commandElement.size()));
        buffer.append(reinterpret_cast<const uint8_t*>(commandElement.data()), commandElement.size());
        buffer.append(reinterpret_cast<const uint8_t*>("\r\n"), 2);
    }
    m_socket->write(buffer.data(), buffer.bytes());
}

string RedisConnect::readLine() const
{
    String line;
    m_reader->readLine(line);
    // Remove \r from the end if present
    string s = line.c_str();
    if (!s.empty() && s.back() == '\r')
    {
        s.pop_back();
    }
    return s;
}

void RedisConnect::readResponse(vector<Variant>& results) const
{
    static Variant nullVariant;

    const string line = readLine();
    if (line.empty())
    {
        throw Exception("Redis: Empty response");
    }

    const char   type = line[0];
    const string payload = line.substr(1);

    switch (type)
    {
        case '+': // Simple String
            results.emplace_back(payload);
            return;

        case '-': // Error
            throw Exception("Redis error: " + payload);

        case ':': // Integer
            results.emplace_back(stoll(payload), 0u);
            return;

        case '$': { // Bulk String
            const auto len = stoi(payload);
            if (len == -1)
            {
                results.emplace_back(nullVariant); // Null
                return;
            }
            Buffer buffer;
            Buffer trailing;
            m_reader->read(buffer, len);
            m_reader->read(trailing, 2); // Read exactly \r\n
            results.emplace_back(buffer);
            return;
        }
        case '*': { // Array
            const auto count = stoi(payload);
            if (count == -1)
            {
                results.emplace_back(nullVariant);
                return;
            }
            // For now, we only support simple responses, but we must consume the array
            for (auto i = 0; i < count; ++i)
            {
                readResponse(results);
            }
            return;
        }
        case '_':                              // Null (RESP3)
            results.emplace_back(nullVariant); // Null
            return;
        case '#': // Boolean (RESP3)
            results.emplace_back(payload == "t");
            return;
        case ',': // Double (RESP3)
            results.emplace_back(stod(payload));
            return;
        case '%': { // Map (RESP3)
            const auto count = stoi(payload);
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
