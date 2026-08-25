/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-06                                             ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/net/URL.h>

#include <format>
#include <sstream> // libc++

using namespace std;
using namespace sptk;

URL::URL(const string& url)
{
    if (url.empty())
    {
        return;
    }

    static const RegularExpression matchUrl(R"(^((\w+)://)?(([^:]+)(:\S*)?@)?([\w:\-\.\[\]]+)?(/[^?\s]*)?(\?\S+)?$)");

    try
    {
        const auto matches = matchUrl.m(url);
        if (!matchUrl.m(url))
        {
            throw Exception("Wrong URL format");
        }

        const auto& groups = matches.groups();

        if (groups.size() <= 1)
        {
            throw Exception("No hostname, IP address, or path");
        }
        m_protocol = groups[1].value;

        if (groups.size() <= 3)
        {
            throw Exception("No hostname or IP address");
        }
        m_username = groups[3].value;

        if (groups.size() <= 4)
        {
            throw Exception("No hostname or IP address");
        }
        if (const auto password = groups[4].value;
            !password.empty())
        {
            m_password = password.substr(1);
        }

        if (groups.size() <= 5)
        {
            throw Exception("No hostname or IP address");
        }
        m_hostAndPort = groups[5].value;

        if (groups.size() <= 6)
        {
            return;
        }
        m_path = groups[6].value;

        if (groups.size() <= 7)
        {
            return;
        }

        if (const auto params = groups[7].value;
            !params.empty())
        {
            if (const Buffer buffer(params.substr(1));
                !buffer.empty())
            {
                m_params.decode(buffer);
            }
        }
    }
    catch (const Exception& e)
    {
        throw Exception(format("Invalid URL: {}", e.what()));
    }
}

URL::URL(const std::string& protocol, const std::string& host, const uint16_t port, const std::string& username, const std::string& password, const std::string& path)
    : m_protocol(protocol)
    , m_username(username)
    , m_password(password)
    , m_hostAndPort(port == 0 ? host : host + ":" + std::to_string(port))
    , m_path(path)
{
}

string URL::toString() const
{
    stringstream str;

    if (!m_protocol.empty())
    {
        str << m_protocol << "://";
    }

    if (!m_username.empty())
    {
        str << m_username;
        if (!m_password.empty())
        {
            str << ":" << m_password;
        }
        str << "@";
    }

    str << m_hostAndPort;

    if (!m_path.empty())
    {
        str << m_path;
    }

    if (!m_params.empty())
    {
        Buffer buffer;
        m_params.encode(buffer);
        str << "?" << buffer.c_str();
    }

    return str.str();
}

std::tuple<string, uint16_t> URL::hostAndPort() const
{
    static const RegularExpression matchHostAndPort(R"(^(\S+):(\d+)$)");
    const auto                     colonCount = ranges::count(m_hostAndPort, ':');
    if (colonCount == 0)
    {
        // No port specified, return default port 0.
        return std::make_tuple(m_hostAndPort, static_cast<uint16_t>(0));
    }

    if (colonCount == 1)
    {
        // Hostname or IPv4 address and port.
        const auto matches = matchHostAndPort.m(m_hostAndPort);
        if (matches.empty())
        {
            throw Exception("Invalid IP address");
        }
        return std::make_tuple(matches[0].value, static_cast<uint16_t>(stoi(matches[1].value)));
    }

    string   host = m_hostAndPort;
    uint16_t port = 0;
    if (m_hostAndPort.starts_with("["))
    {
        // IPv6 address with port.
        const auto matches = matchHostAndPort.m(m_hostAndPort);
        if (matches.empty())
        {
            throw Exception("Invalid IPv6 address");
        }

        host = matches[0].value;
        port = static_cast<uint16_t>(stoi(matches[1].value));
    }

    if (host.starts_with("[") && host.ends_with("]"))
    {
        host = host.substr(1, host.length() - 2);
    }

    return std::make_tuple(host, port);
}

string URL::location() const
{
    static const RegularExpression matchLocation(R"(^(.+)\/[^/]+$)");

    const auto matches = matchLocation.m(m_path);
    if (!matches)
    {
        return "";
    }

    return matches[0].value;
}
