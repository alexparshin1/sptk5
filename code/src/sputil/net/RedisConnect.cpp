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

#include "RedisConnect.h"

#include "sptk5/Base64.h"
#include "sptk5/Exception.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace sptk;

namespace sptk {

void RedisConnect::connect(const std::string& host, const int port)
{
    m_socket.host(Host(host.c_str(), port));
    m_socket.open();
    m_reader = make_unique<SocketReader>(m_socket);

    m_socket.write("HELLO 6\r\n");
    String line;
    m_reader->readLine(line);
}

void RedisConnect::disconnect()
{
    if (m_socket.active())
    {
        m_socket.close();
    }
    m_reader.reset();
}

void RedisConnect::set(const std::string& key, const Variant& value)
{
    if (!m_socket.active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    stringstream ss;
    ss << "*3\r\n"
       << "$3\r\nSET\r\n"
       << "$" << key.length() << "\r\n"
       << key << "\r\n";

    switch (value.dataType())
    {
        using enum VariantDataType;
        case VAR_BOOL:
            ss << "#" << (value.asBool() ? 't' : 'f');
            break;
        case VAR_INT:
            ss << ":" << value.asInt64();
            break;
        case VAR_FLOAT:
            ss << "," << value.asFloat();
            break;
        case VAR_STRING: {
            const string& str = value.asString();
            ss << "$" << str.length() << "\r\n"
               << str;
            break;
        }
        case VAR_NONE:
            ss << "_";
            break;
        default:
            throw Exception("Redis: Unsupported variant type");
    }

    ss << "\r\n";

    const string cmd = ss.str();
    m_socket.write(cmd);

    if (const string response = readLine();
        response[0] == '-')
    {
        throw Exception("Redis error: " + response.substr(1));
    }
}

void RedisConnect::setBinary(const std::string& key, const Buffer& value)
{
    String encoded;
    Base64::encode(encoded, value);
    set(key, encoded);
}

Variant RedisConnect::get(const std::string& key)
{
    if (!m_socket.active())
    {
        throw Exception("RedisConnect: Not connected");
    }

    stringstream ss;
    ss << "*2\r\n"
       << "$3\r\nGET\r\n"
       << "$" << key.length() << "\r\n"
       << key << "\r\n";

    const string cmd = ss.str();
    m_socket.write(cmd);

    auto data = readBulkString();

    switch (data[0])
    {
        case '$':
        case '+':
            return Variant(data.substr(1));
        default: // Simple string
            return Variant(data);
    }
}

Buffer RedisConnect::getBinary(const std::string& key)
{
    Buffer decoded;
    Base64::decode(decoded, get(key).asString());
    return decoded;
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

string RedisConnect::readBulkString() const
{
    const string line = readLine();
    if (line.empty())
    {
        throw Exception("Redis: Empty response");
    }

    if (line[0] == '-')
    {
        throw Exception("Redis error: " + line.substr(1));
    }

    if (line[0] != '$')
    {
        throw Exception("Redis: Expected bulk string ($), got: " + line);
    }

    const auto len = stoi(line.substr(1));
    if (len == -1)
    {
        return ""; // Null bulk string
    }

    Buffer buffer;
    m_reader->read(buffer, len);
    // Read the trailing \r\n
    readLine();

    return string(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

} // namespace sptk
