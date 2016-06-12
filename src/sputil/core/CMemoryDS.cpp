/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CMemoryDS.cpp - description                            ║
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

#include <sptk5/CMemoryDS.h>

using namespace std;
using namespace sptk;

#define checkDSopen(ds) if (!ds) throw Exception("Dataset isn't open") 

// access to the field by name
const CField& CMemoryDS::operator [] (const char *field_name) const
{
    checkDSopen(m_current);
    return (*m_current)[field_name];
}

CField& CMemoryDS::operator [] (const char *field_name)
{
    checkDSopen(m_current);
    return (*m_current)[field_name];
}

uint32_t CMemoryDS::recordCount() const
{
    checkDSopen(m_current);
    return (uint32_t) m_list.size();
}

// how many fields do we have in the current record?

uint32_t CMemoryDS::fieldCount() const
{
    if (!m_current) return 0;
    checkDSopen(m_current);
    return m_current->size();
}

// access to the field by number, 0..field.size()-1
const CField& CMemoryDS::operator [] (uint32_t index) const
{
    checkDSopen(m_current);
    return (*m_current)[index];
}

CField& CMemoryDS::operator [] (uint32_t index)
{
    checkDSopen(m_current);
    return (*m_current)[index];
}

// read this field data into external value
bool CMemoryDS::readField(const char *fname, Variant& fvalue)
{
    try {
        fvalue = (*this)[fname];
    } catch (Exception &) {
        return false;
    }
    return true;
}

// write this field data from external value
bool CMemoryDS::writeField(const char *fname, const Variant& fvalue)
{
    try {
        (*this)[fname] = fvalue;
    } catch (Exception &) {
        return false;
    }
    return true;
}

bool CMemoryDS::close()
{
    clear();
    return true;
}

bool CMemoryDS::first()
{
    if (m_list.size()) {
        m_currentIndex = 0;
        m_current = (CFieldList *) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool CMemoryDS::last()
{
    uint32_t cnt = (uint32_t) m_list.size();
    if (cnt) {
        m_currentIndex = cnt - 1;
        m_current = (CFieldList *) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool CMemoryDS::next()
{
    uint32_t cnt = (uint32_t) m_list.size();
    if (m_currentIndex + 1 < cnt) {
        m_currentIndex++;
        m_current = (CFieldList *) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool CMemoryDS::prior()
{
    if (m_currentIndex > 0) {
        m_currentIndex--;
        m_current = (CFieldList *) m_list[m_currentIndex];
        m_eof = false;
        return true;
    }
    m_eof = true;
    return false;
}

bool CMemoryDS::find(Variant position)
{
    uint32_t cnt = (uint32_t) m_list.size();
    string name;
    uint32_t i;
    switch (position.dataType()) {
        case VAR_INT:
            if (position.asInteger() < (int) cnt) {
                m_currentIndex = position;
                m_current = (CFieldList *) m_list[m_currentIndex];
                return true;
            }
            break;
        case VAR_STRING:
            name = position.asString();
            for (i = 0; i < cnt; i++) {
                CFieldList& entry = *(CFieldList *) m_list[i];
                if (entry["Name"].asString() == name) {
                    m_currentIndex = i;
                    m_current = (CFieldList *) m_list[m_currentIndex];
                    return true;
                }
            }
            break;
        default:
            break;
    }
    return false;
}

void CMemoryDS::clear()
{
    uint32_t cnt = (uint32_t) m_list.size();
    for (uint32_t i = 0; i < cnt; i++) {
        CFieldList *df = (CFieldList *) m_list[i];
        delete df;
    }
    m_list.clear();
    m_current = 0L;
    m_currentIndex = 0;
    m_eof = true;
}
