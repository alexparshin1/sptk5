/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;

FieldList::FieldList(bool indexed)
{
    if (indexed)
    {
        m_index = make_shared<Map>();
    }
}

void FieldList::clear()
{
    m_list.clear();
    if (m_index)
    {
        m_index->clear();
    }
}

Field& FieldList::push_back(const String& fname, bool checkDuplicates)
{
    if (checkDuplicates)
    {
        auto pfld = findField(fname);
        if (pfld)
        {
            throw Exception("Attempt to duplicate field name");
        }
    }

    auto field = make_shared<Field>(fname);

    m_list.push_back(field);

    if (m_index)
    {
        (*m_index)[fname] = field;
    }

    return *field;
}

Field& FieldList::push_back(const SField& field)
{
    m_list.push_back(field);

    if (m_index)
    {
        (*m_index)[field->m_name] = field;
    }

    return *field;
}

SField FieldList::findField(const String& fname) const
{
    if (m_index)
    {
        auto itor = m_index->find(fname);
        if (itor != m_index->end())
        {
            return itor->second;
        }
    }
    else
    {
        for (auto& field: *this)
        {
            if (strcasecmp(field->m_name.c_str(), fname.c_str()) == 0)
            {
                return field;
            }
        }
    }
    return nullptr;
}

void FieldList::exportTo(const xdoc::SNode& node, bool compactMode) const
{
    for (const auto& field: *this)
    {
        field->exportTo(node, compactMode);
    }
}

#ifdef USE_GTEST

static constexpr int testInteger = 12345;

TEST(SPTK_FieldList, ctor)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = testInteger;
}

TEST(SPTK_FieldList, push_back)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = testInteger;

    EXPECT_STREQ("id", fieldList["name"].asString().c_str());
    EXPECT_EQ(testInteger, (int32_t) fieldList["value"]);
}

TEST(SPTK_FieldList, move)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "id";
    fieldList["value"] = testInteger;

    FieldList fieldList2 = move(fieldList);

    EXPECT_STREQ("id", fieldList2["name"].asString().c_str());
    EXPECT_EQ(testInteger, (int32_t) fieldList2["value"]);
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
    fieldList["value"] = testInteger;
    fieldList["online"].setBool(true);
    fieldList["visible"].setBool(false);
    fieldList["date"] = testDate;
    fieldList["null"].setNull(VariantDataType::VAR_STRING);
    fieldList["text"].setBuffer((const uint8_t*) "1234", 5);
    fieldList["float_value"] = double(testInteger);
    fieldList["money_value"].setMoney(1234567, 2);
    fieldList["long_value"] = int64_t(12345678901234567);

    EXPECT_STREQ("id", fieldList["name"].asString().c_str());

    EXPECT_EQ(testInteger, (int32_t) fieldList["value"]);
    EXPECT_STREQ("12345", fieldList["value"].asString().c_str());

    EXPECT_TRUE(fieldList["online"].asBool());
    EXPECT_STREQ("true", fieldList["online"].asString().c_str());
    EXPECT_FALSE(fieldList["visible"].asBool());
    EXPECT_STREQ("false", fieldList["visible"].asString().c_str());

    EXPECT_TRUE(fieldList["date"].asDateTime() == testDate);
    EXPECT_STREQ("2020-02-01T11:22:33Z",
                 fieldList["date"].asDateTime().isoDateTimeString(sptk::DateTime::PrintAccuracy::SECONDS,
                                                                  true).c_str());

    EXPECT_TRUE(fieldList["null"].isNull());
    EXPECT_STREQ("1234", fieldList["text"].asString().c_str());

    EXPECT_DOUBLE_EQ(double(testInteger), fieldList["float_value"].asFloat());
    EXPECT_STREQ("12345.000", fieldList["float_value"].asString().c_str());

    EXPECT_DOUBLE_EQ(12345.67, fieldList["money_value"].asFloat());
    EXPECT_STREQ("12345.67", fieldList["money_value"].asString().c_str());

    EXPECT_EQ(int64_t(12345678901234567), fieldList["long_value"].asInt64());
    EXPECT_STREQ("12345678901234567", fieldList["long_value"].asString().c_str());
    EXPECT_DOUBLE_EQ(double(12345678901234567), fieldList["long_value"].asFloat());
}

TEST(SPTK_FieldList, toXml)
{
    FieldList fieldList(true);

    fieldList.push_back("name", true);
    fieldList.push_back("value", true);
    fieldList["name"] = "John";
    fieldList["value"] = testInteger;

    xdoc::Document xml;
    const auto& fieldsElement = xml.root()->pushNode("fields", xdoc::Node::Type::Object);
    fieldList.exportTo(fieldsElement, false);

    Buffer buffer;
    fieldsElement->exportTo(xdoc::DataFormat::XML, buffer, false);

    EXPECT_STREQ(buffer.c_str(),
                 R"(<fields><field name="name" type="string" size="4">John</field><field name="value" type="int" size="4">12345</field></fields>)");
}

#endif
