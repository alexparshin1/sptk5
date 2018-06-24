/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPackedStrings.cpp - description                       ║
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
#include <sptk5/PackedStrings.h>

using namespace sptk;

#ifndef _WIN32

#  ifdef __APPLE__
#    include <stdlib.h>
#  else

#  endif

#else
#  include <malloc.h>
#endif

CPackedStrings::CPackedStrings(int cnt, const char* strings[])
{
    // compute buffer size and offsets
    int offsetsSpace = cnt * sizeof(uint16_t);
    int sz = offsetsSpace + sizeof(uint16_t);

    auto offset = (uint16_t*) alloca(offsetsSpace * 2);
    uint16_t* len = offset + cnt;

    flags = 0;
    height = 0;
    m_data = nullptr;

    for (int i = 0; i < cnt; i++) {
        const char* s = strings[i];
        auto l = uint16_t(strlen(s) + 1);
        offset[i] = uint16_t(sz);
        len[i] = l;
        sz += l;
    }

    m_size = uint16_t(sz);
    m_buffer = malloc(m_size);

    *(uint16_t*) m_buffer = uint16_t(cnt);
    memcpy((uint16_t*) m_buffer + 1, offset, (size_t) offsetsSpace);

    for (int j = 0; j < cnt; j++)
        memcpy(pchar(m_buffer) + offset[j], strings[j], len[j]);
}

CPackedStrings::CPackedStrings(const Strings& strings)
{
    m_buffer = nullptr;
    operator=(strings);
}

CPackedStrings::CPackedStrings(FieldList& fields, int keyField)
{
    int cnt = fields.size();
    int rcnt = cnt;
    if (keyField >= 0 && keyField < cnt) // if keyField is used - do not store it as string
        rcnt -= 1;

    // compute buffer size and offsets
    int offsetsSpace = rcnt * sizeof(uint16_t);
    int sz = offsetsSpace + sizeof(uint16_t);

    auto offset = (uint16_t*) alloca(offsetsSpace * 2);
    uint16_t* len = offset + rcnt;

    flags = 0;
    height = 0;
    m_data = nullptr;

    int j = 0;
    long keyValue = 0;
    Strings strings;
    {
        for (int i = 0; i < cnt; i++) {
            Field& field = fields[i];
            if (i == keyField) {
                keyValue = field.asInteger();
                continue;
            }
            uint16_t l;
            if (field.dataType() == VAR_STRING) { // conversion isn't required
                l = uint16_t(field.dataSize() + 1);
                strings.push_back(field.getString());
            } else {
                String str = field.asString();
                strings.push_back(str);
                l = uint16_t(str.length() + 1);
            }
            offset[j] = uint16_t(sz);
            len[j] = l;
            sz += l;
            j++;
        }
    }
    m_size = uint16_t(sz);
    m_buffer = malloc(m_size);

    *(uint16_t*) m_buffer = uint16_t(cnt);
    memcpy((uint16_t*) m_buffer + 1, offset, (size_t) offsetsSpace);
    j = 0;
    {/*alex*/
        for (int i = 0; i < cnt; i++) {
            if (i == keyField) continue;
            memcpy(pchar(m_buffer) + offset[j], strings[j].c_str(), len[j]);
            j++;
        }
    }
    m_data = (void*) keyValue;
}

CPackedStrings::~CPackedStrings()
{
    free(m_buffer);
}

const char* CPackedStrings::operator[](uint16_t index) const
{
    uint16_t* offsets = (uint16_t*) m_buffer + 1;
    return pchar(m_buffer) + offsets[index];
}

CPackedStrings& CPackedStrings::operator=(const CPackedStrings& newData)
{
    m_data = newData.m_data;
    if (m_size != newData.m_size) {
        m_size = newData.m_size;
        m_buffer = realloc(m_buffer, m_size);
    }
    memcpy(m_buffer, newData.m_buffer, m_size);
    return *this;
}

CPackedStrings& CPackedStrings::operator=(const Strings& strings)
{
    size_t cnt = strings.size();
    auto offsetsSpace = int(cnt * sizeof(uint16_t));
    auto offset = (uint16_t*) alloca(offsetsSpace * 2);
    uint16_t* len = offset + cnt;

    flags = 0;
    height = 0;
    m_data = (void*) (long) strings.argument();

    int sz = offsetsSpace + sizeof(uint16_t);

    // compute buffer size and offsets
    for (size_t i = 0; i < cnt; i++) {
        auto l = uint16_t(strings[i].length() + 1);
        offset[i] = uint16_t(sz);
        len[i] = l;
        sz += l;
    }

    // create buffer
    m_size = uint16_t(sz);
    m_buffer = realloc(m_buffer, m_size);

    *(uint16_t*) m_buffer = uint16_t(cnt);
    memcpy((uint16_t*) m_buffer + 1, offset, (size_t) offsetsSpace);
    for (size_t j = 0; j < cnt; j++)
        memcpy(pchar(m_buffer) + offset[j], strings[j].c_str(), len[j]);

    return *this;
}
