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
#include <iostream>
#include <sstream>

using namespace std;
using namespace sptk;

namespace sptk {
void RedisConnect::connect(const std::string& host, const int port)
{
    m_socket->host(Host(host.c_str(), static_cast<uint16_t>(port)));
    m_socket->open();
    m_reader = make_unique<SocketReader>(m_socket);

    m_socket->write("HELLO 3\r\n");
    (void) readResponse();
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

void RedisConnect::set(const std::string& key, const Variant& value)
{
    if (!m_socket->active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    auto setKey = format("*3\r\n$3\r\nSET\r\n${}\r\n{}\r\n", key.length(), key);

    auto isBinary = false;
    switch (value.dataType())
    {
        using enum VariantDataType;
        case VAR_BOOL: {
            const string& s = value.asBool() ? "true" : "false";
            setKey += format("${}\r\n{}", s.length(), s);
            break;
        }
        case VAR_INT:
        case VAR_INT64: {
            const string& s = to_string(value.asInt64());
            setKey += format("${}\r\n{}", s.length(), s);
            break;
        }
        case VAR_FLOAT:
        case VAR_DATE:
        case VAR_DATE_TIME: {
            stringstream fss;
            fss.precision(17); // Enough precision for IEEE 754 double
            fss << value.asFloat();
            const string& s = fss.str();
            setKey += format("${}\r\n{}", s.length(), s);
            break;
        }
        case VAR_STRING:
        case VAR_TEXT: {
            const string& s = value.asString();
            setKey += format("${}\r\n{}", s.length(), s);
            break;
        }
        case VAR_BUFFER: {
            const Buffer& buffer = value.asBuffer();
            setKey += format("${}\r\n", buffer.size());
            m_socket->write(setKey);
            m_socket->write(buffer.data(), buffer.size());
            m_socket->write("\r\n");
            isBinary = true;
            break;
        }
        case VAR_NONE:
            setKey += "_";
            break;
        default:
            throw Exception("Redis: Unsupported variant type");
    }

    if (!isBinary)
    {
        setKey += "\r\n";
        m_socket->write(setKey);
    }

    (void) readResponse();
}

void RedisConnect::setBinary(const std::string& key, const Buffer& value)
{
    set(key, Variant(value));
}

Variant RedisConnect::get(const std::string& key) const
{
    if (!m_socket->active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    stringstream ss;
    ss << "*2\r\n"
       << "$3\r\nGET\r\n"
       << "$" << key.length() << "\r\n"
       << key << "\r\n";

    m_socket->write(ss.str());

    return readResponse();
}

Buffer RedisConnect::getBinary(const std::string& key)
{
    return get(key).asBuffer();
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

Variant RedisConnect::readResponse() const
{
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
            return {payload.c_str()};
        case '-': // Error
            throw Exception("Redis error: " + payload);
        case ':': // Integer
            return {static_cast<int64_t>(stoll(payload)), 0u};
        case '$': { // Bulk String
            const auto len = stoi(payload);
            if (len == -1)
            {
                return {}; // Null
            }
            Buffer buffer;
            Buffer trailing;
            m_reader->read(buffer, len);
            m_reader->read(trailing, 2); // Read exactly \r\n
            return {buffer};
        }
        case '*': { // Array
            const auto count = stoi(payload);
            if (count == -1)
            {
                return {};
            }
            // For now, we only support simple responses, but we must consume the array
            for (auto i = 0; i < count; ++i)
            {
                (void) readResponse();
            }
            return {};
        }
        case '_': // Null (RESP3)
            return {};
        case '#': // Boolean (RESP3)
            return {payload == "t"};
        case ',': // Double (RESP3)
            return {stod(payload)};
        case '%': { // Map (RESP3)
            const auto count = stoi(payload);
            for (int i = 0; i < count; ++i)
            {
                (void) readResponse(); // Key
                (void) readResponse(); // Value
            }
            return {};
        }
        default:
            throw Exception("Redis: Unknown response type: " + string(1, type));
    }
}

} // namespace sptk
