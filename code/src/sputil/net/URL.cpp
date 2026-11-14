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
#include <charconv>
#include <algorithm>

using namespace std;
using namespace sptk;

URL::URL(const string& url)
{
    if (url.empty())
    {
        return;
    }

    if (any_of(url.begin(), url.end(), [](unsigned char c) { return c <= 0x20 || c == 0x7f; }))
        throw Exception("Invalid URL: whitespace or control character");
    string source = url;
    // SPTK also parses database connection strings, whose legacy passwords may contain '#'.
    // Delimit query/fragment after userinfo, not inside those credentials.
    const auto schemeEnd = source.find("://");
    const auto authorityStart = schemeEnd == string::npos ? 0 : schemeEnd + 3;
    const auto pathStart = source.find('/', authorityStart);
    const auto at = source.find('@', authorityStart);
    const auto suffixStart = at != string::npos && (pathStart == string::npos || at < pathStart)
                                 ? at + 1 : authorityStart;
    if (const auto fragment = source.find('#', suffixStart); fragment != string::npos)
    {
        m_fragment = source.substr(fragment);
        source.erase(fragment);
    }
    if (const auto query = source.find('?', suffixStart); query != string::npos)
    {
        m_hasQuery = true;
        m_rawQuery = source.substr(query + 1);
        source.erase(query);
        if (!m_rawQuery.empty())
            m_params.decode(Buffer(m_rawQuery));
        Buffer encoded;
        m_params.encode(encoded);
        m_encodedParams.assign(encoded.c_str(), encoded.bytes());
    }
    if (const auto scheme = source.find("://"); scheme != string::npos)
    {
        m_protocol = source.substr(0, scheme);
        if (m_protocol.empty() || !isalpha(static_cast<unsigned char>(m_protocol.front())) ||
            any_of(m_protocol.begin(), m_protocol.end(), [](unsigned char c)
                   { return !isalnum(c) && c != '+' && c != '-' && c != '.'; }))
            throw Exception("Invalid URL scheme");
        source.erase(0, scheme + 3);
    }
    const auto slash = source.find('/');
    auto authority = source.substr(0, slash);
    m_path = slash == string::npos ? "" : source.substr(slash);
    if (const auto at = authority.find('@'); at != string::npos)
    {
        const auto credentials = authority.substr(0, at);
        if (authority.find('@', at + 1) != string::npos)
            throw Exception("Invalid URL credentials");
        const auto colon = credentials.find(':');
        m_username = credentials.substr(0, colon);
        if (colon != string::npos)
            m_password = credentials.substr(colon + 1);
        authority.erase(0, at + 1);
    }
    m_hostAndPort = authority;
    if (!m_protocol.empty() && authority.empty())
        throw Exception("Invalid URL: missing host");
    (void) hostAndPort(); // Validate port before any narrowing conversion.
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

    auto target = requestTarget();
    // requestTarget supplies '/' for HTTP, while serialization preserves an absent input path.
    if (m_path.empty() && target.starts_with("/"))
        target.erase(0, 1);
    str << target << m_fragment;

    return str.str();
}

std::tuple<string, uint16_t> URL::hostAndPort() const
{
    string   host = m_hostAndPort;
    string portText;
    bool explicitPort = false;
    if (host.starts_with("["))
    {
        const auto close = host.find(']');
        if (close == string::npos || close == 1)
            throw Exception("Invalid bracketed URL host");
        const auto suffix = host.substr(close + 1);
        host = host.substr(1, close - 1);
        if (host.find(':') == string::npos)
            throw Exception("Bracketed URL host must be IPv6");
        if (!suffix.empty())
        {
            if (suffix.front() != ':')
                throw Exception("Invalid URL port separator");
            explicitPort = true;
            portText = suffix.substr(1);
        }
    }
    else if (ranges::count(host, ':') == 1)
    {
        const auto colon = host.find(':');
        portText = host.substr(colon + 1);
        host.erase(colon);
        explicitPort = true;
    }
    if (host.find_first_of("[]@/?#\\") != string::npos ||
        any_of(host.begin(), host.end(), [](unsigned char c) { return c <= 0x20 || c == 0x7f; }))
        throw Exception("Invalid URL host");
    unsigned int port = 0;
    if (explicitPort)
    {
        const auto parsed = from_chars(portText.data(), portText.data() + portText.size(), port);
        if (host.empty() || portText.empty() || parsed.ec != errc{} ||
            parsed.ptr != portText.data() + portText.size() || port == 0 || port > 65535)
            throw Exception("Invalid URL port");
    }
    return {host, static_cast<uint16_t>(port)};
}

string URL::requestTarget() const
{
    string target = m_path.empty() ? "/" : m_path;
    Buffer encoded;
    m_params.encode(encoded);
    const string current(encoded.c_str(), encoded.bytes());
    if (m_hasQuery && current == m_encodedParams)
        target += "?" + m_rawQuery;
    else if (!m_params.empty())
        target += "?" + current;
    return target;
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
