/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;

int hexCharToInt(unsigned char ch)
{
    if (ch > '@')
    {
        return ch - 'A' + 10;
    }
    return ch - '0';
}

String Url::encode(const String& str)
{
    auto cnt = (uint32_t) str.length();
    const char* src = str.c_str();
    array<char, 5> hexBuffer {};
    Buffer buffer(cnt * 3 + 1);
    buffer.data();
    int len {0};
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
                dest = char(hexCharToInt((unsigned char) *src) * 16 + hexCharToInt((unsigned char) src[1]));
                buffer.append(dest);
                src += 2;
                break;

            default:
                buffer.append(*src);
                ++src;
                break;
        }
    }
    return {buffer.c_str(), buffer.length()};
}

HttpParams::HttpParams(std::initializer_list<std::pair<String, String>> lst)
{
    for (const auto& [name, value]: lst)
    {
        operator[](name) = value;
    }
}

void HttpParams::decode(const Buffer& cb, bool /*lowerCaseNames*/)
{
    clear();

    Strings sl(cb.c_str(), "&");
    for (const auto& s: sl)
    {
        size_t pos = s.find('=');
        if (pos != string::npos)
        {
            string key = s.substr(0, pos);
            string value = s.substr(pos + 1);
            (*this)[key] = Url::decode(value);
        }
        else
        {
            (*this)[s] = "";
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

#ifdef USE_GTEST

static const String gtestURLencoded("id=1234&items=%5B%22book%22%2C%22pen%22%5D&name=John+Doe");

TEST(SPTK_HttpParams, encode)
{
    HttpParams httpParams;
    httpParams["id"] = "1234";
    httpParams["name"] = "John Doe";
    httpParams["items"] = R"(["book","pen"])";

    Buffer encoded;
    httpParams.encode(encoded);
    EXPECT_STREQ(gtestURLencoded.c_str(), encoded.c_str());
}

TEST(SPTK_HttpParams, decode)
{
    HttpParams httpParams;
    httpParams["noise"] = "noise";

    Buffer encoded(gtestURLencoded);
    httpParams.decode(encoded);
    EXPECT_STREQ("1234", httpParams["id"].c_str());
    EXPECT_STREQ("John Doe", httpParams["name"].c_str());
    EXPECT_STREQ(R"(["book","pen"])", httpParams["items"].c_str());
    EXPECT_EQ(size_t(3), httpParams.size());
}

#endif
