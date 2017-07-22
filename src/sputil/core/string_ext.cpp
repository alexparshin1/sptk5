/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       string_ext.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include <iostream>
#include <sptk5/sptk.h>
#include <sptk5/string_ext.h>

using namespace std;

namespace sptk
{

string upperCase (const string& str)
{
    auto len = (uint32_t) str.length();
    string result;
    result.resize (len);

    for (uint32_t i = 0; i < len; i++)
        result[i] = (char) toupper (str[i]);

    return result;
}

string lowerCase (const string& str)
{
    auto len = (uint32_t) str.length();
    string result;
    result.resize (len);

    for (uint32_t i = 0; i < len; i++)
        result[i] = (char) tolower (str[i]);

    return result;
}

string trim (const string& str)
{
    auto len = (uint32_t) str.length();

    if (len == 0)
        return "";

    auto *s = (const unsigned char *) str.c_str();
    int i, startpos = 0, endpos = int (len - 1);
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

    return str.substr(size_t(startpos), size_t(endpos-startpos+1));
}

void join(string& dest, const vector<string>& src, const string& separator)
{
    dest = "";
    auto itor = src.begin();

    for (; itor != src.end(); ++itor) {
        dest += *itor + separator;
    }
}

void split(vector<string>& dest, const string& src, const string& delimitter)
{
    dest.clear();
    auto buffer = (char *) src.c_str();

    if (strlen(buffer) == 0)
        return;

    auto dlen = (uint32_t) delimitter.length();

    if (dlen == 0)
        return;

    char *p = buffer;

    for (;;) {
        char *end = strstr (p, delimitter.c_str());

        if (end != nullptr) {
            //int len = end - p;
            char sc = *end;
            *end = 0;
            dest.push_back (p);
            *end = sc;
            p = end + dlen;
        } else {
            dest.push_back (p);
            break;
        }
    }
}

string int2string (int32_t value)
{
    char buff[32];
    snprintf(buff, sizeof(buff), "%i", value);
    return string(buff);
}

string int2string (uint32_t value)
{
    char buff[64];
    snprintf(buff, sizeof(buff), "%u", value);
    return string(buff);
}

string int2string (int64_t value)
{
    char buff[128];
#ifdef _WIN32
    snprintf(buff, sizeof(buff), "%lli", value);
#else
    snprintf(buff, sizeof(buff), "%lli", (long long int) value);
#endif
    return string(buff);
}

string int2string (uint64_t value)
{
    char buff[128];
#ifdef _WIN32
    snprintf(buff, sizeof(buff), "%llu", value);
#else
    snprintf(buff, sizeof(buff), "%lu", value);
#endif
    return string(buff);
}

int string2int (const string& str, int defaultValue)
{
    char *endptr;
    errno = 0;
    auto result = (int) strtol(str.c_str(), &endptr, 10);

    if (errno)
        return defaultValue;

    return result;
}

int64_t string2int64 (const string& str, int64_t defaultValue)
{
    char *endptr;
    errno = 0;
    auto result = (int64_t) strtoll(str.c_str(), &endptr, 10);

    if (errno)
        return defaultValue;

    return result;
}

string capitalizeWords (const std::string& str)
{
    string s (str);
    auto current = (char *) s.c_str();
    char *wordStart = nullptr;

    if (*current != char(0)) {
        for (;;) {
            if (isalnum (*current) != 0) {
                if (wordStart == nullptr)
                    wordStart = current;
            } else {
                if (current - wordStart > 3) {
                    if (wordStart != nullptr)
                        *wordStart = (char) toupper (*wordStart);
                    else
                        wordStart = current;

                    for (char *ptr = wordStart + 1; ptr < current; ptr++)
                        *ptr = (char) tolower (*ptr);
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

string replaceAll (const string& src,const string& pattern,const string& replacement)
{
    string str (src);

    if (pattern.empty())
        return src;

    size_t patternLength = pattern.length();
    size_t replacementLength = replacement.length();

    size_t i = str.find (pattern);

    while (i != STRING_NPOS) { // While not at the end of the string
        str.replace (i,patternLength,replacement);
        i = str.find (pattern, i + replacementLength);
    }

    return str;
}

void stringToStringVector (const string& src,vector<string> dest,string delimitter)
{
    string buffer (src);
    dest.clear();

    if (buffer[0] == char(0))
        return;

    auto dlen = (uint32_t) delimitter.length();
    if (dlen == 0)
        return;

    auto p = (char *) buffer.c_str();

    for (;;) {
        char *end = strstr (p,delimitter.c_str());

        if (end != nullptr) {
            char sc = *end;
            *end = 0;
            dest.push_back (p);
            *end = sc;
            p = end + dlen;
        } else {
            dest.push_back (p);
            break;
        }
    }
}

}
