/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CParams.cpp  -  description
                             -------------------
    begin                : Tue Dec 14 1999
    copyright            : (C) 1999-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/db/CParams.h>
#include <sptk5/CIntList.h>
#include <sptk5/CException.h>

#include <sptk5/string_ext.h>
#include <ctype.h>

using namespace std;
using namespace sptk;

//---------------------------------------------------------------------------
CParam::CParam(char *name) :
        CVariant()
{
    m_name = lowerCase(name);
    m_timeData = new char[80];
    m_callbackLength = 0;
}
//---------------------------------------------------------------------------
CParam& CParam::operator =(const CParam& param)
{
    setData(param);
    m_bindParamIndexes = param.m_bindParamIndexes;
    return *this;
}
//---------------------------------------------------------------------------
void CParam::bindAdd(uint32_t bindIndex)
{
    m_bindParamIndexes.push_back(bindIndex);
}
//---------------------------------------------------------------------------
string CParam::asXML() const
{
    string xml("<param ");
    xml += "name='" + m_name + "' ";
    xml += "type='" + int2string(m_dataType) + "' ";
    xml += "null='" + int2string((int) isNull()) + "' ";
    if (dataType() & (VAR_STRING | VAR_TEXT)) {
        string txt = replaceAll(replaceAll(m_data.buffer.data, "\n", "\\n"), "\r", "");
        xml += "value='" + txt + "' ";
    } else
        xml += "value='" + asString() + "' ";
    xml += "size='" + int2string((uint32_t) m_dataSize) + "' />";
    return xml;
}
//---------------------------------------------------------------------------
void CParam::setString(const char * value, uint32_t maxlen)
{
    uint32_t valueLength;
    uint32_t dtype = VAR_STRING;
    if (maxlen)
        valueLength = maxlen;
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
                if (m_data.buffer.data)
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
//---------------------------------------------------------------------------
CParamList::~CParamList()
{
    try {
        clear();
    } catch (...) {
    }
}
//---------------------------------------------------------------------------
void CParamList::clear()
{
    for (unsigned i = 0; i < size(); i++) {
        CParam *item = (CParam *) m_items[i];
        delete item;
    }
    m_items.clear();
    m_index.clear();
    m_paramStreamItor = m_items.begin();
}
//---------------------------------------------------------------------------
void CParamList::add(CParam * item)
{
    m_items.push_back(item);
    m_index[item->name()] = item;
    m_paramStreamItor = m_items.begin();
}
//---------------------------------------------------------------------------
CParam* CParamList::find(const char *paramName)
{
    CParamMap::iterator itor = m_index.find(paramName);
    if (itor == m_index.end())
        return 0;
    return itor->second;
}
//---------------------------------------------------------------------------
CParam& CParamList::operator [](const char *paramName) const
{
    CParamMap::const_iterator itor = m_index.find(paramName);
    if (itor == m_index.end())
        throw CException("Invalid parameter name: " + string(paramName), __FILE__, __LINE__);
    return *itor->second;
}
//---------------------------------------------------------------------------
void CParamList::remove(uint32_t i)
{
    CParam *item = m_items[i];
    m_index.erase(item->name().c_str());
    m_items.erase(m_items.begin() + i);
    delete item;
    m_paramStreamItor = m_items.begin();
}
//---------------------------------------------------------------------------
CParam& CParamList::next()
{
    CParam *currentParam = *m_paramStreamItor;
    m_paramStreamItor++;
    if (m_paramStreamItor == m_items.end())
        m_paramStreamItor = m_items.begin();
    return *currentParam;
}
//---------------------------------------------------------------------------
void CParamList::enumerate(CParamVector& params)
{
    CParamVector::iterator ptor;
    CIntList::iterator btor;
    params.resize(m_items.size() * 2);
    if (m_items.empty())
        return;
    uint32_t maxIndex = 0;
    for (ptor = m_items.begin(); ptor != m_items.end(); ptor++) {
        CParam* param = *ptor;
        CIntList& bindIndex = (*ptor)->m_bindParamIndexes;
        for (btor = bindIndex.begin(); btor != bindIndex.end(); btor++) {
            uint32_t index = *btor;
            if (index >= params.size())
                params.resize(index + 1);
            params[index] = param;
            if (index > maxIndex)
                maxIndex = index;
        }
    }
    params.resize(maxIndex + 1);
}
//---------------------------------------------------------------------------
CParamList& sptk::operator <<(CParamList& paramList, const string& data)
{
    paramList.next() = data;
    return paramList;
}

CParamList& sptk::operator <<(CParamList& paramList, const char *data)
{
    paramList.next() = data;
    return paramList;
}

CParamList& sptk::operator <<(CParamList& paramList, int data)
{
    paramList.next() = data;
    return paramList;
}

CParamList& sptk::operator <<(CParamList& paramList, double data)
{
    paramList.next() = data;
    return paramList;
}

CParamList& sptk::operator <<(CParamList& paramList, CDateTime data)
{
    paramList.next() = data;
    return paramList;
}

CParamList& sptk::operator <<(CParamList& paramList, const CBuffer& data)
{
    paramList.next() = data;
    return paramList;
}
