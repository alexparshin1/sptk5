/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       FieldList.cpp - description                            ║
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

#include <string.h>

#include <sptk5/Exception.h>
#include <sptk5/FieldList.h>

using namespace std;
using namespace sptk;

FieldList::FieldList(bool indexed, bool compactXmlMode)
{
    m_userData = 0;
    m_compactXmlMode = compactXmlMode;
    if (indexed)
        m_index = new CFieldMap;
    else
        m_index = 0L;
}

FieldList::~FieldList()
{
    clear();
    if (m_index)
        delete m_index;
}

void FieldList::clear()
{
    uint32_t cnt = (uint32_t) m_list.size();
    if (cnt) {
        for (uint32_t i = 0; i < cnt; i++)
            delete m_list[i];

        m_list.clear();
        if (m_index)
            m_index->clear();
    }
}

Field& FieldList::push_back(const char *fname, bool checkDuplicates)
{
    bool duplicate = false;
    if (checkDuplicates) {
        if (m_index) {
            CFieldMap::iterator itor = m_index->find(fname);
            if (itor != m_index->end())
                duplicate = true;
        }
        else {
            try {
                Field *pfld = fieldByName(fname);
                duplicate = (pfld != 0L);
            }
            catch (...) {
            }
        }
    }

    if (duplicate)
        throw Exception("Attempt to duplicate field name");

    Field *field = new Field(fname);

    m_list.push_back(field);

    if (m_index)
        (*m_index)[fname] = field;

    return *field;
}

Field& FieldList::push_back(Field *field)
{
    m_list.push_back(field);

    if (m_index)
        (*m_index)[field->m_name] = field;

    return *field;
}

Field *FieldList::fieldByName(const char *fname) const
{
    if (m_index) {
        CFieldMap::const_iterator itor = m_index->find(fname);
        if (itor != m_index->end())
            return itor->second;
    }
    else {
        uint32_t cnt = (uint32_t) m_list.size();
        for (uint32_t i = 0; i < cnt; i++) {
            Field *field = (Field *) m_list[i];
            if (strcmp(field->m_name.c_str(), fname) == 0)
                return field;
        }
    }
    throw Exception("Field name '" + std::string(fname) + "' not found");
}

void FieldList::toXML(CXmlNode& node) const
{
    CFieldVector::const_iterator itor = m_list.begin();
    CFieldVector::const_iterator iend = m_list.end();
    for (; itor != iend; itor++) {
        Field *field = *itor;
        field->toXML(node, m_compactXmlMode);
    }
}

Field& FieldList::next()
{
    Field *currentField = *m_fieldStreamItor;
    m_fieldStreamItor++;
    if (m_fieldStreamItor == m_list.end()) m_fieldStreamItor = m_list.begin();
    return *currentField;
}

FieldList& operator>>(FieldList& fieldList, string& data)
{
    data = fieldList.next().asString();
    return fieldList;
}

FieldList& operator>>(FieldList& fieldList, int& data)
{
    data = fieldList.next();
    return fieldList;
}

FieldList& operator>>(FieldList& fieldList, double& data)
{
    data = fieldList.next();
    return fieldList;
}

FieldList& operator>>(FieldList& fieldList, DateTime& data)
{
    data = fieldList.next();
    return fieldList;
}

FieldList& operator>>(FieldList& fieldList, Buffer& data)
{
    data = fieldList.next();
    return fieldList;
}

FieldList& operator>>(FieldList& fieldList, bool& data)
{
    data = fieldList.next().asBool();
    return fieldList;
}

FieldList& operator>>(FieldList& fieldList, CXmlNode& fields)
{
    fieldList.toXML(fields);
    return fieldList;
}
