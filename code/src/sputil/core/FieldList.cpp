/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       FieldList.cpp - description                            ║
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

#include <cstring>

#include <sptk5/Exception.h>
#include <sptk5/FieldList.h>

using namespace std;
using namespace sptk;

FieldList::FieldList(bool indexed, bool compactXmlMode)
: m_compactXmlMode(compactXmlMode)
{
    m_userData = nullptr;
    if (indexed)
        m_index = new Map;
    else
        m_index = nullptr;
}

FieldList::FieldList(const FieldList& other)
: m_userData(other.m_userData), m_compactXmlMode(false)
{
    if (other.m_index != nullptr)
        m_index = new Map;
    else
        m_index = nullptr;

    for (auto* otherField: other) {
        auto* field = new Field(*otherField);
        m_list.push_back(field);
        if (m_index)
            (*m_index)[field->fieldName()] = field;
    }
}

FieldList::~FieldList()
{
    clear();
    delete m_index;
}

void FieldList::clear()
{
    for (auto* field: *this)
        delete field;
    m_list.clear();
    if (m_index)
        m_index->clear();
}

Field& FieldList::push_back(const String& fname, bool checkDuplicates)
{
    if (checkDuplicates) {
        Field *pfld = findField(fname);
        if (pfld != nullptr)
            throw Exception("Attempt to duplicate field name");
    }

    auto* field = new Field(fname);

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

Field *FieldList::findField(const String& fname) const
{
    if (m_index) {
        auto itor = m_index->find(fname);
        if (itor != m_index->end())
            return itor->second;
    }
    else {
        for (auto* field: *this) {
            if (strcmp(field->m_name.c_str(), fname.c_str()) == 0)
                return field;
        }
    }
    return nullptr;
}

void FieldList::toXML(xml::Node& node) const
{
    for (auto* field: *this)
        field->toXML(node, m_compactXmlMode);
}

#if USE_GTEST

TEST(SPTK_FieldList, copy)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = 12345;

    FieldList fieldList2(fieldList);

    EXPECT_STREQ("id", fieldList2["name"].asString().c_str());
    EXPECT_EQ(12345, (int32_t) fieldList2["value"]);
}

TEST(SPTK_FieldList, push_back)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = 1234;
    fieldList["value"] = 12345;

    EXPECT_STREQ("id", fieldList["name"].asString().c_str());
    EXPECT_EQ(12345, (int32_t) fieldList["value"]);
}
#endif