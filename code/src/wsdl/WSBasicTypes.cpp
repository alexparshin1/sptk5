/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/xdoc/Document.h>

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;
using namespace xdoc;

static const RegularExpression StringIsInteger(R"(^[+\-]?(0|[1-9]\d*)$)");
static const RegularExpression StringIsFloat(R"(^[+\-]?(0|[1-9]\d*)(\.\d*)?(e[+\-]\d+)?$)");

void WSBasicType::exportTo(const SNode& parent, const char* _name) const
{
    String elementName = _name == nullptr ? name() : _name;
    if (String text(isNull() ? "" : asString());
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
            case VariantDataType::VAR_BOOL:
                parent->set(elementName, m_value.asBool());
                break;
            case VariantDataType::VAR_INT:
                parent->set(elementName, m_value.asInteger());
                break;
            case VariantDataType::VAR_INT64:
                parent->set(elementName, m_value.asInt64());
                break;
            case VariantDataType::VAR_FLOAT:
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
            case VariantDataType::VAR_BOOL:
                parent->pushValue(m_value.asBool());
                break;
            case VariantDataType::VAR_INT:
                parent->pushValue(m_value.asInteger());
                break;
            case VariantDataType::VAR_INT64:
                parent->pushValue(m_value.asInt64());
                break;
            case VariantDataType::VAR_FLOAT:
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
    String text = attr->getString();
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
        DateTime dt(attr.c_str());
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
    String text = attr->getText();
    if (text.empty())
    {
        setNull(VariantDataType::VAR_DATE_TIME);
    }
    else
    {
        DateTime dt(text.c_str());
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
        DateTime dt(attr.c_str());
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
    DateTime dt = value().asDateTime();
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
            String textValue = attr->getText();
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

#ifdef USE_GTEST

TEST(SPTK_WSInteger, move_ctor_assign)
{
    constexpr int testIntegerValue = 5;
    WSInteger integer1("I1", false);
    integer1 = testIntegerValue;
    EXPECT_EQ(integer1.asInteger(), testIntegerValue);
    EXPECT_EQ(integer1.isNull(), false);

    WSInteger integer2(move(integer1));
    EXPECT_EQ(integer2.asInteger(), testIntegerValue);

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

TEST(SPTK_WSBasicTypes, defaultType)
{
    WSInteger field1("field1", true);
    EXPECT_EQ(field1.dataType(), VariantDataType::VAR_INT);
}

TEST(SPTK_WSBasicTypes, fieldNames)
{
    WSInteger field1("field1", true);
    WSInteger field2("field2", true);

    field2 = 2;
    field1 = field2;

    EXPECT_EQ(field1.name(), "field1");
    EXPECT_EQ(field2.name(), "field2");
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
}

TEST(SPTK_WSBasicTypes, arrayName)
{
    WSArray<WSInteger> array("array1");
    array.emplace_back(1);
    array.emplace_back(2);
    array.emplace_back(3);
    EXPECT_EQ(array.size(), size_t(3));

    WSArray<WSInteger> array2("array2");

    array2 = array;

    EXPECT_EQ(array2.size(), size_t(3));
    EXPECT_STREQ(array2.name().c_str(), "array2");
}

TEST(SPTK_WSBasicTypes, scriptAttack)
{
    loadScriptAttackData<WSDate>();
    loadScriptAttackData<WSBool>();
    loadScriptAttackData<WSDateTime>();
    loadScriptAttackData<WSDouble>();
    loadScriptAttackData<WSInteger>();
    loadScriptAttackData<WSString>();

    WSString string;
    EXPECT_THROW(string.load("<script>Hello()</script>"), Exception);
    EXPECT_NO_THROW(string.load("<Hello></Hello"));

    Field field("fname");
    field.setString("<script>Hello()</script>");
    EXPECT_THROW(string.load(field), Exception);
    field.setString("<Hello></Hello");
    EXPECT_NO_THROW(string.load(field));
}

TEST(SPTK_WSBasicTypes, loadBoolean)
{
    xdoc::Document document;
    const auto& root = document.root();
    root->set("true", true);
    root->set("false", false);
    root->findOrCreate("null");

    WSBool boolean;
    boolean.load(root->findFirst("true"), true);
    EXPECT_EQ(boolean.asBool(), true);

    boolean.load("true");
    EXPECT_EQ(boolean.asBool(), true);

    boolean.load(root->findFirst("false"), true);
    EXPECT_EQ(boolean.asBool(), false);

    boolean.load("false");
    EXPECT_EQ(boolean.asBool(), false);

    boolean.load(root->findFirst("null"), true);
    EXPECT_TRUE(boolean.isNull());

    boolean.load("");
    EXPECT_TRUE(boolean.isNull());

    Field field("boolean");

    boolean.load(field);
    EXPECT_TRUE(boolean.isNull());

    field.setBool(true);
    boolean.load(field);
    EXPECT_EQ(boolean.asBool(), true);

    field.setBool(false);
    boolean.load(field);
    EXPECT_EQ(boolean.asBool(), false);
}

TEST(SPTK_WSBasicTypes, loadInteger)
{
    constexpr int testIntegerValue = 1234567;

    xdoc::Document document;
    const auto& root = document.root();
    root->findOrCreate("integer");
    root->findOrCreate("null");

    WSInteger integer;

    integer.load(root->findFirst("integer"), true);
    EXPECT_TRUE(integer.isNull());

    root->set("integer", testIntegerValue);
    integer.load(root->findFirst("integer"), true);
    EXPECT_EQ(integer.asInteger(), testIntegerValue);

    Field field("integer");

    integer.load(field);
    EXPECT_TRUE(integer.isNull());

    field.setInteger(testIntegerValue);
    integer.load(field);
    EXPECT_EQ(integer.asInteger(), testIntegerValue);

    auto textNode = root->set("text", "xxx");
    EXPECT_THROW(integer.load(textNode, false), Exception);
}

TEST(SPTK_WSBasicTypes, loadDateTime)
{
    xdoc::Document document;
    const auto& root = document.root();
    root->set("datetime", DateTime("2021-01-02T11:12:13Z"));
    root->findOrCreate("null");

    WSDate date;

    date.load(root->findFirst("null"), true);
    EXPECT_TRUE(date.isNull());

    date.load(root->findFirst("datetime"), true);
    EXPECT_STREQ(date.asDateTime().dateString().c_str(), DateTime("2021-01-02").dateString().c_str());

    WSDateTime datetime;

    datetime.load(root->findFirst("null"), true);
    EXPECT_TRUE(datetime.isNull());

    datetime.load(root->findFirst("datetime"), true);
    EXPECT_STREQ(datetime.asDateTime().isoDateTimeString().c_str(), DateTime("2021-01-02T11:12:13Z").isoDateTimeString().c_str());
}

TEST(SPTK_WSBasicTypes, loadDouble)
{
    constexpr double testDoubleValue = 1234.567;
    constexpr int testIntValue = 1234;
    const String testIntegerValueStr("1234");
    const String testDoubleValueStr("1234.567");

    xdoc::Document document;
    const auto& root = document.root();
    root->set("double", testDoubleValue);
    root->findOrCreate("null");

    WSDouble wsDouble;

    wsDouble.load(root->findFirst("null"), true);
    EXPECT_TRUE(wsDouble.isNull());

    wsDouble.load(root->findFirst("double"), true);
    EXPECT_EQ(wsDouble.asFloat(), testDoubleValue);

    auto textNode = root->set("text", "xxx");
    EXPECT_THROW(wsDouble.load(textNode, false), Exception);

    auto textNode2 = root->set("integerText", testIntegerValueStr);
    wsDouble.load(textNode2, false);
    EXPECT_EQ(wsDouble.asFloat(), double(testIntValue));

    auto textNode3 = root->set("doubleText", testDoubleValueStr);
    wsDouble.load(textNode3, false);
    EXPECT_EQ(wsDouble.asFloat(), testDoubleValue);
}

TEST(SPTK_WSBasicTypes, loadValue)
{
    xdoc::Document document;
    const auto& root = document.root();
    root->set("string", "Hello, World!");
    root->findOrCreate("null");

    WSString wsString;
    wsString.load(root->findFirst("string"), true);
    EXPECT_STREQ(wsString.asString().c_str(), "Hello, World!");

    std::string largeData(300, 'x');
    root->set("string", largeData);
    wsString.load(root->findFirst("string"), true);
    EXPECT_TRUE(wsString.isNull());
}

TEST(SPTK_WSBasicTypes, throws)
{
    WSDate date;
    EXPECT_THROW(date.throwIfNull("date"), Exception);

    date = DateTime::Now();
    EXPECT_NO_THROW(date.throwIfNull("date"));
}

TEST(SPTK_WSBasicTypes, exportValue)
{
    constexpr int testIntegerValue = 1234567;
    constexpr int64_t testIntegerValue64 = 1234567890123456;
    constexpr double testDoubleValue = 1234.567;

    xdoc::Document document;
    const auto& root = document.root();

    // Optional null value shouldn't export
    WSInteger wsInt("integer", true);
    wsInt.exportTo(root);
    EXPECT_TRUE(root->findFirst("integer") == nullptr);

    WSDate date;

    date = DateTime("2021-01-02 10:00:00+10");

    // WS variable doesn't have a filed name, and export doesn't provide one
    date.exportTo(root);
    EXPECT_STREQ(root->getString("date").c_str(), "");

    date.exportTo(root, "date");
    EXPECT_STREQ(root->getString("date").c_str(), "2021-01-02");

    WSDateTime datetime;
    datetime = DateTime("2021-01-02 10:00:00");
    datetime.exportTo(root, "datetime");
    EXPECT_STREQ(root->getString("datetime").substr(0, 19).c_str(), "2021-01-02T10:00:00");

    WSBool boolean;
    boolean = true;
    boolean.exportTo(root, "boolean");
    EXPECT_EQ(root->getBoolean("boolean"), true);

    boolean = false;
    boolean.exportTo(root, "boolean");
    EXPECT_EQ(root->getBoolean("boolean"), false);

    WSInteger integer;
    integer = testIntegerValue;
    integer.exportTo(root, "integer");
    EXPECT_DOUBLE_EQ(root->getNumber("integer"), double(testIntegerValue));

    integer = testIntegerValue64;
    integer.exportTo(root, "integer");
    EXPECT_DOUBLE_EQ(root->getNumber("integer"), double(testIntegerValue64));

    WSDouble wsDouble;
    wsDouble = testDoubleValue;
    wsDouble.exportTo(root, "double");
    EXPECT_DOUBLE_EQ(root->getNumber("double"), testDoubleValue);

    WSString string;
    string = "Hello, World!";
    string.exportTo(root, "string");
    EXPECT_STREQ(root->getString("string").c_str(), "Hello, World!");
}

TEST(SPTK_WSBasicTypes, exportValueToArray)
{
    constexpr int testIntegerValue = 1234567;
    constexpr int64_t testIntegerValue64 = 1234567890123456;
    constexpr double testDoubleValue = 1234.567;

    xdoc::Document document;
    const auto& root = document.root();

    const auto& array = root->pushValue("array", Node::Type::Array);

    WSDate date;
    date = DateTime("2021-01-02 10:00:00+10");
    date.exportTo(array);
    EXPECT_STREQ(array->nodes()[0]->getString().c_str(), "2021-01-02");

    WSDateTime datetime;
    datetime = DateTime("2021-01-02 10:00:00");
    datetime.exportTo(array);
    EXPECT_STREQ(array->nodes()[1]->getString().substr(0, 19).c_str(), "2021-01-02T10:00:00");

    WSBool boolean;
    boolean = true;
    boolean.exportTo(array);
    EXPECT_STREQ(array->nodes()[2]->getString().c_str(), "true");

    boolean = false;
    boolean.exportTo(array);
    EXPECT_STREQ(array->nodes()[3]->getString().c_str(), "false");

    WSInteger integer;
    integer = testIntegerValue;
    integer.exportTo(array);
    EXPECT_DOUBLE_EQ(array->nodes()[4]->getNumber(), double(testIntegerValue));

    integer = testIntegerValue64;
    integer.exportTo(array);
    EXPECT_DOUBLE_EQ(array->nodes()[5]->getNumber(), double(testIntegerValue64));

    WSDouble wsDouble;
    wsDouble = testDoubleValue;
    wsDouble.exportTo(array);
    EXPECT_DOUBLE_EQ(array->nodes()[6]->getNumber(), testDoubleValue);

    WSString string;
    string = "Hello, World!";
    string.exportTo(array);
    EXPECT_STREQ(array->nodes()[7]->getString().c_str(), "Hello, World!");
}

#endif
