/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CVariant.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/CVariant.h>
#include <sptk5/CField.h>

using namespace std;
using namespace sptk;

int64_t MoneyData::dividers[16] = {1, 10, 100, 1000, 10000, 100000, 1000000L, 10000000L, 100000000LL, 1000000000LL,
                                    10000000000LL, 100000000000LL,
                                    1000000000000LL, 10000000000000LL, 100000000000000LL, 1000000000000000LL};

MoneyData::operator double() const
{
    return double(quantity) / dividers[scale];
}

MoneyData::operator int64_t() const
{
    return quantity / dividers[scale];
}

MoneyData::operator int32_t() const
{
    return int32_t(quantity / dividers[scale]);
}

MoneyData::operator bool() const
{
    return quantity != 0;
}

//---------------------------------------------------------------------------
void Variant::releaseBuffers()
{
    if (m_dataType & (VAR_STRING | VAR_BUFFER | VAR_TEXT)) {
        if (m_data.buffer.data) {
            if (!(m_dataType & VAR_EXTERNAL_BUFFER))
                free(m_data.buffer.data);

            m_data.buffer.data = NULL;
            m_data.buffer.size = 0;
        }
    }
}

//---------------------------------------------------------------------------
void Variant::dataSize(size_t ds)
{
    m_dataSize = ds;
    if (m_dataSize > 0)
        m_dataType &= VAR_TYPES | VAR_EXTERNAL_BUFFER;
    }

//---------------------------------------------------------------------------
void Variant::dataType(uint32_t dt)
{
    m_dataType = (uint16_t) dt;
}

//---------------------------------------------------------------------------
Variant::Variant()
{
    m_dataType = VAR_NONE | VAR_NULL;
    m_data.int64Data = 0;
    m_data.buffer.size = 0;
}

//---------------------------------------------------------------------------
Variant::Variant(int32_t value)
{
    m_dataType = VAR_INT;
    m_data.intData = value;
}

//---------------------------------------------------------------------------
Variant::Variant(uint32_t value)
{
    m_dataType = VAR_INT;
    m_data.intData = (int32_t) value;
}

//---------------------------------------------------------------------------
Variant::Variant(int64_t value, unsigned scale)
{
    if (scale > 1) {
        m_dataType = VAR_MONEY;
        m_data.moneyData.quantity = value;
        m_data.moneyData.scale = (uint8_t) scale;
    } else {
        m_dataType = VAR_INT64;
        m_data.int64Data = value;
    }
}

//---------------------------------------------------------------------------
Variant::Variant(uint64_t value)
{
    m_dataType = VAR_INT64;
    m_data.int64Data = (int64_t) value;
}

//---------------------------------------------------------------------------
Variant::Variant(float value)
{
    m_dataType = VAR_FLOAT;
    m_data.floatData = value;
}

//---------------------------------------------------------------------------
Variant::Variant(double value)
{
    m_dataType = VAR_FLOAT;
    m_data.floatData = value;
}

//---------------------------------------------------------------------------
Variant::Variant(const char * value)
{
    m_dataType = VAR_NONE;
    setString(value);
}

//---------------------------------------------------------------------------
Variant::Variant(const std::string& v)
{
    m_dataType = VAR_NONE;
    setString(v.c_str(), v.length());
}

//---------------------------------------------------------------------------
Variant::Variant(CDateTime v)
{
    m_dataType = VAR_DATE_TIME;
    m_data.timeData = v;
}

//---------------------------------------------------------------------------
Variant::Variant(const void * value, size_t sz)
{
    m_dataType = VAR_NONE;
    setBuffer(value, sz);
}

//---------------------------------------------------------------------------
Variant::Variant(const CBuffer& value)
{
    m_dataType = VAR_NONE;
    setBuffer(value.data(), value.bytes());
}

//---------------------------------------------------------------------------
Variant::Variant(const Variant& value)
{
    m_dataType = VAR_NONE;
    setData(value);
}

//---------------------------------------------------------------------------
Variant::~Variant()
{
    releaseBuffers();
}

//---------------------------------------------------------------------------
void Variant::setBool(bool value)
{
    if (m_dataType != VAR_BOOL) {
        releaseBuffers();
        m_dataType = VAR_BOOL;
        m_dataSize = sizeof(value);
    }

    m_data.boolData = value;
}

//---------------------------------------------------------------------------
void Variant::setInteger(int32_t value)
{
    if (m_dataType != VAR_INT) {
        releaseBuffers();
        m_dataType = VAR_INT;
        m_dataSize = sizeof(value);
    }

    m_data.intData = value;
}

//---------------------------------------------------------------------------
void Variant::setInt64(int64_t value)
{
    if (m_dataType != VAR_INT64) {
        releaseBuffers();
        m_dataType = VAR_INT64;
        m_dataSize = sizeof(value);
    }

    m_data.int64Data = value;
}

//---------------------------------------------------------------------------
void Variant::setFloat(double value)
{
    if (m_dataType != VAR_FLOAT) {
        releaseBuffers();
        m_dataType = VAR_FLOAT;
        m_dataSize = sizeof(value);
    }

    m_data.floatData = value;
}

//---------------------------------------------------------------------------
void Variant::setMoney(int64_t value, unsigned scale)
{
    if (m_dataType != VAR_MONEY) {
        releaseBuffers();
        m_dataType = VAR_MONEY;
        m_dataSize = sizeof(MoneyData);
    }

    m_data.moneyData.quantity = value;
    m_data.moneyData.scale = (uint8_t) scale;
}

//---------------------------------------------------------------------------
void Variant::setString(const char* value, size_t maxlen)
{
    uint32_t dtype = VAR_STRING;

    if (dataType() == VAR_STRING && maxlen && m_data.buffer.size == maxlen + 1) {
        if (value) {
            strncpy(m_data.buffer.data, value, maxlen);
            m_data.buffer.data[maxlen] = 0;
        }
        else {
            m_data.buffer.data[0] = 0;
            dtype |= VAR_NULL;
        }
    }
    else {
        releaseBuffers();

        if (value) {
            if (maxlen) {
                dataSize(maxlen);
                m_data.buffer.size = maxlen + 1;
                m_data.buffer.data = (char*) malloc(m_data.buffer.size);
                if (m_data.buffer.data) {
                    strncpy(m_data.buffer.data, value, maxlen);
                    m_data.buffer.data[maxlen] = 0;
                }
            }
            else {
                dataSize(strlen(value));
                dtype &= VAR_TYPES | VAR_EXTERNAL_BUFFER;
                m_data.buffer.size = dataSize() + 1;
                m_data.buffer.data = strdup(value);
            }
        }
        else {
            m_data.buffer.data = NULL;
            dataSize(0);
            m_data.buffer.size = 0;
            dtype |= VAR_NULL;
        }
    }

    dataType(dtype);
}

//---------------------------------------------------------------------------
void Variant::setString(const std::string& value)
{
    setString(value.c_str(), value.length());
}

//---------------------------------------------------------------------------
void Variant::setExternalString(const char* value, int length)
{
    uint32_t dtype = VAR_STRING;

    if (!(m_dataType & VAR_EXTERNAL_BUFFER))
        releaseBuffers();

    if (value) {
        if (length < 0)
            length = (int) strlen(value);

        m_dataSize = size_t(length);
        m_data.buffer.size = size_t(length + 1);
        dtype |= VAR_EXTERNAL_BUFFER;
    }
    else {
        m_dataSize = 0;
        m_data.buffer.size = 0;
        dtype |= VAR_NULL;
    }

    m_data.buffer.data = (char*) value;
    m_dataType = (uint16_t) dtype;
}

//---------------------------------------------------------------------------
void Variant::setExternalString(const std::string& value)
{
    setExternalString(value.c_str(), (int) value.length());
}

//---------------------------------------------------------------------------
void Variant::setText(const char* value)
{
    releaseBuffers();
    dataType(VAR_TEXT);

    if (value) {
        dataSize(strlen(value));
        m_data.buffer.size = dataSize() + 1;
        m_data.buffer.data = strdup(value);
    }
    else {
        m_dataType |= VAR_NULL;
        m_data.buffer.data = NULL;
        m_data.buffer.size = 0;
        dataSize(0);
    }
}

//---------------------------------------------------------------------------
void Variant::setText(const string& value)
{
    releaseBuffers();
    dataType(VAR_TEXT);
    size_t vlen = value.length();

    if (vlen) {
        dataSize(vlen);
        m_data.buffer.size = vlen + 1;
        m_data.buffer.data = strdup(value.c_str());
    }
    else {
        m_dataType |= VAR_NULL;
        m_data.buffer.data = NULL;
        m_data.buffer.size = 0;
        dataSize(0);
    }
}

//---------------------------------------------------------------------------
void Variant::setExternalText(const char* value)
{
    releaseBuffers();
    dataType(VAR_TEXT);

    if (value) {
        dataSize(strlen(value));
        m_data.buffer.size = dataSize() + 1;
        m_data.buffer.data = (char*) value;
        m_dataType |= VAR_EXTERNAL_BUFFER;
    }
    else {
        m_dataType |= VAR_NULL;
        m_data.buffer.data = NULL;
        m_data.buffer.size = 0;
        dataSize(0);
    }
}

//---------------------------------------------------------------------------
void Variant::setBuffer(const void* value, size_t sz)
{
    releaseBuffers();
    dataType(VAR_BUFFER);

    if (value || sz) {
        m_data.buffer.size = sz;
        dataSize(sz);
        m_data.buffer.data = (char*) malloc(sz);

        if (m_data.buffer.data && value)
            memcpy(m_data.buffer.data, value, sz);
    }
    else
        setNull();
}

void Variant::setBuffer(const CBuffer& value)
{
    setBuffer(value.data(), value.bytes());
}

//---------------------------------------------------------------------------
void Variant::setBuffer(const string& value)
{
    releaseBuffers();
    dataType(VAR_BUFFER);
    size_t vlen = value.length();

    if (vlen) {
        size_t sz = vlen + 1;
        m_data.buffer.size = sz;
        dataSize(sz);
        m_data.buffer.data = (char*) malloc(sz);
        if (m_data.buffer.data && value.c_str())
            memcpy(m_data.buffer.data, value.c_str(), sz);
    }
    else
        setNull();
}

//---------------------------------------------------------------------------
void Variant::setExternalBuffer(const void* value, size_t sz)
{
    releaseBuffers();
    dataType(VAR_BUFFER);
    m_data.buffer.data = (char*) value;

    if (value) {
        m_data.buffer.size = sz;
        dataSize(sz);
        m_dataType |= VAR_EXTERNAL_BUFFER;
    }
    else {
        dataSize(0);
        m_dataType |= VAR_NULL;
    }
}

//---------------------------------------------------------------------------
void Variant::setDateTime(CDateTime value)
{
    if (m_dataType != VAR_DATE_TIME) {
        releaseBuffers();
        dataType(VAR_DATE_TIME);
        dataSize(sizeof(value));
    }

    m_data.floatData = value;
}

//---------------------------------------------------------------------------
void Variant::setDate(CDateTime value)
{
    if (m_dataType != VAR_DATE) {
        releaseBuffers();
        dataType(VAR_DATE);
        dataSize(sizeof(value));
    }

    m_data.floatData = value;
}

//---------------------------------------------------------------------------
void Variant::setImagePtr(const void* value)
{
    if (m_dataType != VAR_IMAGE_PTR) {
        releaseBuffers();
        dataType(VAR_IMAGE_PTR);
        dataSize(sizeof(value));
    }

    m_data.imagePtr = (void*) value;
}

//---------------------------------------------------------------------------
void Variant::setImageNdx(uint32_t value)
{
    if (dataType() != VAR_IMAGE_NDX) {
        releaseBuffers();
        dataType(VAR_IMAGE_NDX);
    }

    dataSize(sizeof(value));
    m_data.imageNdx = (int32_t) value;
}

//---------------------------------------------------------------------------
void Variant::setMoney(const MoneyData& value)
{
    if (dataType() != VAR_MONEY) {
        releaseBuffers();
        dataType(VAR_MONEY);
    }

    dataSize(sizeof(value));
    m_data.moneyData = value;
}

//---------------------------------------------------------------------------
void Variant::setData(const Variant& C)
{
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
            setString(C.getString(), C.dataSize());
            break;

        case VAR_TEXT:
            setText(C.getText());
            break;

        case VAR_BUFFER:
            setBuffer(C.getBuffer(), C.dataSize());
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
Variant& Variant::operator =(const Variant &C)
{
    if (this == &C)
        return *this;
    setData(C);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(int32_t value)
{
    setInteger(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(int64_t value)
{
    setInt64(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(uint32_t value)
{
    setInteger((int32_t) value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(uint64_t value)
{
    setInt64((int64_t)value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(int16_t value)
{
    setInteger(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(uint16_t value)
{
    setInteger(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(float value)
{
    setFloat(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(double value)
{
    setFloat(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(const MoneyData& value)
{
    setMoney(value.quantity, value.scale);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(const char * value)
{
    setString(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(const std::string& value)
{
    setString(value.c_str(), value.length());
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(CDateTime value)
{
    setDateTime(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(const void *value)
{
    setImagePtr(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator =(const CBuffer& value)
{
    setBuffer(value.data(), value.bytes());
    return *this;
}

//---------------------------------------------------------------------------
bool Variant::getBool() const
{
    return m_data.boolData;
}

//---------------------------------------------------------------------------
const int32_t& Variant::getInteger() const
{
    return m_data.intData;
}

//---------------------------------------------------------------------------
const int64_t& Variant::getInt64() const
{
    return m_data.int64Data;
}

//---------------------------------------------------------------------------
const double& Variant::getFloat() const
{
    return m_data.floatData;
}

//---------------------------------------------------------------------------
const MoneyData& Variant::getMoney() const
{
    return m_data.moneyData;
}

//---------------------------------------------------------------------------
const char* Variant::getString() const
{
    return m_data.buffer.data;
}

//---------------------------------------------------------------------------
const char* Variant::getBuffer() const
{
    return m_data.buffer.data;
}

//---------------------------------------------------------------------------
const char* Variant::getText() const
{
    return m_data.buffer.data;
}

//---------------------------------------------------------------------------
CDateTime Variant::getDateTime() const
{
    return m_data.floatData;
}

//---------------------------------------------------------------------------
CDateTime Variant::getDate() const
{
    return (int) m_data.floatData;
}

//---------------------------------------------------------------------------
void* Variant::getImagePtr() const
{
    return m_data.imagePtr;
}

//---------------------------------------------------------------------------
uint32_t Variant::getImageNdx() const
{
    return (uint32_t) m_data.imageNdx;
}

//---------------------------------------------------------------------------
VariantType Variant::dataType() const
{
    return (VariantType) (m_dataType & VAR_TYPES);
}

//---------------------------------------------------------------------------
size_t Variant::dataSize() const
{
    return m_dataSize;
}

//---------------------------------------------------------------------------
size_t Variant::bufferSize() const
{
    return m_data.buffer.size;
}

//---------------------------------------------------------------------------
void* Variant::dataBuffer() const
{
    return (void *) (variantData *) &m_data;
}

//---------------------------------------------------------------------------
Variant::operator bool() const THROWS_EXCEPTIONS
{
    return asBool();
}

//---------------------------------------------------------------------------
Variant::operator int32_t() const THROWS_EXCEPTIONS
{
    return asInteger();
}

//---------------------------------------------------------------------------
Variant::operator uint32_t() const THROWS_EXCEPTIONS
{
    return (uint32_t) asInteger();
}

//---------------------------------------------------------------------------
Variant::operator int64_t() const THROWS_EXCEPTIONS
{
    return asInt64();
}

//---------------------------------------------------------------------------
Variant::operator uint64_t() const THROWS_EXCEPTIONS
{
    return (uint64_t) asInt64();
}

//---------------------------------------------------------------------------
Variant::operator float() const THROWS_EXCEPTIONS
{
    return (float) asFloat();
}

//---------------------------------------------------------------------------
Variant::operator double() const THROWS_EXCEPTIONS
{
    return asFloat();
}

//---------------------------------------------------------------------------
Variant::operator std::string() const THROWS_EXCEPTIONS
{
    return asString();
}

//---------------------------------------------------------------------------
Variant::operator CDateTime() const THROWS_EXCEPTIONS
{
    return asDateTime();
}

//---------------------------------------------------------------------------
// convertors
int32_t Variant::asInteger() const THROWS_EXCEPTIONS
{
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
            return (int32_t) atol(m_data.buffer.data);

        case VAR_DATE:
        case VAR_DATE_TIME:
            return (int32_t) m_data.floatData;

        default:
            throw CException("Can't convert field for that type");
    }
}

int64_t Variant::asInt64() const THROWS_EXCEPTIONS
{
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
            return (int64_t) m_data.moneyData;

        case VAR_FLOAT:
            return (int64_t) m_data.floatData;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
#ifdef _MSC_VER
            return _atoi64 (m_data.buffer.data);
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

bool Variant::asBool() const THROWS_EXCEPTIONS
{
    char ch;

    if (m_dataType & VAR_NULL)
        return false;

    switch (dataType()) {
        case VAR_BOOL:
            return m_data.boolData;

        case VAR_INT:
            return (m_data.intData > 0);

        case VAR_INT64:
            return (m_data.int64Data > 0);

        case VAR_MONEY:
            return (m_data.moneyData.quantity > 0);

        case VAR_FLOAT:
            return (m_data.floatData > 0);

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            ch = m_data.buffer.data[0];
            return (strchr("YyTt1", ch) != 0);

        case VAR_DATE:
        case VAR_DATE_TIME:
            return bool(m_data.floatData != 0);

        case VAR_IMAGE_PTR:
            return bool(m_data.imagePtr != 0);

        case VAR_IMAGE_NDX:
            return bool(m_data.imageNdx != 0);

        default:
            throw CException("Can't convert field for that type");
    }
}

double Variant::asFloat() const THROWS_EXCEPTIONS
{
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

string Variant::asString() const THROWS_EXCEPTIONS
{
    if (m_dataType & VAR_NULL)
        return "";

    char print_buffer[64];

    switch (dataType()) {
        default:
            return "";

        case VAR_BOOL:
            if (m_data.boolData)
                return "t";

            return "f";

        case VAR_INT:
            sprintf(print_buffer, "%i", m_data.intData);
            return string(print_buffer);

        case VAR_INT64:
#ifndef _WIN32
            sprintf(print_buffer, "%li", m_data.int64Data);
#else
            sprintf (print_buffer,"%lli",m_data.int64Data);
#endif
            return string(print_buffer);

        case VAR_MONEY: {
            char format[16];
            int64_t absValue;
            char* formatPtr = format;

            if (m_data.moneyData.quantity < 0) {
                *formatPtr = '-';
                formatPtr++;
                absValue = -m_data.moneyData.quantity;
            }
            else
                absValue = m_data.moneyData.quantity;

            sprintf(formatPtr, "%%Ld.%%0%dLd", m_data.moneyData.scale);
            int64_t intValue = absValue / MoneyData::dividers[m_data.moneyData.scale];
            int64_t fraction = absValue % MoneyData::dividers[m_data.moneyData.scale];
            sprintf(print_buffer, format, intValue, fraction);
            return string(print_buffer);
        }

        case VAR_FLOAT: {
            const char* formatString = "%0.4f";

            if (floor(m_data.floatData) == m_data.floatData)
                formatString = "%0.0f";

            if (fabs(m_data.floatData) > 1e16)
                formatString = "%0.4e";

            sprintf(print_buffer, formatString, m_data.floatData);
            return string(print_buffer);
        }

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (m_data.buffer.data)
                return m_data.buffer.data;
            else
                return "";

        case VAR_DATE:
            return CDateTime(m_data.floatData).dateString();

        case VAR_DATE_TIME: {
            CDateTime dt(m_data.floatData);
            return dt.dateString() + " " + dt.timeString(true);
        }

        case VAR_IMAGE_PTR:
            sprintf(print_buffer, "%p", m_data.imagePtr);
            return string(print_buffer);

        case VAR_IMAGE_NDX:
            sprintf(print_buffer, "%i", m_data.imageNdx);
            return string(print_buffer);
    }
}

CDateTime Variant::asDate() const THROWS_EXCEPTIONS
{
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

CDateTime Variant::asDateTime() const THROWS_EXCEPTIONS
{
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

void* Variant::asImagePtr() const THROWS_EXCEPTIONS
{
    if (m_dataType & VAR_NULL)
        return 0;

    switch (dataType()) {
        case VAR_IMAGE_PTR:
            return m_data.imagePtr;

        default:
            throw CException("Can't convert field for that type");
    }
}

void Variant::setNull(VariantType vtype)
{
    if (vtype != sptk::VAR_NONE) {
        releaseBuffers();
        dataType(vtype);
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

bool Variant::isNull() const
{
    return (m_dataType & VAR_NULL) != 0;
}

string Variant::typeName(VariantType type)
{
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

VariantType Variant::nameType(const char* name)
{
    static std::map<string, VariantType> nameToTypeMap;

    if (nameToTypeMap.empty()) {
        for (int type = VAR_INT; type <= VAR_BOOL; type *= 2) {
            VariantType vtype = (VariantType) type;
            nameToTypeMap[typeName(vtype)] = vtype;
        }
    }

    if (!name || name[0] == 0)
        name = "string";

    std::map<string, VariantType>::iterator itor = nameToTypeMap.find(lowerCase(name));

    if (itor == nameToTypeMap.end())
        throw CException("Type name " + string(name) + " isn't recognized", __FILE__, __LINE__);

    return itor->second;
}

void Variant::load(const CXmlNode& node)
{
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

void Variant::load(const CXmlNode* node)
{
    load(*node);
}

void Variant::save(CXmlNode& node) const
{
    string stringValue(asString());
    node.setAttribute("type", typeName(dataType()));

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
                new CXmlText(node, stringValue);
                break;

            case VAR_TEXT:
            case VAR_BUFFER:
                new CXmlCDataSection(node, asString());
                break;

            default:
            case VAR_IMAGE_PTR:
                break;
        }
    }
}

void Variant::save(CXmlNode* node) const
{
    save(*node);
}
