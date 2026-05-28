/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-11                                             ║
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

#include <iomanip>
#include <sptk5/Field.h>

using namespace std;
using namespace sptk;

Field::Field(const String& name)
    : m_name(name)
    , m_displayName(name)
{
    m_view.width = -1;
    m_view.flags = 4; // FL_ALIGN_LEFT
    m_view.visible = true;
    m_view.precision = 3; // default precision, only affects floating point fields
    dataSize(0);
}

Field& Field::operator=(const Variant& C)
{
    if (this == &C)
    {
        return *this;
    }

    setData(C);
    return *this;
}

Field& Field::operator=(const bool value)
{
    setBool(value);
    return *this;
}

Field& Field::operator=(const int32_t value)
{
    setInteger(value);
    return *this;
}

Field& Field::operator=(const int64_t value)
{
    setInt64(value);
    return *this;
}

Field& Field::operator=(const double value)
{
    setFloat(value);
    return *this;
}

Field& Field::operator=(const char* value)
{
    setString(value);
    return *this;
}

Field& Field::operator=(const String& value)
{
    setBuffer(reinterpret_cast<const uint8_t*>(value.c_str()), value.length(), VariantDataType::VAR_STRING);
    return *this;
}

Field& Field::operator=(const DateTime& value)
{
    setDateTime(value);
    return *this;
}

Field& Field::operator=(const MoneyData& value)
{
    setMoney(value.quantity(), value.scale());
    return *this;
}

Field& Field::operator=(const uint8_t* value)
{
    setImagePtr(value);
    return *this;
}

Field& Field::operator=(const Buffer& value)
{
    setBuffer(value.data(), value.bytes(), VariantDataType::VAR_BUFFER);
    return *this;
}

String Field::asString() const
{
    String result;

    if (isNull())
    {
        return result;
    }

    switch (dataType())
    {
        using enum VariantDataType;
        case VAR_BOOL:
            result = get<bool>() != 0 ? "true" : "false";
            break;

        case VAR_INT:
        case VAR_IMAGE_NDX:
            result = to_string(m_data.get<int32_t>());
            break;

        case VAR_INT64:
            result = to_string(m_data.get<int64_t>());
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
            if (isExternalBuffer())
            {
                result.assign(bit_cast<const char*>(get<const uint8_t*>()), dataSize());
            }
            else if (dataType() == VAR_STRING)
            {
                result = get<String>();
            }
            else
            {
                const auto& buffer = get<Buffer>();
                result.assign(buffer.c_str(), dataSize());
            }
            break;

        case VAR_DATE:
            result = epochDataToDateTimeString(true);
            break;

        case VAR_DATE_TIME:
            result = epochDataToDateTimeString(false);
            break;

        case VAR_IMAGE_PTR: {
            const auto* ptr = bit_cast<const void*>(get<const uint8_t*>());
            result = format("0x{:p}", ptr);
            break;
        }

        default:
            throw Exception("Can't convert field " + fieldName() + " to type String");
    }
    return result;
}

String Field::epochDataToDateTimeString(const bool dateOnly) const
{
    const auto& dateTime(get<DateTime>());
    if (dateOnly)
    {
        return dateTime.dateString();
    }
    return dateTime.dateString() + " " + dateTime.timeString(DateTime::PF_TIMEZONE, DateTime::PrintAccuracy::SECONDS);
}

String Field::doubleDataToString() const
{
    stringstream output;
    output << fixed << setprecision(static_cast<int>(m_view.precision)) << get<double>();
    return output.str();
}

void Field::exportTo(const xdoc::SNode& node, const bool compactXmlMode, const bool detailedInfo, const bool nullLargeData) const
{
    auto value = asString();

    xdoc::SNode element;

    if (constexpr size_t minLargeFieldSize {256};
        nullLargeData && value.length() >= minLargeFieldSize)
    {
        value = "";
    }

    if (dataType() == VariantDataType::VAR_TEXT && !value.empty())
    {
        element = node->pushNode(fieldName(), xdoc::Node::Type::CData);
        element->set(value);
    }
    else
    {
        if (compactXmlMode)
        {
            node->attributes().set(fieldName(), value);
        }
        else
        {
            element = node->pushValue(fieldName(), Variant(value));
        }
    }

    if (detailedInfo && element)
    {
        element->attributes().set("type", typeName(dataType()));
        element->attributes().set("size", to_string(dataSize()));
    }
}
