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

#include <format>
#include <sptk5/Buffer.h>

using namespace std;
using namespace sptk;

string sptk::upperCase(const string_view str)
{
    const auto len = static_cast<uint32_t>(str.length());
    string     result;
    result.resize(len);

    for (uint32_t i = 0; i < len; ++i)
    {
        result[i] = static_cast<char>(toupper(str[i]));
    }

    return result;
}

string sptk::lowerCase(const string_view str)
{
    const auto len = static_cast<uint32_t>(str.length());
    string     result;
    result.resize(len);

    auto* resultPtr = result.data();
    for (auto& ch: str)
    {
        *resultPtr = static_cast<char>(tolower(ch));
        ++resultPtr;
    }

    return result;
}

string sptk::trim(const string_view str)
{
    const auto len = static_cast<uint32_t>(str.length());

    if (len == 0)
    {
        return "";
    }

    const auto* s = reinterpret_cast<const unsigned char*>(str.data());
    auto        endPosition = static_cast<int>(len - 1);
    bool        found = false;

    constexpr unsigned char space = ' ';
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

    auto startPosition = 0;
    for (auto i = 0; i <= endPosition; ++i)
    {
        if (s[i] > space)
        {
            startPosition = i;
            break;
        }
    }

    return {str.data() + startPosition, static_cast<size_t>(endPosition - startPosition + 1)};
}

int sptk::string2int(const string_view str, int defaultValue)
{
    char* endPointer = nullptr;
    errno = 0;
    const auto result = static_cast<int>(strtol(str.data(), &endPointer, 10));

    if (errno)
    {
        return defaultValue;
    }

    return result;
}

int64_t sptk::string2int64(const string_view str, int64_t defaultValue)
{
    char* endPointer = nullptr;
    errno = 0;
    const auto result = static_cast<int64_t>(strtoll(str.data(), &endPointer, 10));

    if (errno)
    {
        return defaultValue;
    }

    return result;
}

string sptk::double2string(double value)
{
    auto buffer = format("{}", value);
    auto len = static_cast<int>(buffer.length());
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

    if (len == static_cast<int>(buffer.length()))
    {
        return buffer;
    }

    return buffer.substr(0, len);
}

double sptk::string2double(const string_view str)
{
    char* endPointer = nullptr;
    errno = 0;
    const auto result = strtod(str.data(), &endPointer);

    if (errno)
    {
        throw Exception("Invalid number");
    }

    return result;
}

double sptk::string2double(const string_view str, double defaultValue)
{
    char* endPointer = nullptr;
    errno = 0;
    const auto result = strtod(str.data(), &endPointer);

    if (errno)
    {
        return defaultValue;
    }

    return result;
}

namespace {
void capitalizeWord(char* current, char* wordStart)
{
    if (wordStart != nullptr)
    {
        *wordStart = static_cast<char>(toupper(*wordStart));
    }
    else
    {
        wordStart = current;
    }

    for (char* ptr = wordStart + 1; ptr < current; ++ptr)
    {
        *ptr = static_cast<char>(tolower(*ptr));
    }
}

void lowerCaseWord(const char* current, char* wordStart)
{
    if (wordStart != nullptr)
    {
        for (char* ptr = wordStart; ptr < current; ++ptr)
        {
            *ptr = static_cast<char>(tolower(*ptr));
        }
    }
}
} // namespace

String sptk::capitalizeWords(const String& s)
{
    if (s.empty())
    {
        return s;
    }

    Buffer buffer(s);
    char*  wordStart = nullptr;

    for (auto* current = reinterpret_cast<char*>(buffer.data());; ++current)
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
            if (*current == static_cast<char>(0))
            {
                break;
            }
        }
    }

    return {buffer.c_str(), buffer.size()};
}
