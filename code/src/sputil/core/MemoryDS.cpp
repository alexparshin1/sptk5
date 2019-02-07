/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MemoryDS.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/MemoryDS.h>

using namespace std;
using namespace sptk;

#define checkDSopen(ds) if (!(ds)) throw Exception("Dataset isn't open")

// access to the field by name
const Field& MemoryDS::operator[](const String& field_name) const
{
    SharedLock(m_mutex);
    checkDSopen(m_current);
    return (*m_current)[field_name];
}

Field& MemoryDS::operator[](const String& field_name)
{
    UniqueLock(m_mutex);
    checkDSopen(m_current);
    return (*m_current)[field_name];
}

uint32_t MemoryDS::recordCount() const
{
    SharedLock(m_mutex);
    checkDSopen(m_current);
    return (uint32_t) m_list.size();
}

// how many fields do we have in the current record?

uint32_t MemoryDS::fieldCount() const
{
    SharedLock(m_mutex);
    checkDSopen(m_current);
    if (m_current == nullptr) {
        return 0;
    }
    return m_current->size();
}

// access to the field by number, 0..field.size()-1
const Field& MemoryDS::operator[](uint32_t index) const
{
    SharedLock(m_mutex);
    checkDSopen(m_current);
    return (*m_current)[index];
}

Field& MemoryDS::operator[](uint32_t index)
{
    UniqueLock(m_mutex);
    checkDSopen(m_current);
    return (*m_current)[index];
}

// read this field data into external value
bool MemoryDS::readField(const char* fname, Variant& fvalue)
{
    SharedLock(m_mutex);
    try {
        fvalue = *(Variant*) &(*this)[fname];
    } catch (Exception&) {
        return false;
    }
    return true;
}

// write this field data from external value
bool MemoryDS::writeField(const char* fname, const Variant& fvalue)
{
    UniqueLock(m_mutex);
    try {
        (*this)[fname] = fvalue;
    } catch (Exception&) {
        return false;
    }
    return true;
}

bool MemoryDS::close()
{
    MemoryDS::clear();
    return true;
}

bool MemoryDS::first()
{
    UniqueLock(m_mutex);

    if (!m_list.empty()) {
        m_currentIndex = 0;
        m_current = (FieldList*) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool MemoryDS::last()
{
    UniqueLock(m_mutex);

    auto cnt = (uint32_t) m_list.size();
    if (cnt != 0) {
        m_currentIndex = cnt - 1;
        m_current = (FieldList*) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool MemoryDS::next()
{
    UniqueLock(m_mutex);

    auto cnt = (uint32_t) m_list.size();
    if (m_currentIndex + 1 < cnt) {
        m_currentIndex++;
        m_current = (FieldList*) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool MemoryDS::prior()
{
    UniqueLock(m_mutex);

    if (m_currentIndex > 0) {
        m_currentIndex--;
        m_current = (FieldList*) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool MemoryDS::find(Variant position)
{
    SharedLock(m_mutex);

    auto cnt = (uint32_t) m_list.size();
    String name;
    uint32_t i;
    switch (position.dataType()) {
        case VAR_INT:
            if (position.asInteger() < (int) cnt) {
                m_currentIndex = position.asInteger();
                m_current = (FieldList*) m_list[m_currentIndex];
                return true;
            }
            break;
        case VAR_STRING:
            name = position.asString();
            for (i = 0; i < cnt; i++) {
                FieldList& entry = *(FieldList*) m_list[i];
                if (entry["Name"].asString() == name) {
                    m_currentIndex = i;
                    m_current = (FieldList*) m_list[m_currentIndex];
                    return true;
                }
            }
            break;
        default:
            break;
    }
    return false;
}

void MemoryDS::clear()
{
    UniqueLock(m_mutex);

    auto cnt = (uint32_t) m_list.size();
    for (uint32_t i = 0; i < cnt; i++)
        delete (FieldList*) m_list[i];
    m_list.clear();
    m_current = nullptr;
    m_currentIndex = 0;
    m_eof = true;
}

void MemoryDS::push_back(FieldList* fieldList)
{
    UniqueLock(m_mutex);
    m_list.push_back(fieldList);
    m_eof = false;
}

bool MemoryDS::empty() const
{
    SharedLock(m_mutex);
    return m_list.empty();
}
