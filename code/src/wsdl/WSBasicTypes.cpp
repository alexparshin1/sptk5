/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/wsdl/WSArray.h>
#include <sptk5/wsdl/WSBasicTypes.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

static const RegularExpression StringIsInteger(R"(^[+\-]?(0|[1-9]\d*)$)");
static const RegularExpression StringIsFloat(R"(^[+\-]?(0|[1-9]\d*)(\.\d*)?(e[+\-]\d+)?$)");

void WSBasicType::exportTo(const SNode& parent, const char* _name) const
{
    using enum VariantDataType;

    const String elementName = _name == nullptr ? name() : _name;
    if (const String text(isNull() ? "" : asString());
        m_optional && (isNull() || text.empty()))
    {
        return;
    }

    if (parent->type() != Node::Type::Array)
    {
        if (elementName.empty())
        {
            return;
        }
        switch (dataType())
        {
            case VAR_BOOL:
                parent->set(elementName, m_value.asBool());
                break;
            case VAR_INT:
                parent->set(elementName, m_value.asInteger());
                break;
            case VAR_INT64:
                parent->set(elementName, m_value.asInt64());
                break;
            case VAR_FLOAT:
                parent->set(elementName, m_value.asFloat());
                break;
            default:
                parent->set(elementName, asString());
                break;
        }
    }
    else
    {
        switch (dataType())
        {
            case VAR_BOOL:
                parent->pushValue(m_value.asBool());
                break;
            case VAR_INT:
                parent->pushValue(m_value.asInteger());
                break;
            case VAR_INT64:
                parent->pushValue(m_value.asInt64());
                break;
            case VAR_FLOAT:
                parent->pushValue(m_value.asFloat());
                break;
            default:
                parent->pushValue(asString());
                break;
        }
    }
}

void WSBasicType::throwIfNull(const String& parentTypeName) const
{
    if (isNull())
    {
        throw SOAPException("Element '" + name() + "' is required in '" + parentTypeName + "'.");
    }
}

void WSString::load(const SNode& attr, bool nullLargeData)
{
    constexpr size_t longStringLength = 256;
    if (attr->type() == Node::Type::Null || (nullLargeData && attr->getString().length() > longStringLength))
    {
        setNull(VariantDataType::VAR_STRING);
    }
    else
    {
        auto str = attr->getString();
        owaspCheck(str);
        value().setString(str);
    }
}

void WSString::load(const String& attr)
{
    owaspCheck(attr);
    value().setString(attr);
}

void WSString::load(const Field& field)
{
    load(field.asString());
}

void WSBool::load(const SNode& attr, bool)
{
    if (attr->type() == Node::Type::Null)
    {
        setNull(VariantDataType::VAR_BOOL);
    }
    else
    {
        value().setBool(attr->getBoolean());
    }
}

void WSBool::load(const String& attr)
{
    if (attr.empty())
    {
        setNull(VariantDataType::VAR_BOOL);
    }
    else
    {
        if (attr == "true")
        {
            value().setBool(true);
        }
        else if (attr == "false")
        {
            value().setBool(false);
        }
        else
        {
            throw Exception("Invalid data: not true or false");
        }
    }
}

void WSBool::load(const Field& field)
{
    if (field.isNull())
    {
        setNull(VariantDataType::VAR_BOOL);
    }
    else
    {
        this->value().setBool(field.asBool());
    }
}

void WSDate::load(const SNode& attr, bool)
{
    const String text = attr->getString();
    if (attr->type() == Node::Type::Null || text.empty())
    {
        setNull(VariantDataType::VAR_DATE);
    }
    else
    {
        value().setDateTime(DateTime(text.c_str()), false);
    }
}

void WSDate::load(const String& attr)
{
    if (attr.empty())
    {
        setNull(VariantDataType::VAR_DATE);
    }
    else
    {
        const DateTime dt(attr.c_str());
        value().setDateTime(dt, true);
    }
}

void WSDate::load(const Field& field)
{
    if (field.isNull())
    {
        setNull(VariantDataType::VAR_DATE);
    }
    else
    {
        this->value().setDateTime(field.asDate(), true);
    }
}

void WSDateTime::load(const SNode& attr, bool)
{
    const String text = attr->getText();
    if (text.empty())
    {
        setNull(VariantDataType::VAR_DATE_TIME);
    }
    else
    {
        const DateTime dt(text.c_str());
        value().setDateTime(dt);
    }
}

void WSDateTime::load(const String& attr)
{
    if (attr.empty())
    {
        setNull(VariantDataType::VAR_DATE_TIME);
    }
    else
    {
        const DateTime dt(attr.c_str());
        value().setDateTime(DateTime(attr.c_str()));
    }
}

void WSDateTime::load(const Field& field)
{
    if (field.isNull())
    {
        setNull(VariantDataType::VAR_DATE_TIME);
    }
    else
    {
        this->value().setDateTime(field.asDateTime());
    }
}

String WSDateTime::asString() const
{
    const DateTime dt = value().asDateTime();
    return dt.isoDateTimeString();
}

void WSDouble::load(const SNode& attr, bool)
{
    if (attr->type() == Node::Type::Null)
    {
        setNull(VariantDataType::VAR_FLOAT);
    }
    else
    {
        if (attr->type() == Node::Type::Number)
        {
            value().setFloat(attr->getNumber());
            return;
        }

        if (attr->type() == Node::Type::Text)
        {
            const String textValue = attr->getText();
            if (optional() && textValue.empty())
            {
                setNull(VariantDataType::VAR_FLOAT);
                return;
            }

            if (StringIsInteger.m(textValue) || StringIsFloat.m(textValue))
            {
                value().setFloat(attr->getNumber());
                return;
            }
        }

        throw Exception(attr->name() + " is not a float number");
    }
}

void WSDouble::load(const String& attr)
{
    if (attr.empty())
    {
        setNull(VariantDataType::VAR_INT);
    }
    else
    {
        if (StringIsInteger.m(attr) || StringIsFloat.m(attr))
        {
            value().setFloat(strtod(attr.c_str(), nullptr));
        }
        throw Exception("Value is not a float number");
    }
}

void WSDouble::load(const Field& field)
{
    if (field.isNull())
    {
        setNull(VariantDataType::VAR_FLOAT);
    }
    else
    {
        this->value().setFloat(field.asFloat());
    }
}

void WSInteger::load(const SNode& attr, bool)
{
    if (attr->type() == Node::Type::Null)
    {
        setNull(VariantDataType::VAR_INT64);
    }
    else
    {
        if (attr->type() == Node::Type::Number)
        {
            value().setInt64((int64_t) attr->getNumber());
            return;
        }

        if (attr->type() == Node::Type::Text)
        {
            auto textValue = attr->getText();

            if (optional() && textValue.empty())
            {
                setNull(VariantDataType::VAR_INT64);
                return;
            }

            if (StringIsInteger.m(textValue))
            {
                value().setInt64((int64_t) attr->getNumber());
                return;
            }
        }

        throw Exception(attr->name() + " is not an integer number");
    }
}

void WSInteger::load(const String& attr)
{
    if (attr.empty())
    {
        setNull(VariantDataType::VAR_INT64);
    }
    else
    {
        if (!StringIsInteger.m(attr))
        {
            throw Exception("Value is not an integer number");
        }

        constexpr int decimal = 10;
        value().setInt64(strtol(attr.c_str(), nullptr, decimal));
    }
}

void WSInteger::load(const Field& field)
{
    if (field.isNull())
    {
        setNull(VariantDataType::VAR_INT64);
    }
    else
    {
        this->value().setInt64(field.asInt64());
    }
}

String sptk::wsTypeIdToName(const String& typeIdName)
{
    static const RegularExpression matchClassName("^\\d+C([A-Z]\\S+)$");

    if (auto matches = matchClassName.m(typeIdName);
        matches)
    {
        return matches.groups()[0].value;
    }

    return "Unknown";
}
