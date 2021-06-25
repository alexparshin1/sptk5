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

#include <sptk5/db/QueryParameter.h>

using namespace std;
using namespace sptk;

void QueryParameter::bindAdd(uint32_t bindIndex)
{
    m_bindParamIndexes.push_back(bindIndex);
}

uint32_t QueryParameter::bindCount() const
{
    return (uint32_t) m_bindParamIndexes.size();
}

uint32_t QueryParameter::bindIndex(uint32_t ind)
{
    return m_bindParamIndexes[ind];
}

QueryParameter::QueryParameter(const char* name, bool isOutput)
    : m_binding(isOutput), m_name(lowerCase(name))
{
}

String QueryParameter::name() const
{
    return m_name;
}

void QueryParameter::setOutput()
{
    m_binding.setOutput();
}

QueryParameter& QueryParameter::operator=(const Variant& param)
{
    if (this != &param)
    {
        setData(param);
    }
    return *this;
}

void QueryParameter::reallocateBuffer(const char* value, size_t maxlen, size_t valueLength)
{
    m_data.size(maxlen > 0 ? min(valueLength, maxlen) : valueLength);
    m_data.getBuffer().size = m_data.size() + 1;
    if ((dataType() & (VAR_STRING | VAR_TEXT | VAR_BUFFER)) != 0)
    {
        delete[] m_data.getBuffer().data;
    }
    auto* data = new char[m_data.size() + 1];
    m_data.getBuffer().data = data;
    if (value == nullptr)
    {
        memset(data, 0, m_data.size());
    }
    else
    {
        memcpy(data, value, m_data.size());
    }
    data[m_data.size()] = 0;
}

void QueryParameter::setString(const char* value, size_t maxlen)
{
    size_t valueLength;
    uint32_t dtype = VAR_STRING;
    if (maxlen != 0)
    {
        valueLength = (uint32_t) maxlen;
    }
    else
    {
        if (value != nullptr)
        {
            valueLength = (uint32_t) strlen(value);
        }
        else
        {
            valueLength = 0;
        }
    }

    if (dataType() == VAR_STRING && m_data.getBuffer().size >= valueLength + 1)
    {
        if (value != nullptr)
        {
            memcpy(m_data.getBuffer().data, value, valueLength);
            m_data.getBuffer().data[valueLength] = 0;
            m_data.size(valueLength);
        }
        else
        {
            m_data.getBuffer().data[0] = 0;
            dtype |= VAR_NULL;
            m_data.size(0);
        }
    }
    else
    {
        reallocateBuffer(value, maxlen, valueLength);
        if (value == nullptr)
        {
            m_data.size(0);
            dtype |= VAR_NULL;
        }
    }
    dataType((VariantType) dtype);
}

#if USE_GTEST

TEST(SPTK_QueryParameter, minimal)
{
    QueryParameter param1("param1");

    EXPECT_STREQ(param1.name().c_str(), "param1");
}

TEST(SPTK_QueryParameter, setString)
{
    QueryParameter param1("param1");

    param1.setString("String 1");
    EXPECT_STREQ(param1.getString(), "String 1");

    param1.setString("String 1", 3);
    EXPECT_STREQ(param1.getString(), "Str");

    param1.setString("String 1 + String 2");
    EXPECT_STREQ(param1.getString(), "String 1 + String 2");

    param1.setString("String 1");
    EXPECT_STREQ(param1.getString(), "String 1");

    param1.setString("String 1 + String 2 + String 3", 22);
    EXPECT_STREQ(param1.getString(), "String 1 + String 2 + ");

    param1.setString(nullptr);
    EXPECT_TRUE(param1.isNull());
}

TEST(SPTK_QueryParameter, assign)
{
    QueryParameter param1("param1");

    param1 = "String 1";
    EXPECT_STREQ(param1.getString(), "String 1");

    param1 = "String 1, String 2";
    EXPECT_STREQ(param1.getString(), "String 1, String 2");

    param1 = 123;
    EXPECT_EQ(param1.getInteger(), 123);

    param1 = 123.0;
    EXPECT_FLOAT_EQ(param1.getFloat(), 123.0);

    param1.setString(nullptr);
    EXPECT_TRUE(param1.isNull());

    DateTime dt("2020-03-01 10:11:12");
    Variant v1(dt);
    param1 = v1;
    EXPECT_TRUE(param1.asDateTime() == dt);
}

#endif
