/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       String.cpp - description                               ║
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

#include <fstream>
#include <sptk5/Strings.h>
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
    return Strings(*this, pattern.c_str(), Strings::SM_REGEXP);
}

bool String::startsWith(const String& subject) const
{
    return find(subject) == 0;
}

String String::replace(const String& pattern, const String& replacement) const
{
    RegularExpression regexp(pattern);
    bool replaced = false;
    return regexp.replaceAll(*this, replacement, replaced);
}

bool String::endsWith(const String& subject) const
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
