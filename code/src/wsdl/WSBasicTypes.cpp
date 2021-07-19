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

#include <sptk5/wsdl/WSBasicTypes.h>
#include <sptk5/wsdl/WSArray.h>
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

void WSBasicType::exportTo(const SNode& parent, const char* _name) const
{
    String elementName = _name == nullptr ? name() : _name;
    if (String text(isNull() ? "" : asString());
        m_optional && (isNull() || text.empty()))
    {
        return;
    }

    if (!parent->is(Node::Type::Array))
    {
        if (name().empty())
        {
            return;
        }
        switch (dataType())
        {
            case VariantDataType::VAR_BOOL:
                parent->set(name(), m_field.asBool());
                break;
            case VariantDataType::VAR_INT:
                parent->set(name(), m_field.asInteger());
                break;
            case VariantDataType::VAR_INT64:
                parent->set(name(), m_field.asInt64());
                break;
            case VariantDataType::VAR_FLOAT:
                parent->set(name(), m_field.asFloat());
                break;
            default:
                parent->set(name(), asString());
                break;
        }
    }
    else
    {
        switch (dataType())
        {
            case VariantDataType::VAR_BOOL:
                parent->pushValue(m_field.asBool());
                break;
            case VariantDataType::VAR_INT:
                parent->pushValue(m_field.asInteger());
                break;
            case VariantDataType::VAR_INT64:
                parent->pushValue(m_field.asInt64());
                break;
            case VariantDataType::VAR_FLOAT:
                parent->pushValue(m_field.asFloat());
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
        throw SOAPException("Element '" + m_field.fieldName() + "' is required in '" + parentTypeName + "'.");
    }
}

void WSString::load(const SNode& attr)
{
    if (attr->is(Node::Type::Null))
    {
        setNull(VariantDataType::VAR_STRING);
    }
    else
    {
        auto str = attr->getString();
        owaspCheck(str);
        field().setString(str);
    }
}

void WSString::load(const String& attr)
{
    owaspCheck(attr);
    field().setString(attr);
}

void WSString::load(const Field& field)
{
    load(field.asString());
}

void WSBool::load(const SNode& attr)
{
    if (attr->is(Node::Type::Null))
    {
        setNull(VariantDataType::VAR_BOOL);
    }
    else
    {
        field().setBool(attr->getBoolean());
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
            field().setBool(true);
        }
        else if (attr == "false")
        {
            field().setBool(false);
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
        this->field().setBool(field.asBool());
    }
}

void WSDate::load(const SNode& attr)
{
    String text = attr->getString();
    if (attr->is(Node::Type::Null) || text.empty())
    {
        setNull(VariantDataType::VAR_DATE);
    }
    else
    {
        field().setDateTime(DateTime(text.c_str()), true);
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
        DateTime dt(attr.c_str());
        field().setDateTime(dt, true);
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
        this->field().setDateTime(field.asDate(), true);
    }
}

void WSDateTime::load(const SNode& attr)
{
    String text = attr->getText();
    if (text.empty())
    {
        setNull(VariantDataType::VAR_DATE_TIME);
    }
    else
    {
        DateTime dt(text.c_str());
        field().setDateTime(dt);
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
        DateTime dt(attr.c_str());
        field().setDateTime(DateTime(attr.c_str()));
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
        this->field().setDateTime(field.asDateTime());
    }
}

String WSDateTime::asString() const
{
    DateTime dt = field().asDateTime();
    return dt.isoDateTimeString();
}

void WSDouble::load(const SNode& attr)
{
    field().setFloat(attr->getNumber());
}

void WSDouble::load(const String& attr)
{
    if (attr.empty())
    {
        setNull(VariantDataType::VAR_INT);
    }
    else
    {
        field().setFloat(strtod(attr.c_str(), nullptr));
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
        this->field().setFloat(field.asFloat());
    }
}

void WSInteger::load(const SNode& attr)
{
    if (attr->is(Node::Type::Null))
    {
        setNull(VariantDataType::VAR_INT64);
    }
    else
    {
        field().setInt64((int64_t) attr->getNumber());
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
        field().setInt64(strtol(attr.c_str(), nullptr, 10));
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
        this->field().setInt64(field.asInt64());
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

#ifdef USE_GTEST

TEST(SPTK_WSInteger, move_ctor_assign)
{
    WSInteger integer1("I1", false);
    integer1 = 5;
    EXPECT_EQ(integer1.asInteger(), 5);
    EXPECT_EQ(integer1.isNull(), false);

    WSInteger integer2(move(integer1));
    EXPECT_EQ(integer2.asInteger(), 5);

    WSInteger integer3("I3", false);
    integer3 = move(integer2);
    EXPECT_EQ(integer3.asInteger(), 5);
}

template<class T>
void loadScriptAttackData()
{
    T type("type", false);
    try
    {
        type.load("Hello, <script>alert(1);</script>");
        if (type.asString().find("<script>") != string::npos)
            FAIL() << type.className() << ": Script attack is accepted";
    }
    catch (const Exception& e)
    {
        if (String(e.what()).find("<script>") != string::npos)
            FAIL() << type.className() << ": Script attack is reflected back";
    }
}

TEST(SPTK_WSBasicTypes, array)
{
    WSArray<WSInteger> array("");
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
