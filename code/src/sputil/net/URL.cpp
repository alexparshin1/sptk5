/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/net/URL.h>

using namespace std;
using namespace sptk;

namespace {
bool nextToken(const String& url, size_t& start, const String& delimiter, String& value)
{
    value = "";
    if (auto end = url.find(delimiter, start);
        end != string::npos)
    {
        value = url.substr(start, end - start);
        start = end + delimiter.length();
        return true;
    }
    return false;
}
} // namespace

URL::URL(const String& url)
{
    size_t start = 0;
    nextToken(url, start, "://", m_protocol);

    String credentials;
    nextToken(url, start, "@", credentials);
    if (!credentials.empty())
    {
        auto pos = credentials.find(":");
        if (pos == string::npos)
        {
            m_username = credentials;
        }
        else
        {
            m_username = credentials.substr(0, pos);
            m_password = credentials.substr(pos + 1);
        }
    }

    Buffer buffer;
    if (!nextToken(url, start, "/", m_hostAndPort))
    {
        m_hostAndPort = url.substr(start);
    }
    else
    {
        --start;
        if (nextToken(url, start, "?", m_path))
        {
            buffer.set(url.substr(start));
            m_params.decode(buffer);
        }
        else
        {
            m_path = url.substr(start);
        }
    }
}

String URL::toString() const
{
    stringstream str;

    if (!m_protocol.empty())
    {
        str << m_protocol << "://";
    }

    if (!m_username.empty())
    {
        str << m_username << ":" << m_password << "@";
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

String URL::location() const
{
    static const RegularExpression matchLocation(R"(^(.+)\/[^/]+$)");

    auto matches = matchLocation.m(m_path);
    if (!matches)
    {
        return "";
    }

    return matches[0].value;
}
