/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

FieldList::FieldList(bool indexed, bool compactXmlMode)
: m_compactXmlMode(compactXmlMode)
{
    if (indexed)
        m_index = new Map;
}

FieldList::FieldList(const FieldList& other)
{
    assign(other);
    if (other.m_index != nullptr)
        m_index = new Map;
    else
        m_index = nullptr;

    for (const auto* otherField: other) {
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

void FieldList::assign(const FieldList& other)
{
    if (other.m_index != nullptr)
        m_index = new Map;
    else
        m_index = nullptr;

    for (const auto* otherField: other) {
        auto* field = new Field(*otherField);
        m_list.push_back(field);
        if (m_index)
            (*m_index)[field->fieldName()] = field;
    }
}

void FieldList::clear()
{
    for (auto* field: *this)
        delete field;
    m_list.clear();
    if (m_index)
        m_index->clear();
}

FieldList& FieldList::operator=(const FieldList &other)
{
    if (&other != this)
        assign(other);
    return *this;
}

Field& FieldList::push_back(const String& fname, bool checkDuplicates)
{
    if (checkDuplicates) {
        const Field *pfld = findField(fname);
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
    for (const auto* field: *this)
        field->toXML(node, m_compactXmlMode);
}

#if USE_GTEST

TEST(SPTK_FieldList, ctors)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = 12345;

    FieldList fieldList2(fieldList);

    EXPECT_STREQ("id", fieldList2["name"].asString().c_str());
    EXPECT_EQ(12345, (int32_t) fieldList2["value"]);

    fieldList2["name"] = "id2";
    EXPECT_STREQ("id", fieldList["name"].asString().c_str());
    EXPECT_STREQ("id2", fieldList2["name"].asString().c_str());
}

TEST(SPTK_FieldList, push_back)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = 12345;

    EXPECT_STREQ("id", fieldList["name"].asString().c_str());
    EXPECT_EQ(12345, (int32_t) fieldList["value"]);
}

TEST(SPTK_FieldList, assign)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = 12345;

    FieldList fieldList2 = fieldList;

    EXPECT_STREQ("id", fieldList2["name"].asString().c_str());
    EXPECT_EQ(12345, (int32_t) fieldList2["value"]);
}

TEST(SPTK_FieldList, dataTypes)
{
    FieldList fieldList(true);

    DateTime testDate("2020-02-01 11:22:33Z");

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList.push_back("online", true);
    fieldList.push_back("visible", true);
    fieldList.push_back("date", true);
    fieldList.push_back("null", true);
    fieldList.push_back("text", true);
    fieldList.push_back("float_value", true);
    fieldList.push_back("money_value", true);
    fieldList.push_back("long_value", true);
    fieldList["name"] = "id";
    fieldList["value"] = 12345;
    fieldList["online"] = true;
    fieldList["visible"] = false;
    fieldList["date"] = testDate;
    fieldList["null"].setNull(VAR_STRING);
    fieldList["text"].setBuffer("1234", 5);
    fieldList["float_value"] = 12345.0;
    fieldList["money_value"].setMoney(1234567,2);
    fieldList["long_value"] = int64_t(12345678901234567);

    EXPECT_STREQ("id", fieldList["name"].asString().c_str());

    EXPECT_EQ(12345, (int32_t) fieldList["value"]);
    EXPECT_STREQ("12345", fieldList["value"].asString().c_str());

    EXPECT_TRUE(fieldList["online"].asBool());
    EXPECT_FALSE(fieldList["visible"].asBool());

    EXPECT_TRUE(fieldList["date"].asDateTime() == testDate);
    EXPECT_STREQ("2020-02-01T11:22:33Z", fieldList["date"].asDateTime().isoDateTimeString(sptk::DateTime::PA_SECONDS, true).c_str());

    EXPECT_TRUE(fieldList["null"].isNull());
    EXPECT_STREQ("1234", fieldList["text"].asString().c_str());

    EXPECT_FLOAT_EQ(12345.0, fieldList["float_value"].asFloat());
    EXPECT_STREQ("12345.000", fieldList["float_value"].asString().c_str());

    EXPECT_FLOAT_EQ(12345.67, fieldList["money_value"].asFloat());
    EXPECT_STREQ("12345.67", fieldList["money_value"].asString().c_str());

    EXPECT_EQ(int64_t(12345678901234567), fieldList["long_value"].asInt64());
    EXPECT_STREQ("12345678901234567", fieldList["long_value"].asString().c_str());
}

TEST(SPTK_FieldList, toXml)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = 12345;

    xml::Document xml;
    auto* fieldsElement = new xml::Element(xml, "fields");
    fieldList.toXML(*fieldsElement);

    Buffer buffer;
    fieldsElement->save(buffer);

    EXPECT_STREQ(buffer.c_str(), R"(<fields name="id" value="12345"/>)");
}

#endif