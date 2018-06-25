/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       string_ext.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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
#include <cstring>
#include <fstream>
#include <sptk5/sptk.h>
#include <sptk5/string_ext.h>
#include <sptk5/Exception.h>

using namespace std;

using namespace sptk;

String sptk::upperCase(const String& str)
{
    auto len = (uint32_t) str.length();
    string result;
    result.resize(len);

    for (uint32_t i = 0; i < len; i++)
        result[i] = (char) toupper(str[i]);

    return result;
}

String sptk::lowerCase(const String& str)
{
    auto len = (uint32_t) str.length();
    string result;
    result.resize(len);

    for (uint32_t i = 0; i < len; i++)
        result[i] = (char) tolower(str[i]);

    return result;
}

String sptk::trim(const String& str)
{
    auto len = (uint32_t) str.length();

    if (len == 0)
        return "";

    auto* s = (const unsigned char*) str.c_str();
    int i, startpos = 0, endpos = int(len - 1);
    bool found = false;

    for (i = endpos; i >= 0; i--) {
        if (s[i] > 32) {
            endpos = i;
            found = true;
            break;
        }
    }

    if (!found)
        return "";

    for (i = 0; i <= endpos; i++) {
        if (s[i] > 32) {
            startpos = i;
            break;
        }
    }

    return str.substr(size_t(startpos), size_t(long(endpos - startpos + 1)));
}

void sptk::join(string& dest, const vector<string>& src, const string& separator)
{
    dest = "";
    auto itor = src.begin();

    for (; itor != src.end(); ++itor) {
        dest += *itor + separator;
    }
}

void sptk::split(vector<string>& dest, const string& src, const string& delimitter)
{
    dest.clear();
    auto buffer = (char*) src.c_str();

    if (strlen(buffer) == 0)
        return;

    auto dlen = (uint32_t) delimitter.length();

    if (dlen == 0)
        return;

    char* p = buffer;

    for (;;) {
        char* end = strstr(p, delimitter.c_str());

        if (end != nullptr) {
            //int len = end - p;
            char sc = *end;
            *end = 0;
            dest.emplace_back(p);
            *end = sc;
            p = end + dlen;
        } else {
            dest.emplace_back(p);
            break;
        }
    }
}

String sptk::int2string(int32_t value)
{
    char buff[32];
    int len = snprintf(buff, sizeof(buff), "%i", value);
    return string(buff, (unsigned) len);
}

String sptk::int2string(uint32_t value)
{
    char buff[64];
    int len = snprintf(buff, sizeof(buff), "%u", value);
    return string(buff, (unsigned) len);
}

String sptk::int2string(int64_t value)
{
    char buff[128];
#ifdef _WIN32
    int len = snprintf(buff, sizeof(buff), "%lli", value);
#else
    int len = snprintf(buff, sizeof(buff), "%lli", (long long int) value);
#endif
    return string(buff, (unsigned) len);
}

String sptk::int2string(uint64_t value)
{
    char buff[128];
#ifdef _WIN32
    int len = snprintf(buff, sizeof(buff), "%llu", value);
#else
    int len = snprintf(buff, sizeof(buff), "%lu", value);
#endif
    return string(buff, (unsigned) len);
}

int sptk::string2int(const String& str, int defaultValue)
{
    char* endptr;
    errno = 0;
    auto result = (int) strtol(str.c_str(), &endptr, 10);

    if (errno)
        return defaultValue;

    return result;
}

int64_t sptk::string2int64(const String& str, int64_t defaultValue)
{
    char* endptr;
    errno = 0;
    auto result = (int64_t) strtoll(str.c_str(), &endptr, 10);

    if (errno)
        return defaultValue;

    return result;
}

double sptk::string2double(const String& str)
{
    char* endptr;
    errno = 0;
    auto result = strtod(str.c_str(), &endptr);

    if (errno)
        throw Exception("Invalid number");

    return result;
}

double sptk::string2double(const String& str, double defaultValue)
{
    char* endptr;
    errno = 0;
    auto result = strtod(str.c_str(), &endptr);

    if (errno)
        return defaultValue;

    return result;
}

String sptk::capitalizeWords(const String& str)
{
    String s(str);
    auto current = (char*) s.c_str();
    char* wordStart = nullptr;

    if (*current != char(0)) {
        for (;;) {
            if (isalnum(*current) != 0) {
                if (wordStart == nullptr)
                    wordStart = current;
            } else {
                if (current - wordStart > 3) {
                    if (wordStart != nullptr)
                        *wordStart = (char) toupper(*wordStart);
                    else
                        wordStart = current;

                    for (char* ptr = wordStart + 1; ptr < current; ptr++)
                        *ptr = (char) tolower(*ptr);
                }

                wordStart = nullptr;
            }

            if (*current == char(0))
                break;

            current++;
        }
    }

    return s;
}
