/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSBasicTypes.cpp - description                        ║
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

#include <sptk5/wsdl/WSBasicTypes.h>

using namespace std;
using namespace sptk;

xml::Element* WSBasicType::addElement(xml::Element* parent, const char* _name) const
{
    String elementName = _name == nullptr? name() : _name;
    String text(asString());
    if (m_optional && (isNull() || text.empty()))
        return nullptr;
    auto* element = new xml::Element(*parent, elementName);
    element->text(text);
    return element;
}

json::Element* WSBasicType::addElement(json::Element* parent) const
{
    String text(asString());
    if (m_optional && (isNull() || text.empty()))
        return nullptr;
    json::Element* element;
    switch (dataType()) {
        case VAR_BOOL:
            element = parent->set(name(), asBool());
            break;
        case VAR_INT:
            element = parent->set(name(), asInteger());
            break;
        case VAR_INT64:
            element = parent->set(name(), asInt64());
            break;
        case VAR_FLOAT:
            element = parent->set(name(), asFloat());
            break;
        default:
            element = parent->set(name(), asString());
            break;
    }

    return element;
}

void WSString::load(const xml::Node* attr)
{
    setString(attr->text());
}

void WSString::load(const json::Element* attr)
{
    if (attr->isNull())
        setNull(VAR_STRING);
    else
        setString(attr->getString());
}

void WSString::load(const String& attr)
{
    setString(attr);
}

void WSString::load(const Field& field)
{
    if (field.isNull())
        setNull(VAR_STRING);
    else
        setString(field.asString());
}

void WSBool::load(const xml::Node* attr)
{
    String text = attr->text();
    if (text.empty())
        setNull(VAR_BOOL);
    else
        setBool(text == "true");
}

void WSBool::load(const json::Element* attr)
{
    if (attr->isNull())
        setNull(VAR_BOOL);
    else
        setBool(attr->getBoolean());
}

void WSBool::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_BOOL);
    else
        setBool(attr == "true");
}

void WSBool::load(const Field& field)
{
    if (field.isNull())
        setNull(VAR_BOOL);
    else
        setBool(field.asBool());
}

void WSDate::load(const xml::Node* attr)
{
    String text = attr->text();
    if (text.empty())
        setNull(VAR_DATE);
    else
        setDateTime(DateTime(text.c_str()), true);
}

void WSDate::load(const json::Element* attr)
{
    String text = attr->getString();
    if (attr->isNull() || text.empty())
        setNull(VAR_DATE);
    else
        setDateTime(DateTime(text.c_str()), true);
}

void WSDate::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_DATE);
    else
        setDateTime(DateTime(attr.c_str()), true);
}

void WSDate::load(const Field& field)
{
    if (field.isNull())
        setNull(VAR_DATE);
    else
        setDateTime(field.asDate(), true);
}

void WSDateTime::load(const xml::Node* attr)
{
    String text = attr->text();
    if (text.empty())
        setNull(VAR_DATE_TIME);
    else
        setDateTime(DateTime(text.c_str()));
}

void WSDateTime::load(const json::Element* attr)
{
    String text = attr->getString();
    if (text.empty())
        setNull(VAR_DATE_TIME);
    else
        setDateTime(DateTime(text.c_str()));
}

void WSDateTime::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_DATE_TIME);
    else
        setDateTime(DateTime(attr.c_str()));
}

void WSDateTime::load(const Field& field)
{
    if (field.isNull())
        setNull(VAR_DATE_TIME);
    else
        setDateTime(field.asDateTime());
}

String WSDateTime::asString() const
{
    DateTime dt = asDateTime();
    return dt.isoDateTimeString();
}

void WSDouble::load(const xml::Node* attr)
{
    setFloat(strtod(attr->text().c_str(), nullptr));
}

void WSDouble::load(const json::Element* attr)
{
    setFloat(attr->getNumber());
}

void WSDouble::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_INT);
    else
        setFloat(strtod(attr.c_str(), nullptr));
}

void WSDouble::load(const Field& field)
{
    if (field.isNull())
        setNull(VAR_FLOAT);
    else
        setFloat(field.asFloat());
}

void WSInteger::load(const xml::Node* attr)
{
    String text = attr->text();
    if (text.empty())
        setNull(VAR_INT64);
    else
        setInt64(strtol(text.c_str(), nullptr, 10));
}

void WSInteger::load(const json::Element* attr)
{
    if (attr->isNull())
        setNull(VAR_INT64);
    else
        setInt64(attr->getNumber());
}

void WSInteger::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_INT64);
    else
        setInt64(strtol(attr.c_str(), nullptr, 10));
}

void WSInteger::load(const Field& field)
{
    if (field.isNull())
        setNull(VAR_INT64);
    else
        setInt64(field.asInt64());
}
