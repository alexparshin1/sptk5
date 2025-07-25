/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CParam.cpp - description                               ║
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

#include <sptk5/db/QueryParameter.h>

using namespace std;
using namespace sptk;

void QueryParameter::bindClear()
{
    m_bindParamIndexes.clear();
}

void QueryParameter::bindAdd(uint32_t bindIndex)
{
    m_bindParamIndexes.push_back(bindIndex);
}

uint32_t QueryParameter::bindCount()
{
    return (uint32_t) m_bindParamIndexes.size();
}

uint32_t QueryParameter::bindIndex(uint32_t ind)
{
    return m_bindParamIndexes[ind];
}

QueryParameter::QueryParameter(const char *name, bool isOutput)
: m_name(lowerCase(name)),
  m_timeData(new char[80]),
  m_callbackLength(0),
  m_paramList(nullptr),
  m_binding(isOutput)
{
}

QueryParameter::~QueryParameter()
{
    delete [] m_timeData;
}

String QueryParameter::name() const
{
    return m_name;
}

void QueryParameter::setOutput()
{
    m_binding.setOutput();
}

QueryParameter& QueryParameter::operator = (const Variant& param)
{
    if (this == &param)
        return *this;
    setData(param);
    return *this;
}

void QueryParameter::reallocateBuffer(const char* value, size_t maxlen, size_t valueLength)
{
    m_dataSize = valueLength;
    m_data.getBuffer().size = valueLength + 1;
    if ((dataType() & (VAR_STRING | VAR_TEXT | VAR_BUFFER)) != 0)
        delete[] m_data.getBuffer().data;
    m_data.getBuffer().data = new char[m_data.getBuffer().size];
    if (maxlen != 0) {
        if (m_data.getBuffer().data != nullptr) {
            strncpy(m_data.getBuffer().data, value, maxlen);
            m_data.getBuffer().data[maxlen] = 0;
        }
    } else {
        m_data.getBuffer().size = m_dataSize + 1;
        strncpy(m_data.getBuffer().data, value, m_data.getBuffer().size);
    }
}

void QueryParameter::setString(const char * value, size_t maxlen)
{
    size_t valueLength;
    uint32_t dtype = VAR_STRING;
    if (maxlen != 0)
        valueLength = (uint32_t) maxlen;
    else {
        if (value != nullptr)
            valueLength = (uint32_t) strlen(value);
        else
            valueLength = 0;
    }

    if (dataType() == VAR_STRING && m_data.getBuffer().size >= valueLength + 1) {
        if (value != nullptr) {
            memcpy(m_data.getBuffer().data, value, valueLength);
            m_data.getBuffer().data[valueLength] = 0;
            m_dataSize = valueLength;
        } else {
            m_data.getBuffer().data[0] = 0;
            dtype |= VAR_NULL;
            m_dataSize = 0;
        }
    } else {
        if (value != nullptr)
            reallocateBuffer(value, maxlen, valueLength);
        else {
            m_dataSize = 0;
            dtype |= VAR_NULL;
        }
    }
    dataType((VariantType) dtype);
}
