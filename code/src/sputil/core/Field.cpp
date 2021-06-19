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

#include <sptk5/Field.h>
#include <iomanip>

using namespace std;
using namespace sptk;

Field::Field(const String& name)
: m_name(name), m_displayName(name)
{
    m_view.width = -1;
    m_view.flags = 4;       // FL_ALIGN_LEFT
    m_view.visible = true;
    m_view.precision = 3;   // default precision, only affects floating point fields
    dataSize(0);
}

void Field::setNull(VariantType vtype)
{
    switch (dataType()) {
        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (isExternalBuffer())
                m_data.getBuffer().data = nullptr;
            else if (m_data.getBuffer().data != nullptr)
                m_data.getBuffer().data[0] = 0;

            break;

        default:
            m_data.getInt64() = 0;
            break;
    }

    if (vtype == VAR_NONE)
        m_data.type(uint16_t (m_data.type() | VAR_NULL));
    else
        m_data.type(vtype | VAR_NULL);
}

String Field::asString() const
{
    constexpr int maxPrintLength = 64;

    String result;
    array<char,maxPrintLength+1> print_buffer;
    int len = 0;

    if (isNull())
        return result;

    switch (dataType()) {
        case VAR_BOOL:
            result = m_data.getInteger() != 0 ? "true" : "false";
            break;

        case VAR_INT:
        case VAR_IMAGE_NDX:
            len = snprintf(print_buffer.data(), maxPrintLength, "%i", m_data.getInteger());
            result.assign(print_buffer.data(), len);
            break;

        case VAR_INT64:
#ifndef _WIN32
            len = snprintf(print_buffer.data(), maxPrintLength, "%li", m_data.getInt64());
#else
            len = snprintf(print_buffer, sizeof(print_buffer), "%lli", m_data.getInt64());
#endif
            result.assign(print_buffer.data(), len);
            break;

        case VAR_FLOAT:
            result = doubleDataToString();
            break;

        case VAR_MONEY:
            result = moneyDataToString();
            break;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (m_data.getBuffer().data != nullptr)
                result = m_data.getBuffer().data;
            break;

        case VAR_DATE:
            result = DateTime(chrono::microseconds(m_data.getInt64())).dateString();
            break;

        case VAR_DATE_TIME:
            result = epochDataToDateTimeString();
            break;

        case VAR_IMAGE_PTR:
            len = snprintf(print_buffer.data(), maxPrintLength, "%p", m_data.getImagePtr());
            result.assign(print_buffer.data(), len);
            break;

        default:
            throw Exception("Can't convert field " + fieldName() + " to type String");
    }
    return result;
}

String Field::epochDataToDateTimeString() const
{
    DateTime dt(chrono::microseconds(m_data.getInt64()));
    return dt.dateString() + " " + dt.timeString(DateTime::PF_TIMEZONE, DateTime::PrintAccuracy::SECONDS);
}

String Field::doubleDataToString() const
{
    stringstream output;
    output << fixed << setprecision((int)m_view.precision) << m_data.getFloat();
    return output.str();
}

void Field::toXML(xml::Node& node, bool compactXmlMode) const
{
    String value = asString();

    if (!value.empty()) {
        xml::Element* element = nullptr;

        if (dataType() == VAR_TEXT) {
            element = new xml::Element(node, fieldName());
            new xml::CDataSection(*element, value);
        } else {
            if (compactXmlMode)
                node.setAttribute(fieldName(), value);
            else {
                element = new xml::Element(node, "field");
                new xml::Text(element, value.c_str());
            }
        }

        if (!compactXmlMode) {
            element->setAttribute("name", fieldName());
            element->setAttribute("type", Variant::typeName(dataType()));
            element->setAttribute("size", int2string((uint32_t) dataSize()));
        }
    }
}

#if USE_GTEST

TEST(SPTK_Field, move_ctor_assign)
{
    constexpr int testInteger = 10;
    Field   field1("f1");
    field1 = testInteger;

    Field   field2(move(field1));
    EXPECT_EQ(field2.asInteger(), testInteger);
    EXPECT_EQ(field1.isNull(), true);

    Field   field3("f3");
    field3 = move(field2);
    EXPECT_EQ(field3.asInteger(), testInteger);
    EXPECT_EQ(field2.isNull(), true);
}

TEST(SPTK_Field, double)
{
    Field   field1("f1");

    constexpr double testDouble = 12345678.123456;
    field1 = testDouble;
    field1.view().precision = 3;

    EXPECT_DOUBLE_EQ(field1.asFloat(), testDouble);
    EXPECT_STREQ(field1.asString().c_str(), "12345678.123");
}

TEST(SPTK_Field, money)
{
    constexpr long testLong = 1234567890123456789L;
    constexpr int64_t testInt64 = 12345678901;
    constexpr int scaleDigits = 8;

    MoneyData money1(testLong, scaleDigits);
    MoneyData money2(-testLong, scaleDigits);
    Field   field1("f1");

    field1.setMoney(money1);
    EXPECT_EQ(field1.asInt64(), testInt64);
    EXPECT_STREQ(field1.asString().c_str(), "12345678901.23456789");

    field1.setMoney(money2);
    EXPECT_EQ(field1.asInt64(), -testInt64);
    EXPECT_STREQ(field1.asString().c_str(), "-12345678901.23456789");
}

#endif