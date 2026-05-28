/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-06                                             ║
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

#include <sptk5/Strings.h>
#include <sptk5/net/HttpParams.h>

using namespace std;
using namespace sptk;

namespace {
int hexCharToInt(const char character)
{
    constexpr auto digitsOffset = 10;
    if (character >= 'A' && character <= 'F')
    {
        return character - 'A' + digitsOffset;
    }
    if (character >= 'a' && character <= 'f')
    {
        return character - 'a' + digitsOffset;
    }
    if (character >= '0' && character <= '9')
    {
        return character - '0';
    }
    throw Exception("Invalid hex character");
}
} // namespace

String Url::encode(const String& str)
{
    const auto  cnt = static_cast<uint32_t>(str.length());
    const char* src = str.c_str();

    constexpr auto          bufferSize = 5;
    array<char, bufferSize> hexBuffer {};
    Buffer                  buffer(cnt * 3 + 1);
    while (*src != 0)
    {
        if (isalnum(static_cast<unsigned char>(*src)) != 0)
        {
            buffer.append(*src);
        }
        else
        {
            switch (*src)
            {
                case ' ':
                    buffer.append('+');
                    break;
                case '.':
                case '-':
                case '_':
                case '~':
                    buffer.append(*src);
                    break;
                default:
                    const auto len = snprintf(hexBuffer.data(), sizeof(hexBuffer), "%%%02X", static_cast<unsigned char>(*src));
                    buffer.append(hexBuffer.data(), static_cast<size_t>(len));
                    break;
            }
        }
        ++src;
    }
    return {buffer.c_str(), buffer.bytes()};
}

String Url::decode(const String& str)
{
    const char* src = str.c_str();
    char        dest {0};
    Buffer      buffer;
    size_t      pos = 0;
    while (pos < str.length())
    {
        constexpr auto base16 = 16;
        switch (*src)
        {
            case '+':
                buffer.append(' ');
                ++src;
                break;

            case '%':
                ++src;
                if (pos + 3 > str.length())
                {
                    throw Exception("Invalid URL encoding");
                }
                dest = static_cast<char>(hexCharToInt(*src) * base16 + hexCharToInt(src[1]));
                buffer.append(dest);
                src += 2;
                pos += 2;
                break;

            default:
                buffer.append(*src);
                ++src;
                break;
        }
        ++pos;
    }
    return {buffer.c_str(), buffer.size()};
}

HttpParams::HttpParams(std::initializer_list<std::pair<String, String>> lst)
{
    for (const auto& [name, value]: lst)
    {
        m_params[name] = value;
    }
}

void HttpParams::decode(const Buffer& buffer)
{
    m_params.clear();

    const Strings params(buffer.c_str(), "&");
    for (const auto& param: params)
    {
        const auto pos = param.find('=');
        if (pos != string::npos)
        {
            const String key = Url::decode(param.substr(0, pos));
            const String value = Url::decode(param.substr(pos + 1));
            m_params[key] = value;
        }
        else
        {
            const String key = Url::decode(param);
            m_params[key] = "";
        }
    }
}

void HttpParams::encode(Buffer& result) const
{
    unsigned cnt = 0;
    result.bytes(0);
    for (const auto& [name, value]: m_params)
    {
        String param;
        param = Url::encode(name) + "=" + Url::encode(value);
        if (cnt != 0)
        {
            result.append('&');
        }
        result.append(param);
        ++cnt;
    }
}

String HttpParams::get(const String& paramName) const
{
    const auto itor = m_params.find(paramName);
    if (itor == m_params.end())
    {
        return "";
    }
    return itor->second;
}

bool HttpParams::has(const String& paramName) const
{
    const auto itor = m_params.find(paramName);
    return itor != m_params.end();
}

bool HttpParams::empty() const
{
    return m_params.empty();
}
