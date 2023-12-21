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

#include <sptk5/Strings.h>
#include <sptk5/net/HttpParams.h>

using namespace std;
using namespace sptk;

int hexCharToInt(unsigned char character)
{
    if (character > '@')
    {
        constexpr int digitsOffset = 10;
        return character - 'A' + digitsOffset;
    }
    return character - '0';
}

String Url::encode(const String& str)
{
    auto cnt = (uint32_t) str.length();
    const char* src = str.c_str();

    constexpr int bufferSize = 5;
    array<char, bufferSize> hexBuffer {};
    Buffer buffer(cnt * 3 + 1);
    buffer.data();
    int len;
    while (*src != 0)
    {
        if (isalnum(*src) != 0)
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
                    buffer.append(*src);
                    break;
                default:
                    len = snprintf(hexBuffer.data(), sizeof(hexBuffer), "%%%02X", (unsigned char) *src);
                    buffer.append(hexBuffer.data(), (size_t) len);
                    break;
            }
        }
        ++src;
    }
    return {buffer.c_str(), buffer.bytes()};
}

String Url::decode(const String& str)
{
    constexpr int base16 = 16;
    const char* src = str.c_str();
    char dest {0};
    Buffer buffer;
    while (*src != 0)
    {
        switch (*src)
        {
            case '+':
                buffer.append(' ');
                ++src;
                break;

            case '%':
                ++src;
                dest = char(hexCharToInt((unsigned char) *src) * base16 + hexCharToInt((unsigned char) src[1]));
                buffer.append(dest);
                src += 2;
                break;

            default:
                buffer.append(*src);
                ++src;
                break;
        }
    }
    return {buffer.c_str(), buffer.size()};
}

HttpParams::HttpParams(std::initializer_list<std::pair<String, String>> lst)
{
    for (const auto& [name, value]: lst)
    {
        operator[](name) = value;
    }
}

void HttpParams::decode(const Buffer& buffer, bool /*lowerCaseNames*/)
{
    clear();

    const Strings params(buffer.c_str(), "&");
    for (const auto& param: params)
    {
        const size_t pos = param.find('=');
        if (pos != string::npos)
        {
            const String key = param.substr(0, pos);
            const String value = param.substr(pos + 1);
            (*this)[key] = Url::decode(value);
        }
        else
        {
            (*this)[param] = "";
        }
    }
}

void HttpParams::encode(Buffer& result) const
{
    unsigned cnt = 0;
    for (const auto& [name, value]: *this)
    {
        String param;
        param = name + "=" + Url::encode(value);
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
    auto itor = find(paramName);
    if (itor == end())
    {
        return "";
    }
    return itor->second;
}

bool HttpParams::has(const String& paramName) const
{
    auto itor = find(paramName);
    return itor != end();
}
