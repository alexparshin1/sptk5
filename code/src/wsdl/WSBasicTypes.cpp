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

#include <sptk5/wsdl/WSBasicTypes.h>

using namespace std;
using namespace sptk;

void WSTypeName::owaspCheck(const String& value)
{
    if (value.find("<script") != string::npos || value.find("</script>") != string::npos)
        throw Exception("Invalid value: constains a script");
}

xml::Element* WSBasicType::addElement(xml::Element* parent, const char* _name) const
{
    String elementName = _name == nullptr? name() : _name;
    String text(isNull()? "": asString());
    if (m_optional && (isNull() || text.empty()))
        return nullptr;
    auto* element = new xml::Element(*parent, elementName);
    element->text(text);
    return element;
}

json::Element* WSBasicType::addElement(json::Element* parent) const
{
    String text(isNull()? "": asString());
    if (m_optional && (isNull() || text.empty()))
        return nullptr;
    json::Element* element;
    if (!parent->is(json::JDT_ARRAY)) {
        if (name().empty())
            return nullptr;
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
    } else {
        switch (dataType()) {
            case VAR_BOOL:
                element = parent->push_back(asBool());
                break;
            case VAR_INT:
                element = parent->push_back(asInteger());
                break;
            case VAR_INT64:
                element = parent->push_back(asInt64());
                break;
            case VAR_FLOAT:
                element = parent->push_back(asFloat());
                break;
            default:
                element = parent->push_back(asString());
                break;
        }
    }

    return element;
}

void WSBasicType::throwIfNull(const String& parentTypeName) const
{
    if (isNull())
        throw SOAPException("Element '" + fieldName() + "' is required in '" + parentTypeName + "'.");
}

void WSString::load(const xml::Node* attr)
{
    owaspCheck(attr->text());
    setString(attr->text());
}

void WSString::load(const json::Element* attr)
{
    if (attr->is(json::JDT_NULL))
        setNull(VAR_STRING);
    else {
        owaspCheck(attr->getString());
        setString(attr->getString());
    }
}

void WSString::load(const String& attr)
{
    owaspCheck(attr);
    setString(attr);
}

void WSString::load(const Field& field)
{
    if (field.isNull())
        setNull(VAR_STRING);
    else {
        owaspCheck(field.asString());
        setString(field.asString());
    }
}

void WSBool::load(const xml::Node* attr)
{
    String text = attr->text();
    if (text.empty())
        setNull(VAR_BOOL);
    else {
        if (text == "true")
            setBool(true);
        else if (text == "false")
            setBool(false);
        else
            throw Exception("Invalid data: not true or false");
    }
}

void WSBool::load(const json::Element* attr)
{
    if (attr->is(json::JDT_NULL))
        setNull(VAR_BOOL);
    else
        setBool(attr->getBoolean());
}

void WSBool::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_BOOL);
    else {
        if (attr == "true")
            setBool(true);
        else if (attr == "false")
            setBool(false);
        else
            throw Exception("Invalid data: not true or false");
    }
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
    if (attr->is(json::JDT_NULL) || text.empty())
        setNull(VAR_DATE);
    else
        setDateTime(DateTime(text.c_str()), true);
}

void WSDate::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_DATE);
    else {
        DateTime dt(attr.c_str());
        setDateTime(dt, true);
    }
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
    else {
        DateTime dt(text.c_str());
        setDateTime(dt);
    }
}

void WSDateTime::load(const json::Element* attr)
{
    String text = attr->getString();
    if (text.empty())
        setNull(VAR_DATE_TIME);
    else {
        DateTime dt(text.c_str());
        setDateTime(dt);
    }
}

void WSDateTime::load(const String& attr)
{
    if (attr.empty())
        setNull(VAR_DATE_TIME);
    else {
        DateTime dt(attr.c_str());
        setDateTime(DateTime(attr.c_str()));
    }
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
    if (attr->is(json::JDT_NULL))
        setNull(VAR_INT64);
    else
        setInt64((int64_t)attr->getNumber());
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

String sptk::wsTypeIdToName(const String& typeIdName)
{
    static const RegularExpression matchClassName("^\\d+C([A-Z]\\S+)$");

    auto matches = matchClassName.m(typeIdName);
    if (matches)
        return matches.groups()[0].value;

    return "Unknown";
}

#if USE_GTEST

TEST(SPTK_WSInteger, move_ctor_assign)
{
    WSInteger   integer1("I1");
    integer1 = 5;
    EXPECT_EQ(integer1.asInteger(), 5);
    EXPECT_EQ(integer1.isNull(), false);

    WSInteger   integer2(move(integer1));
    EXPECT_EQ(integer2.asInteger(), 5);
    EXPECT_EQ(integer2.isNull(), false);
    EXPECT_EQ(integer1.isNull(), true);

    WSInteger   integer3("I3");
    integer3 = move(integer2);
    EXPECT_EQ(integer3.asInteger(), 5);
    EXPECT_EQ(integer3.isNull(), false);
    EXPECT_EQ(integer2.isNull(), true);
}

template <class T>
void loadScriptAttackData()
{
    T  type("type");
    try {
        type.load("Hello, <script>alert(1);</script>");
        if (type.asString().find("<script>") != string::npos)
            FAIL() << type.className() << ": Script attack is accepted";
    }
    catch (const Exception& e) {
        if (String(e.what()).find("<script>") != string::npos)
            FAIL() << type.className() << ": Script attack is reflected back";
    }
}

TEST(SPTK_WSBasicTypes, array)
{
    WSArray<WSInteger> array;
    array.emplace_back(1);
    array.emplace_back(2);
    array.emplace_back(3);
    EXPECT_EQ(array.size(), size_t(3));

    WSArray<WSInteger> array2(array);
    EXPECT_EQ(array2.size(), size_t(3));
    EXPECT_EQ(array2[1].asInteger(), 2);

    WSArray<WSInteger> array3;
    array3 = array;
    EXPECT_EQ(array3.size(), size_t(3));
    EXPECT_EQ(array3[1].asInteger(), 2);

    WSArray<WSInteger> array4;
    array4 = move(array);
    EXPECT_EQ(array4.size(), size_t(3));
    EXPECT_EQ(array4[1].asInteger(), 2);
    EXPECT_TRUE(array.empty());
}

TEST(SPTK_WSBasicTypes, scriptAttack)
{
    loadScriptAttackData<WSDate>();
    loadScriptAttackData<WSBool>();
    loadScriptAttackData<WSDateTime>();
    loadScriptAttackData<WSDouble>();
    loadScriptAttackData<WSInteger>();
    loadScriptAttackData<WSString>();
}

#endif
