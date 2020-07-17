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

#include <sptk5/wsdl/WSComplexType.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

void WSComplexType::copyFrom(const WSComplexType& other)
{
    xml::Document xml;
    auto* element = new xml::Element(xml, "temp");
    other.unload(element);
    load(element);
}

void WSComplexType::unload(QueryParameterList& output, const char* paramName, const WSBasicType* elementOrAttribute)
{
    if (elementOrAttribute == nullptr)
        return;

    sptk::QueryParameter* param = output.find(paramName);
    if (param != nullptr)
        *param = *elementOrAttribute;
}

void WSComplexType::addElement(xml::Element* parent, const char* name) const
{
    if (m_exportable) {
        const char* elementName = name == nullptr ? m_name.c_str() : name;
        unload(new xml::Element(parent, elementName));
    }
}

void WSComplexType::addElement(json::Element* parent) const
{
    if (m_exportable) {
        json::Element* element;
        if (parent->is(json::JDT_ARRAY))
            element = parent->push_back();
        else
            element = parent->set(m_name);
        unload(element);
    }
}

String WSComplexType::toString(bool asJSON) const
{
    xml::Document   outputXML;
    json::Document  outputJSON;
    Buffer          output;

    auto element = new xml::Element(outputXML, "type");
    unload(element);

    if (asJSON) {
        outputXML.exportTo(outputJSON.root());
        outputJSON.exportTo(output, false);
    }
    else
        outputXML.save(output, 2);

    return String(output.c_str(), output.bytes());
}

void WSComplexType::throwIfNull(const String& parentTypeName) const
{
    if (!m_loaded)
        throw SOAPException("Element '" + m_name + "' is required in '" + parentTypeName + "'.");
}

WSComplexType::FieldNameIndex::FieldNameIndex(initializer_list<const char*> list)
{
    int id = 0;
    for (const auto* item: list) {
        emplace_back(item, strlen(item), id);
        m_index.emplace(item, strlen(item), id);
        ++id;
    }
}

int WSComplexType::FieldNameIndex::indexOf(const String& name) const
{
    auto itor = m_index.find(name);
    if (itor == m_index.end())
        return -1;
    return itor->ident();
}

#if USE_GTEST

TEST(SPTK_WSComplexType, FieldNameIndex)
{
    WSComplexType::FieldNameIndex fields = { "zero", "one", "two", "three" };
    EXPECT_STREQ("zero,one,two,three", fields.join(",").c_str());
    EXPECT_EQ(1, fields.indexOf("one"));
}

#endif