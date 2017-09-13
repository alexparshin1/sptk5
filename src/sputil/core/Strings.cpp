/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Strings.cpp - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string.h>
#include <sptk5/Strings.h>
#include <sptk5/Buffer.h>
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;

bool String::matches(string pattern, string options) const
{
    return *this == RegularExpression(pattern, options);
}

string String::toUpperCase() const
{
    return upperCase(*this);
}

string String::toLowerCase() const
{
    return lowerCase(*this);
}

Strings String::split(string pattern) const
{
    return Strings(*this, pattern.c_str(), Strings::SM_REGEXP);
}

bool String::startsWith(string subject) const
{
    return find(subject) == 0;
}

string String::replace(string pattern, string replacement) const
{
    RegularExpression regexp(pattern);
    bool replaced = false;
    return regexp.replaceAll(*this, replacement, replaced);
}

bool String::endsWith(string subject) const
{
    size_t pos = rfind(subject);
    return pos != string::npos && pos == length() - subject.length();
}

String String::trim() const
{
    auto startPos = find_first_not_of(" \n\r\t\b");
    if (startPos == string::npos)
        return String("");
    size_t endPos = find_last_not_of(" \n\r\t\b");
    return substr(startPos, endPos - startPos + 1);
}

void Strings::splitByDelimiter(const string &src, const char *delimitter)
{
    size_t pos = 0;
    size_t delimitterLength = strlen(delimitter);
    while (true) {
        size_t end = src.find(delimitter, pos);
        if (end != string::npos) {
            push_back(src.substr(pos, end - pos));
            pos = end + delimitterLength;
        }
        else {
            if (pos + 1 <= src.length())
                push_back(src.substr(pos));
            break;
        }
    }
}

void Strings::splitByAnyChar(const string &src, const char *delimitter)
{
    size_t pos = 0;
    while (pos != string::npos) {
        size_t end = src.find_first_of(delimitter, pos);
        if (end != string::npos) {
            push_back(src.substr(pos, end - pos));
            pos = src.find_first_not_of(delimitter, end + 1);
        }
        else {
            if (pos + 1 < src.length())
                push_back(src.substr(pos));
            break;
        }
    }
}

void Strings::splitByRegExp(const string &src, const char *pattern)
{
    RegularExpression regularExpression(pattern);
    regularExpression.split(src, *this);
}

void Strings::fromString(const string &src, const char *delimitter, SplitMode mode)
{
    clear();
    switch (mode) {
        case SM_DELIMITER:
            splitByDelimiter(src, delimitter);
            break;
        case SM_ANYCHAR:
            splitByAnyChar(src, delimitter);
            break;
        case SM_REGEXP:
            splitByRegExp(src, delimitter);
            break;
    }
}

string Strings::asString(const char *delimitter) const
{
    stringstream result;
    bool first = true;
    for (auto& str: *this) {
        if (first) {
            result << str;
            first = false;
        } else
            result << delimitter << str;
    }
    return result.str();
}

int Strings::indexOf(string s) const
{
    const_iterator itor = find(begin(), end(), s.c_str());
    if (itor == end())
        return -1;
    return (int) distance(begin(), itor);
}

void Strings::saveToFile(string fileName) const
{
    Buffer buffer;
    for (const_iterator str = begin(); str != end(); ++str) {
        buffer.append(*str);
        buffer.append("\n");
    }
    buffer.saveToFile(fileName);
}

void Strings::loadFromFile(string fileName)
{
    Buffer buffer;
    buffer.loadFromFile(fileName);

    clear();
    // Load text
    string text(buffer.c_str(), buffer.bytes());

    // Determine delimiter
    size_t pos1 = text.find_first_of("\n\r");
    size_t pos2 = text.find_first_of("\n\r", pos1 + 1);
    string delimiter = text.substr(pos1, 1);
    if (pos1 + 1 == pos2) {
        if (text[pos1] != text[pos2]) // Two chars delimiter
            delimiter = text.substr(pos1, 2);
    }

    splitByDelimiter(text, delimiter.c_str());
}

string Strings::join(string delimiter) const
{
    return asString(delimiter.c_str());
}

Strings Strings::grep(string pattern) const
{
    RegularExpression regularExpression(pattern);
    Strings output;
    for (const String& str : *(this)) {
        if (str == regularExpression)
            output.push_back(str);
    }
    return output;
}

bool Strings::sortAscending(const String& first, const String& second)
{
    return first < second;
}

bool Strings::sortDescending(const String& first, const String& second)
{
    return first > second;
}

void Strings::sort(bool ascending)
{
    if (ascending)
        ::sort(begin(),end(), sortAscending);
    else
        ::sort(begin(),end(), sortDescending);
}
