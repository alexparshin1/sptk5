/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CVariant.cpp  -  description
                             -------------------
    begin                : Tue Dec 14 1999
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

#include <sptk5/CVariant.h>
#include <sptk5/CField.h>
#include <sptk5/CException.h>
#include <sptk5/string_ext.h>

#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;


int64_t CMoneyData::dividers[16] = {
    1, 10, 100, 1000,
    10000, 100000, 1000000L, 10000000L,
    100000000L, 1000000000L, 10000000000L, 100000000000L,
    1000000000000L, 10000000000000L, 100000000000000L, 1000000000000000L
};

CMoneyData::operator double () const
{
    return double(quantity) / dividers[scale];
}

CMoneyData::operator int64_t () const
{
    return quantity / dividers[scale];
}

CMoneyData::operator int32_t () const
{
    return int32_t(quantity / dividers[scale]);
}
//---------------------------------------------------------------------------
void CVariant::releaseBuffers() {
    if (m_dataType & (VAR_STRING|VAR_BUFFER|VAR_TEXT)) {
        if (m_data.buffer.data) {
            if (!(m_dataType & VAR_EXTERNAL_BUFFER))
                free(m_data.buffer.data);
            m_data.buffer.data = NULL;
            m_data.buffer.size = 0;
        }
    }
}
//---------------------------------------------------------------------------
void CVariant::setBool(bool value) {
    if (m_dataType != VAR_BOOL) {
        releaseBuffers();
        m_dataType = VAR_BOOL;
        m_dataSize = sizeof(value);
    }
    m_data.boolData = value;
}
//---------------------------------------------------------------------------
void CVariant::setInteger(int32_t value) {
    if (m_dataType != VAR_INT) {
        releaseBuffers();
        m_dataType = VAR_INT;
        m_dataSize = sizeof(value);
    }
    m_data.intData = value;
}
//---------------------------------------------------------------------------
void CVariant::setInt64(int64_t value) {
    if (m_dataType != VAR_INT64) {
        releaseBuffers();
        m_dataType = VAR_INT64;
        m_dataSize = sizeof(value);
    }
    m_data.int64Data = value;
}
//---------------------------------------------------------------------------
void CVariant::setFloat(double value) {
    if (m_dataType != VAR_FLOAT) {
        releaseBuffers();
        m_dataType = VAR_FLOAT;
        m_dataSize = sizeof(value);
    }
    m_data.floatData = value;
}
//---------------------------------------------------------------------------
void CVariant::setMoney(int64_t value, unsigned scale) {
    if (m_dataType != VAR_MONEY) {
        releaseBuffers();
        m_dataType = VAR_MONEY;
        m_dataSize = sizeof(CMoneyData);
    }
    m_data.moneyData.quantity = value;
    m_data.moneyData.scale = scale;
}
//---------------------------------------------------------------------------
void CVariant::setString(const char * value,size_t maxlen) {
    uint32_t dtype = VAR_STRING;
    if (dataType() == VAR_STRING && maxlen && m_data.buffer.size == maxlen+1) {
        if (value) {
            strncpy(m_data.buffer.data,value,maxlen);
            m_data.buffer.data[maxlen] = 0;
        } else {
            m_data.buffer.data[0] = 0;
            dtype |= VAR_NULL;
        }
    } else {
        releaseBuffers();
        if (value) {
            if (maxlen) {
                dataSize(maxlen);
                m_data.buffer.size = maxlen + 1;
                m_data.buffer.data = (char *)malloc(m_data.buffer.size);
                strncpy(m_data.buffer.data,value,maxlen);
                m_data.buffer.data[maxlen] = 0;
            } else {
                dataSize(strlen(value));
                m_data.buffer.size = dataSize() + 1;
                m_data.buffer.data = strdup(value);
            }
        } else {
            m_data.buffer.data = NULL;
            dataSize(0);
            m_data.buffer.size = 0;
            dtype |= VAR_NULL;
        }
    }
    dataType(dtype);
}
//---------------------------------------------------------------------------
void CVariant::setExternalString(const char * value, int length) {
    uint32_t dtype = VAR_STRING;

    if (!(m_dataType & VAR_EXTERNAL_BUFFER))
        releaseBuffers();

    if (value) {
        if (length < 0)
            length = (int) strlen(value);
        m_dataSize = length;
        m_data.buffer.size = length + 1;
        dtype |= VAR_EXTERNAL_BUFFER;
    } else {
        m_dataSize = 0;
        m_data.buffer.size = 0;
        dtype |= VAR_NULL;
    }
    m_data.buffer.data = (char*) value;
    m_dataType = dtype;
}
//---------------------------------------------------------------------------
void CVariant::setText(const char * value) {
    releaseBuffers();
    dataType(VAR_TEXT);
    if (value) {
        dataSize(strlen(value));
        m_data.buffer.size = dataSize() + 1;
        m_data.buffer.data = strdup(value);
    } else {
        m_dataType |= VAR_NULL;
        m_data.buffer.data = NULL;
        m_data.buffer.size = 0;
        dataSize(0);
    }
}
//---------------------------------------------------------------------------
void CVariant::setText(const string& value) {
    releaseBuffers();
    dataType(VAR_TEXT);
    size_t vlen = value.length();
    if (vlen) {
        dataSize(vlen);
        m_data.buffer.size = vlen + 1;
        m_data.buffer.data = strdup(value.c_str());
    } else {
        m_dataType |= VAR_NULL;
        m_data.buffer.data = NULL;
        m_data.buffer.size = 0;
        dataSize(0);
    }
}
//---------------------------------------------------------------------------
void CVariant::setExternalText(const char * value) {
    releaseBuffers();
    dataType(VAR_TEXT);
    if (value) {
        dataSize(strlen(value));
        m_data.buffer.size = dataSize() + 1;
        m_data.buffer.data = (char*) value;
        m_dataType |= VAR_EXTERNAL_BUFFER;
    } else {
        m_dataType |= VAR_NULL;
        m_data.buffer.data = NULL;
        m_data.buffer.size = 0;
        dataSize(0);
    }
}
//---------------------------------------------------------------------------
void CVariant::setBuffer(const void * value, size_t sz) {
    releaseBuffers();
    dataType(VAR_BUFFER);
    if (value || sz) {
        m_data.buffer.size = sz;
        dataSize(sz);
        m_data.buffer.data = (char *)malloc(sz);
        if (value)
            memcpy(m_data.buffer.data,value,sz);
    } else
        setNull();
}
//---------------------------------------------------------------------------
void CVariant::setBuffer(const string& value) {
    releaseBuffers();
    dataType(VAR_BUFFER);
    size_t vlen = value.length();
    if (vlen) {
        size_t sz = vlen + 1;
        m_data.buffer.size = sz;
        dataSize(sz);
        m_data.buffer.data = (char *)malloc(sz);
        memcpy(m_data.buffer.data,value.c_str(),sz);
    } else
        setNull();
}
//---------------------------------------------------------------------------
void CVariant::setExternalBuffer(const void * value, size_t sz) {
    releaseBuffers();
    dataType(VAR_BUFFER);
    m_data.buffer.data = (char*) value;
    if (value) {
        m_data.buffer.size = sz;
        dataSize(sz);
        m_dataType |= VAR_EXTERNAL_BUFFER;
    } else {
        dataSize(0);
        m_dataType |= VAR_NULL;
    }
}
//---------------------------------------------------------------------------
void CVariant::setDateTime(CDateTime value) {
    if (m_dataType != VAR_DATE_TIME) {
        releaseBuffers();
        dataType(VAR_DATE_TIME);
        dataSize(sizeof(value));
    }
    m_data.floatData = value;
}
//---------------------------------------------------------------------------
void CVariant::setDate(CDateTime value) {
    if (m_dataType != VAR_DATE) {
        releaseBuffers();
        dataType(VAR_DATE);
        dataSize(sizeof(value));
    }
    m_data.floatData = value;
}
//---------------------------------------------------------------------------
void CVariant::setImagePtr(const void *value) {
    if (m_dataType != VAR_IMAGE_PTR) {
        releaseBuffers();
        dataType(VAR_IMAGE_PTR);
        dataSize(sizeof(value));
    }
    m_data.imagePtr = (void *) value;
}
//---------------------------------------------------------------------------
void CVariant::setImageNdx(uint32_t value) {
    if (dataType() != VAR_IMAGE_NDX) {
        releaseBuffers();
        dataType(VAR_IMAGE_NDX);
    }
    dataSize(sizeof(value));
    m_data.imageNdx = value;
}
//---------------------------------------------------------------------------
void CVariant::setData(const CVariant &C) {
    switch (C.dataType()) {
    case VAR_BOOL:
        setInteger(C.getBool());
        break;
    case VAR_INT:
        setInteger(C.getInteger());
        break;
    case VAR_INT64:
        setInt64(C.getInt64());
        break;
    case VAR_FLOAT:
        setFloat(C.getFloat());
        break;
    case VAR_MONEY:
        setMoney(C.m_data.moneyData.quantity, C.m_data.moneyData.scale);
        break;
    case VAR_STRING:
        setString(C.getString(),C.dataSize());
        break;
    case VAR_TEXT:
        setText(C.getText());
        break;
    case VAR_BUFFER:
        setBuffer(C.getBuffer(),C.dataSize());
        break;
    case VAR_DATE:
        setDate(C.getDateTime());
        break;
    case VAR_DATE_TIME:
        setDateTime(C.getDateTime());
        break;
    case VAR_IMAGE_PTR:
        setImagePtr(C.getImagePtr());
        break;
    case VAR_IMAGE_NDX:
        setImageNdx(C.getImageNdx());
        break;
    case VAR_NONE:
        break;
    }
    m_dataType = C.m_dataType;
}
//---------------------------------------------------------------------------
// convertors
int32_t CVariant::asInteger() const throw(CException) {
    if (m_dataType & VAR_NULL)
        return 0;
    switch (dataType()) {
    case VAR_BOOL:
        return m_data.boolData;
    case VAR_INT:
        return m_data.intData;
    case VAR_INT64:
        return (int32_t) m_data.int64Data;
    case VAR_MONEY:
        return (int32_t) m_data.moneyData;
    case VAR_FLOAT:
        return (int32_t) m_data.floatData;
    case VAR_STRING:
    case VAR_TEXT:
    case VAR_BUFFER:
        return atol(m_data.buffer.data);
    case VAR_DATE:
    case VAR_DATE_TIME:
        return (long) m_data.floatData;
    default:
        throw CException("Can't convert field for that type");
    }
}

int64_t CVariant::asInt64() const throw(CException) {
    if (m_dataType & VAR_NULL)
        return true;
    switch (dataType()) {
        case VAR_BOOL:
            return m_data.boolData;
        case VAR_INT:
            return m_data.intData;
        case VAR_INT64:
            return m_data.int64Data;
        case VAR_MONEY:
            return (int64_t)m_data.moneyData;
        case VAR_FLOAT:
            return (int64_t)m_data.floatData;
        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
#ifdef _MSC_VER
            return _atoi64(m_data.buffer.data);
#else
            return atol(m_data.buffer.data);
#endif
        case VAR_DATE:
        case VAR_DATE_TIME:
            return int64_t(m_data.floatData);
	case VAR_IMAGE_PTR:
            return int64_t(m_data.imagePtr);
	case VAR_IMAGE_NDX:
            return int64_t(m_data.imageNdx);
        default:
            throw CException("Can't convert field for that type");
    }
}

bool CVariant::asBool() const throw(CException) {
    char ch;
    if (m_dataType & VAR_NULL)
        return true;
    switch (dataType()) {
    case VAR_BOOL:
        return m_data.boolData;
    case VAR_INT:
        return (m_data.intData>0);
    case VAR_INT64:
        return (m_data.int64Data>0);
    case VAR_MONEY:
        return (m_data.moneyData.quantity>0);
    case VAR_FLOAT:
        return (m_data.floatData>0);
    case VAR_STRING:
    case VAR_TEXT:
    case VAR_BUFFER:
        ch = m_data.buffer.data[0];
        return (strchr("YyTt1",ch)!=0);
    case VAR_DATE:
    case VAR_DATE_TIME:
        return bool(m_data.floatData!=0);
    case VAR_IMAGE_PTR:
        return bool(m_data.imagePtr!=0);
    case VAR_IMAGE_NDX:
        return bool(m_data.imageNdx!=0);
    default:
        throw CException("Can't convert field for that type");
    }
}

double CVariant::asFloat() const throw(CException) {
    if (m_dataType & VAR_NULL)
        return 0;
    switch (dataType()) {
    case VAR_BOOL:
        return m_data.boolData;
    case VAR_INT:
        return m_data.intData;
    case VAR_INT64:
        return (double) m_data.int64Data;
    case VAR_MONEY:
        return (double) m_data.moneyData;
    case VAR_FLOAT:
        return m_data.floatData;
    case VAR_STRING:
    case VAR_TEXT:
    case VAR_BUFFER:
        return strtod(m_data.buffer.data, 0);
    case VAR_DATE:
    case VAR_DATE_TIME:
        return m_data.floatData;
    default:
        throw CException("Can't convert field for that type");
    }
}

string CVariant::asString() const throw(CException) {
    if (m_dataType & VAR_NULL)
        return "";
    char print_buffer[64];
    switch (dataType()) {
    case VAR_NONE:
        return "";
    case VAR_BOOL:
        if (m_data.boolData)
            return "t";
        return "f";
    case VAR_INT:
        sprintf(print_buffer,"%i",m_data.intData);
        return string(print_buffer);
    case VAR_INT64:
#if SIZEOF_LONG == 8
        sprintf(print_buffer,"%li",m_data.int64Data);
#else
        sprintf(print_buffer,"%lli",m_data.int64Data);
#endif
        return string(print_buffer);
    case VAR_MONEY: {
            char format[16];
            int64_t absValue;
            char *formatPtr = format;
            if (m_data.moneyData.quantity < 0) {
                *formatPtr = '-';
                formatPtr++;
                absValue = -m_data.moneyData.quantity;
            } else
                absValue = m_data.moneyData.quantity;
            sprintf(formatPtr, "%%Ld.%%0%dLd", m_data.moneyData.scale);
            int64_t intValue = absValue / CMoneyData::dividers[m_data.moneyData.scale];
            int64_t fraction = absValue % CMoneyData::dividers[m_data.moneyData.scale];
            sprintf(print_buffer, format, intValue, fraction);
            return string(print_buffer);
        }
    case VAR_FLOAT: {
            const char *formatString = "%0.4f";
            if (floor(m_data.floatData) == m_data.floatData)
                formatString = "%0.0f";
            if (fabs(m_data.floatData) > 1e16)
                formatString = "%0.4e";
            sprintf(print_buffer,formatString,m_data.floatData);
            return string(print_buffer);
        }
    case VAR_STRING:
    case VAR_TEXT:
    case VAR_BUFFER:
        return m_data.buffer.data;
    case VAR_DATE:
        return CDateTime(m_data.floatData).dateString();
    case VAR_DATE_TIME: {
            CDateTime dt(m_data.floatData);
            return dt.dateString() + " " + dt.timeString();
        }
    case VAR_IMAGE_PTR:
        sprintf(print_buffer,"%p",(char *)m_data.imagePtr);
        return string(print_buffer);
    case VAR_IMAGE_NDX:
        sprintf(print_buffer,"%i",m_data.imageNdx);
        return string(print_buffer);
    default:
        throw CException("Can't convert field for that type");
    }
}

CDateTime CVariant::asDate() const throw(CException) {
    if (m_dataType & VAR_NULL)
        return 0.0;
    switch (dataType()) {
    case VAR_BOOL:
        return double(0);
    case VAR_INT:
        return m_data.intData;
    case VAR_INT64:
        return (double) m_data.int64Data;
    case VAR_MONEY:
        return (double) m_data.moneyData;
    case VAR_FLOAT:
        return m_data.floatData;

    case VAR_STRING:
    case VAR_TEXT:
    case VAR_BUFFER:
        return m_data.buffer.data;

    case VAR_DATE:
    case VAR_DATE_TIME:
        return int(m_data.floatData);

    default:
        throw CException("Can't convert field for that type");
    }
}

CDateTime CVariant::asDateTime() const throw(CException) {
    if (m_dataType & VAR_NULL)
        return 0.0;
    switch (dataType()) {
    case VAR_BOOL:
        return double(0);
    case VAR_INT:
        return m_data.intData;
    case VAR_INT64:
        return (double) m_data.int64Data;
    case VAR_MONEY:
        return (double) m_data.moneyData;
    case VAR_FLOAT:
        return m_data.floatData;

    case VAR_STRING:
    case VAR_TEXT:
    case VAR_BUFFER:
        return m_data.buffer.data;

    case VAR_DATE:
    case VAR_DATE_TIME:
        return m_data.floatData;

    default:
        throw CException("Can't convert field for that type");
    }
}

void *CVariant::asImagePtr() const throw(CException) {
    if (m_dataType & VAR_NULL)
        return 0;
    switch (dataType()) {
    case VAR_IMAGE_PTR:
        return m_data.imagePtr;
    default:
        throw CException("Can't convert field for that type");
    }
}

void CVariant::setNull(CVariantType vtype) {
    if (vtype != sptk::VAR_NONE) {
        releaseBuffers();
        dataType(VAR_BUFFER);
    }
    switch (dataType()) {
    default:
        m_data.int64Data = 0;
        break;

    case VAR_STRING:
    case VAR_TEXT:
    case VAR_BUFFER:
        if (m_dataType & VAR_EXTERNAL_BUFFER)
            m_data.buffer.data = 0;
        else if (m_data.buffer.data)
            m_data.buffer.data[0] = 0;
        break;
    }
    m_dataType |= VAR_NULL;
}

string CVariant::typeName(CVariantType type) {
    switch (type) {
    default:
        return "undefined";
    case VAR_BOOL:
        return "bool";
    case VAR_INT:
        return "int";
    case VAR_INT64:
        return "int64";
    case VAR_FLOAT:
        return "double";
    case VAR_MONEY:
        return "money";
    case VAR_STRING:
        return "string";
    case VAR_TEXT:
        return "text";
    case VAR_BUFFER:
        return "blob";
    case VAR_DATE:
        return "date";
    case VAR_DATE_TIME:
        return "datetime";
    case VAR_IMAGE_PTR:
        return "imageptr";
    case VAR_IMAGE_NDX:
        return "imagendx";
    }
}

CVariantType CVariant::nameType(const char* name) {
    static std::map<string,CVariantType> nameToTypeMap;
    if (nameToTypeMap.empty()) {
        for (int type = VAR_INT; type <= VAR_BOOL; type*=2) {
            CVariantType vtype = (CVariantType) type;
            nameToTypeMap[typeName(vtype)] = vtype;
        }
    }
    if (!name || name[0] == 0)
        name = "string";
    std::map<string,CVariantType>::iterator itor = nameToTypeMap.find(lowerCase(name));
    if (itor == nameToTypeMap.end())
        throw CException("Type name "+string(name)+" isn't recognized",__FILE__,__LINE__);
    return itor->second;
}

void CVariant::load(const CXmlNode& node) {
    const string& ntype = node.getAttribute("type").str();
    unsigned type = nameType(ntype.c_str());
    switch (type) {
    case VAR_BOOL:
    case VAR_INT:
    case VAR_INT64:
    case VAR_FLOAT:
    case VAR_MONEY:
    case VAR_STRING:
    case VAR_DATE:
    case VAR_DATE_TIME:
    case VAR_IMAGE_NDX:
        *this = node.text();
        break;
    case VAR_TEXT:
    case VAR_BUFFER:
        *this = node.text();
        break;
    default:
    case VAR_IMAGE_PTR:
        break;
    }
}

void CVariant::save(CXmlNode& node) const {
    string stringValue(asString());
    node.setAttribute("type",typeName(dataType()));

    if (!stringValue.empty()) {
        switch (dataType()) {
        case VAR_BOOL:
        case VAR_INT:
        case VAR_INT64:
        case VAR_FLOAT:
        case VAR_MONEY:
        case VAR_STRING:
        case VAR_DATE:
        case VAR_DATE_TIME:
        case VAR_IMAGE_NDX:
            new CXmlText(node,stringValue);
            break;
        case VAR_TEXT:
        case VAR_BUFFER:
            new CXmlCDataSection(node,asString());
            break;
        default:
        case VAR_IMAGE_PTR:
            break;
        }
    }
}
