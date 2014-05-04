/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CParam.cpp  -  description
                             -------------------
    begin                : 14 Jul 2013
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/db/CParam.h>
#include <stdint.h>

using namespace std;
using namespace sptk;

void CParam::bindClear()
{
    m_bindParamIndexes.clear();
}

void CParam::bindAdd(uint32_t bindIndex)
{
    m_bindParamIndexes.push_back(bindIndex);
}

uint32_t CParam::bindCount()
{
    return (uint32_t) m_bindParamIndexes.size();
}

uint32_t CParam::bindIndex(uint32_t ind)
{
    return m_bindParamIndexes[ind];
}

CParam::CParam(const char *name, bool isOutput) :
    CVariant(),
    m_name(lowerCase(name)),
    m_timeData(new char[80]),
    m_callbackLength(0),
    m_paramList(NULL),
    m_binding(isOutput)
{
}

CParam::~CParam()
{
    delete [] m_timeData;
}

string CParam::name() const
{
    return m_name;
}

CParam& CParam::operator = (const CVariant& param)
{
    if (this == &param)
        return *this;
    setData(param);
    return *this;
}

void CParam::setString(const char * value, size_t maxlen)
{
    uint32_t valueLength;
    uint32_t dtype = VAR_STRING;
    if (maxlen)
        valueLength = (uint32_t) maxlen;
    else
        valueLength = (uint32_t) strlen(value);
    if (m_dataType == VAR_STRING && m_data.buffer.size >= valueLength + 1) {
        if (value) {
            memcpy(m_data.buffer.data, value, valueLength);
            m_data.buffer.data[valueLength] = 0;
            m_dataSize = valueLength;
        } else {
            m_data.buffer.data[0] = 0;
            dtype |= VAR_NULL;
            m_dataSize = 0;
        }
    } else {
        if (value) {
            m_dataSize = valueLength;
            m_data.buffer.size = valueLength + 1;
            if (maxlen) {
                m_data.buffer.data = (char *) realloc(m_data.buffer.data, m_data.buffer.size);
                strncpy(m_data.buffer.data, value, maxlen);
                m_data.buffer.data[maxlen] = 0;
            } else {
                if (m_dataType & (VAR_STRING | VAR_TEXT | VAR_BUFFER) && m_data.buffer.data)
                    free(m_data.buffer.data);
                m_data.buffer.size = m_dataSize + 1;
                m_data.buffer.data = strdup(value);
            }
        } else {
            m_dataSize = 0;
            dtype |= VAR_NULL;
        }
    }
    dataType((CVariantType) dtype);
}
