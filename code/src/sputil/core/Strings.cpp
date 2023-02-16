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

#include <sptk5/Buffer.h>
#include <sptk5/RegularExpression.h>
#include <sstream>

using namespace std;
using namespace sptk;

static void splitByDelimiter(Strings& dest, const String& src, const char* delimiter)
{
    dest.clear();
    const auto* pos = src.c_str();
    size_t delimiterLength = strlen(delimiter);
    while (true)
    {
        const auto* end = strstr(pos, delimiter);
        if (end != nullptr)
        {
            dest.emplace_back(pos, size_t(end - pos));
            pos = end + delimiterLength;
        }
        else
        {
            if (*pos != 0)
            {
                dest.emplace_back(pos);
            }
            break;
        }
    }
}

static void splitByAnyChar(Strings& dest, const String& src, const char* delimiter)
{
    dest.clear();
    size_t pos = 0;
    while (pos != string::npos)
    {
        size_t end = src.find_first_of(delimiter, pos);
        if (end != string::npos)
        {
            dest.emplace_back(src.substr(pos, end - pos));
            pos = src.find_first_not_of(delimiter, end + 1);
        }
        else
        {
            if (pos + 1 < src.length())
            {
                dest.emplace_back(src.substr(pos));
            }
            break;
        }
    }
}

static void splitByRegExp(Strings& dest, const String& src, const char* pattern)
{
    RegularExpression regularExpression(pattern);

    dest.clear();
    dest = regularExpression.split(src);
}

Strings::Strings(const String& src, const char* delimiter, SplitMode mode) noexcept
{
    try
    {
        fromString(src.c_str(), delimiter, mode);
    }
    catch (const Exception& e)
    {
        push_back("# ERROR: " + String(e.what()));
    }
}

void Strings::fromString(const String& src, const char* delimiter, SplitMode mode)
{
    clear();
    switch (mode)
    {
        case SplitMode::ANYCHAR:
            splitByAnyChar(*this, src, delimiter);
            break;
        case SplitMode::REGEXP:
            splitByRegExp(*this, src, delimiter);
            break;
        default:
            splitByDelimiter(*this, src, delimiter);
            break;
    }
}

int Strings::indexOf(const String& needle) const
{
    int result = -1;
    const_iterator itor;
    const_reverse_iterator xtor;

    switch (m_sorted)
    {
        case SortOrder::DESCENDING:
            xtor = lower_bound(rbegin(), rend(), needle);
            if (xtor != rend() && *xtor == needle)
            {
                result = (int) distance(rbegin(), xtor);
            }
            break;
        case SortOrder::ASCENDING:
            itor = lower_bound(begin(), end(), needle);
            if (itor != end() && *itor == needle)
            {
                result = (int) distance(begin(), itor);
            }
            break;
        default:
            itor = find(begin(), end(), needle);
            if (itor != end() && *itor == needle)
            {
                result = (int) distance(begin(), itor);
            }
            break;
    }
    return result;
}

void Strings::saveToFile(const fs::path& fileName) const
{
    Buffer buffer;
    for (const auto& str: *this)
    {
        buffer.append(str);
        buffer.append("\n");
    }
    buffer.saveToFile(fileName);
}

void Strings::loadFromFile(const fs::path& fileName)
{
    Buffer buffer;
    buffer.loadFromFile(fileName);

    clear();
    // Load text
    String text(buffer.c_str(), buffer.bytes());

    // Determine delimiter
    String delimiter = "\n";
    if (size_t pos1 = text.find_first_of("\n\r");
        pos1 != string::npos)
    {
        size_t pos2 = text.find_first_of("\n\r", pos1 + 1);
        delimiter = text.substr(pos1, 1);
        if (pos1 + 1 == pos2 && text[pos1] != text[pos2])
        { // Two chars delimiter
            delimiter = text.substr(pos1, 2);
        }
    }

    splitByDelimiter(*this, text, delimiter.c_str());
}

String Strings::join(const String& delimiter) const
{
    stringstream result;
    bool first = true;
    for (const auto& str: *this)
    {
        if (first)
        {
            result << str;
            first = false;
        }
        else
        {
            result << delimiter << str;
        }
    }
    return result.str();
}

Strings Strings::grep(const String& pattern) const
{
    RegularExpression regularExpression(pattern);
    Strings output;
    for (const String& str: *(this))
    {
        if (regularExpression.matches(str))
        {
            output.push_back(str);
        }
    }
    return output;
}

static bool sortAscending(const String& first, const String& second)
{
    return first < second;
}

static bool sortDescending(const String& first, const String& second)
{
    return first > second;
}

void Strings::sort(bool ascending)
{
    if (ascending)
    {
        ::sort(begin(), end(), sortAscending);
        m_sorted = SortOrder::ASCENDING;
    }
    else
    {
        ::sort(begin(), end(), sortDescending);
        m_sorted = SortOrder::DESCENDING;
    }
}
