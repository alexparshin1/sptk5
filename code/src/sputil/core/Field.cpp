/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Field.cpp - description                                ║
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

#include <sptk5/Field.h>

using namespace std;
using namespace sptk;

Field::Field(const char* name)
: m_name(name), displayName(name)
{
    view.width = -1;
    view.flags = 4; // FL_ALIGN_LEFT
    view.visible = true;
    view.precision = 3; // default precision, only affects floating point fields
    dataSize(0);
}

Field::Field(const Field& other)
: Variant(other), m_name(other.m_name), displayName(other.m_name)
{
    view.width = -1;
    view.flags = 4; // FL_ALIGN_LEFT
    view.visible = true;
    view.precision = 3; // default precision, only affects floating point fields
}

void Field::setNull(VariantType vtype)
{
    switch (dataType()) {
        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if ((m_dataType & VAR_EXTERNAL_BUFFER) == VAR_EXTERNAL_BUFFER)
                m_data.buffer.data = nullptr;
            else if (m_data.buffer.data != nullptr)
                m_data.buffer.data[0] = 0;

            break;

        default:
            m_data.int64Data = 0;
            break;
    }

    if (vtype == VAR_NONE)
        m_dataType |= VAR_NULL;
    else
        m_dataType = uint16_t(vtype | VAR_NULL);
}

String Field::asString() const
{
    char print_buffer[64];
    int len;

    if ((m_dataType & VAR_NULL) == VAR_NULL) return "";

    switch (dataType()) {
        case VAR_BOOL:
            if (m_data.intData != 0)
                return "true";
            else
                return "false";

        case VAR_INT:
            len = snprintf(print_buffer, sizeof(print_buffer), "%i", m_data.intData);
            return String(print_buffer, len);

        case VAR_INT64:
#ifndef _WIN32
            len = snprintf(print_buffer, sizeof(print_buffer), "%li", m_data.int64Data);
#else
            len = snprintf(print_buffer, sizeof(print_buffer), "%lli", m_data.int64Data);
#endif
            return String(print_buffer, len);

        case VAR_FLOAT: {
            char formatString[10];
            snprintf(formatString, sizeof(formatString), "%%0.%if", view.precision);
            len = snprintf(print_buffer, sizeof(print_buffer), formatString, m_data.floatData);
            return String(print_buffer, len);
        }

        case VAR_MONEY: {
            char    format[32];
            int64_t absValue;
            char* formatPtr = format;

            if (m_data.moneyData.quantity < 0) {
                *formatPtr = '-';
                formatPtr++;
                absValue = -m_data.moneyData.quantity;
            } else
                absValue = m_data.moneyData.quantity;

            snprintf(formatPtr, sizeof(format) - 2, "%%Ld.%%0%dLd", m_data.moneyData.scale);
            int64_t intValue = absValue / MoneyData::dividers[m_data.moneyData.scale];
            int64_t fraction = absValue % MoneyData::dividers[m_data.moneyData.scale];
            len = snprintf(print_buffer, sizeof(print_buffer) - 1, format, intValue, fraction);
            return String(print_buffer, len);
        }

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (m_data.buffer.data == nullptr)
                return "";

            return m_data.buffer.data;

        case VAR_DATE:
            return DateTime(chrono::microseconds(m_data.timeData)).dateString();

        case VAR_DATE_TIME: {
            DateTime dt(chrono::microseconds(m_data.timeData));
            return dt.dateString() + " " + dt.timeString(DateTime::PF_TIMEZONE, DateTime::PA_SECONDS);
        }

        case VAR_IMAGE_PTR:
            len = snprintf(print_buffer, sizeof(print_buffer), "%p", m_data.imagePtr);
            return String(print_buffer, len);

        case VAR_IMAGE_NDX:
            len = snprintf(print_buffer, sizeof(print_buffer), "%i", m_data.imageNdx);
            return String(print_buffer, len);

        default:
            throw Exception("Can't convert field for that type");
    }
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
                element->value(value);
            }
        }

        if (!compactXmlMode) {
            element->setAttribute("name", fieldName());
            element->setAttribute("type", Variant::typeName(dataType()));
            element->setAttribute("size", int2string((uint32_t) dataSize()));
        }
    }
}
