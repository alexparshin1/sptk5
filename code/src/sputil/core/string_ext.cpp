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

#include <cstdlib>
#include <fstream>
#include <sptk5/Buffer.h>

using namespace std;
using namespace sptk;

String sptk::upperCase(const String& str)
{
    auto len = (uint32_t) str.length();
    string result;
    result.resize(len);

    for (uint32_t i = 0; i < len; ++i)
    {
        result[i] = (char) toupper(str[i]);
    }

    return result;
}

String sptk::lowerCase(const String& str)
{
    auto len = (uint32_t) str.length();
    string result;
    result.resize(len);

    for (uint32_t i = 0; i < len; ++i)
    {
        result[i] = (char) tolower(str[i]);
    }

    return result;
}

String sptk::trim(const String& str)
{
    auto len = (uint32_t) str.length();

    if (len == 0)
    {
        return "";
    }

    const auto* s = (const unsigned char*) str.c_str();
    auto endPosition = int(len - 1);
    bool found = false;

    const unsigned char space = ' ';
    for (int i = endPosition; i >= 0; --i)
    {
        if (s[i] > space)
        {
            endPosition = i;
            found = true;
            break;
        }
    }

    if (!found)
    {
        return "";
    }

    int startPosition = 0;
    for (int i = 0; i <= endPosition; ++i)
    {
        if (s[i] > space)
        {
            startPosition = i;
            break;
        }
    }

    return str.substr(size_t(startPosition), size_t(long(endPosition - startPosition + 1)));
}

String sptk::int2string(int32_t value)
{
    constexpr int maxLength = 32;
    array<char, maxLength + 1> buff {};
    const int len = snprintf(buff.data(), maxLength, "%i", value);
    return string(buff.data(), (unsigned) len);
}

String sptk::int2string(uint32_t value)
{
    constexpr int maxLength = 64;
    array<char, maxLength + 1> buff {};
    const int len = snprintf(buff.data(), maxLength, "%u", value);
    return string(buff.data(), (unsigned) len);
}

String sptk::int2string(int64_t value)
{
    constexpr int maxLength = 128;
    array<char, maxLength + 1> buff {};
#ifdef _WIN32
    const int len = snprintf(buff.data(), maxLength, "%lli", value);
#else
    const int len = snprintf(buff.data(), maxLength, "%lli", (long long int) value);
#endif
    return string(buff.data(), (unsigned) len);
}

String sptk::int2string(uint64_t value)
{
    constexpr int maxLength = 64;
    array<char, maxLength + 1> buff {};
#ifdef _WIN32
    const int len = snprintf(buff.data(), sizeof(buff), "%llu", value);
#else
    const int len = snprintf(buff.data(), maxLength, "%lu", value);
#endif
    return string(buff.data(), (unsigned) len);
}

int sptk::string2int(const String& str, int defaultValue)
{
    char* endPointer = nullptr;
    errno = 0;
    auto result = (int) strtol(str.c_str(), &endPointer, 10);

    if (errno)
    {
        return defaultValue;
    }

    return result;
}

int64_t sptk::string2int64(const String& str, int64_t defaultValue)
{
    char* endPointer = nullptr;
    errno = 0;
    auto result = (int64_t) strtoll(str.c_str(), &endPointer, 10);

    if (errno)
    {
        return defaultValue;
    }

    return result;
}

String sptk::double2string(double value)
{
    constexpr int maxLength = 64;
    array<char, maxLength + 1> buffer {};
    int len = snprintf(buffer.data(), maxLength, "%f", value);
    for (int i = len - 1; i > 0; --i)
    {
        if (buffer[i] != '0')
        {
            if (buffer[i] == '.')
            {
                len = i + 2;
            }
            else
            {
                len = i + 1;
            }
            break;
        }
    }
    return {buffer.data(), size_t(len), 0};
}

double sptk::string2double(const String& str)
{
    char* endPointer = nullptr;
    errno = 0;
    auto result = strtod(str.c_str(), &endPointer);

    if (errno)
    {
        throw Exception("Invalid number");
    }

    return result;
}

double sptk::string2double(const String& str, double defaultValue)
{
    char* endPointer = nullptr;
    errno = 0;
    auto result = strtod(str.c_str(), &endPointer);

    if (errno)
    {
        return defaultValue;
    }

    return result;
}

static void capitalizeWord(char* current, char* wordStart)
{
    if (wordStart != nullptr)
    {
        *wordStart = (char) toupper(*wordStart);
    }
    else
    {
        wordStart = current;
    }

    for (char* ptr = wordStart + 1; ptr < current; ++ptr)
    {
        *ptr = (char) tolower(*ptr);
    }
}

static void lowerCaseWord(const char* current, char* wordStart)
{
    if (wordStart != nullptr)
    {
        for (char* ptr = wordStart; ptr < current; ++ptr)
        {
            *ptr = (char) tolower(*ptr);
        }
    }
}

String sptk::capitalizeWords(const String& s)
{
    if (s.empty())
    {
        return s;
    }

    Buffer buffer(s);
    char* wordStart = nullptr;

    for (auto* current = (char*) buffer.data();; ++current)
    {
        if (isalnum(*current) != 0)
        {
            if (wordStart == nullptr)
            {
                wordStart = current;
            }
        }
        else
        {
            if (current - wordStart > 3)
            {
                capitalizeWord(current, wordStart);
            }
            else
            {
                lowerCaseWord(current, wordStart);
            }
            wordStart = nullptr;
            if (*current == char(0))
            {
                break;
            }
        }
    }

    return {buffer.c_str(), buffer.size()};
}
