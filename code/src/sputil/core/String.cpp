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

#include <fstream>
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;

bool String::matches(const String& pattern, const String& options) const
{
    RegularExpression regexp(pattern, options);
    return regexp.matches(*this);
}

String String::toUpperCase() const
{
    return upperCase(*this);
}

String String::toLowerCase() const
{
    return lowerCase(*this);
}

Strings String::split(const String& pattern) const
{
    return {*this, pattern.c_str(), Strings::SplitMode::REGEXP};
}

bool String::startsWith(const String& subject) const
{
    return find(subject) == 0;
}

bool String::endsWith(const String& subject) const
{
    size_t pos = rfind(subject);
    return pos != string::npos && pos == length() - subject.length();
}

bool String::contains(const String& subject) const
{
    size_t pos = find(subject);
    return pos != string::npos;
}

String String::replace(const String& pattern, const String& replacement) const
{
    RegularExpression regexp(pattern);
    bool replaced = false;
    return regexp.replaceAll(*this, replacement, replaced);
}

String String::trim() const
{
    auto startPos = find_first_not_of(" \n\r\t\b");
    if (startPos == string::npos)
    {
        return {};
    }
    size_t endPos = find_last_not_of(" \n\r\t\b");
    return substr(startPos, endPos - startPos + 1);
}

int String::toInt() const
{
    return string2int(*this, 0);
}

bool String::in(std::initializer_list<String> list) const
{
    return any_of(list.begin(), list.end(), [this](const String& value) {
        return value == *this;
    });
}
