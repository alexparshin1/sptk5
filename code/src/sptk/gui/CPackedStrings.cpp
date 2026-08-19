/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <cstdlib>
#include <sptk5/PackedStrings.h>

using namespace sptk;

#ifndef _WIN32

#ifdef __APPLE__
#include <stdlib.h>
#else

#endif

#else
#include <malloc.h>
#endif

CPackedStrings::CPackedStrings(int cnt, const char* strings[])
{
    for (int i = 0; i < cnt; i++)
    {
        push_back(strings[i]);
    }

    flags = 0;
    height = 0;
}

CPackedStrings::CPackedStrings(const Strings& strings)
    : Strings(strings)
{
}

CPackedStrings::CPackedStrings(FieldList& fields, int keyField)
{
    const auto cnt = static_cast<int>(fields.size());

    flags = 0;
    height = 0;

    int64_t keyValue = 0;

    for (auto i = 0; i < cnt; i++)
    {
        Field& field = fields[i];
        if (i == keyField)
        {
            keyValue = field.asInteger();
            continue;
        }
        push_back(field.asString());
    }

    argument(keyValue);
}

CPackedStrings& CPackedStrings::operator=(const CPackedStrings& other)
{
    if (&other != this)
    {
        Strings::operator=(other);
        flags = other.flags;
        height = other.height;
    }
    return *this;
}

CPackedStrings& CPackedStrings::operator=(const Strings& other)
{
    if (&other != this)
    {
        Strings::operator=(other);
        flags = 0;
        height = 0;
    }
    return *this;
}
