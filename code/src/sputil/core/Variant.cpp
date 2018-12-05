/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CVariant.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <cmath>
#include <sptk5/Field.h>

using namespace std;
using namespace sptk;

#define BUFFER_TYPES (VAR_STRING|VAR_TEXT|VAR_BUFFER)

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

MoneyData::operator size_t() const
{
    return size_t(quantity / dividers[scale]);
}

MoneyData::operator int() const
{
    return int(quantity / dividers[scale]);
}

MoneyData::operator bool() const
{
    return quantity != 0;
}

//---------------------------------------------------------------------------
void BaseVariant::releaseBuffers()
{
    if ((dataType() & (VAR_STRING | VAR_BUFFER | VAR_TEXT)) != 0 &&
        m_data.getBuffer().data != nullptr) {
        if (!isExternalBuffer())
            delete [] m_data.getBuffer().data;

        m_data.getBuffer().data = nullptr;
        m_data.getBuffer().size = 0;
    }
}

//---------------------------------------------------------------------------
void BaseVariant::dataSize(size_t ds)
{
    m_dataSize = ds;
    if (m_dataSize > 0)
        m_dataType &= VAR_TYPES | VAR_EXTERNAL_BUFFER;
}

//---------------------------------------------------------------------------
Variant::Variant()
{
    setDataType(VAR_NONE | VAR_NULL);
    m_data.getInt64() = 0;
    m_data.getBuffer().size = 0;
}

//---------------------------------------------------------------------------
Variant::Variant(int32_t value)
{
    setDataType(VAR_INT);
    m_data.getInteger() = value;
}

//---------------------------------------------------------------------------
Variant::Variant(uint32_t value)
{
    setDataType(VAR_INT);
    m_data.getInteger() = (int32_t) value;
}

//---------------------------------------------------------------------------
Variant::Variant(int64_t value, unsigned scale)
{
    if (scale > 1) {
        setDataType(VAR_MONEY);
        m_data.getMoneyData().quantity = value;
        m_data.getMoneyData().scale = (uint8_t) scale;
    } else {
        setDataType(VAR_INT64);
        m_data.getInt64() = value;
    }
}

//---------------------------------------------------------------------------
Variant::Variant(uint64_t value)
{
    setDataType(VAR_INT64);
    m_data.getInt64() = (int64_t) value;
}

//---------------------------------------------------------------------------
Variant::Variant(double value)
{
    setDataType(VAR_FLOAT);
    m_data.getFloat() = value;
}

//---------------------------------------------------------------------------
Variant::Variant(const char* value)
{
    setDataType(VAR_NONE);
    Variant::setString(value);
}

//---------------------------------------------------------------------------
Variant::Variant(const String& v)
{
    setDataType(VAR_NONE);
    Variant::setString(v.c_str(), v.length());
}

//---------------------------------------------------------------------------
Variant::Variant(DateTime v)
{
    setDataType(VAR_DATE_TIME);
    DateTime::duration sinceEpoch = v.timePoint().time_since_epoch();
    m_data.getInt64() = chrono::duration_cast<chrono::microseconds>(sinceEpoch).count();
}

//---------------------------------------------------------------------------
Variant::Variant(const void* value, size_t sz)
{
    setDataType(VAR_NONE);
    Variant::setBuffer(value, sz, VAR_DATE_TIME, false);
}

//---------------------------------------------------------------------------
Variant::Variant(const Buffer& value)
{
    setDataType(VAR_NONE);
    Variant::setBuffer(value.data(), value.bytes(), VAR_DATE_TIME, false);
}

//---------------------------------------------------------------------------
Variant::Variant(const Variant& value)
{
    setDataType(VAR_NONE);
    setData(value);
}

//---------------------------------------------------------------------------
Variant::~Variant()
{
    releaseBuffers();
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setBool(bool value)
{
    if (m_dataType != VAR_BOOL) {
        releaseBuffers();
        setDataType(VAR_BOOL);
        m_dataSize = sizeof(value);
    }

    m_data.getBool() = value;
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setInteger(int32_t value)
{
    if (m_dataType != VAR_INT) {
        releaseBuffers();
        setDataType(VAR_INT);
        m_dataSize = sizeof(value);
    }

    m_data.getInteger() = value;
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setInt64(int64_t value)
{
    if (m_dataType != VAR_INT64) {
        releaseBuffers();
        setDataType(VAR_INT64);
        m_dataSize = sizeof(value);
    }

    m_data.getInt64() = value;
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setFloat(double value)
{
    if (m_dataType != VAR_FLOAT) {
        releaseBuffers();
        setDataType(VAR_FLOAT);
        m_dataSize = sizeof(value);
    }

    m_data.getFloat() = value;
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setMoney(int64_t value, unsigned scale)
{
    if (m_dataType != VAR_MONEY) {
        releaseBuffers();
        setDataType(VAR_MONEY);
        m_dataSize = sizeof(MoneyData);
    }

    m_data.getMoneyData().quantity = value;
    m_data.getMoneyData().scale = (uint8_t) scale;
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setString(const char* value, size_t maxlen)
{
    uint32_t dtype = VAR_STRING;

    if (dataType() == VAR_STRING && maxlen != 0 && m_data.getBuffer().size == maxlen + 1) {
        if (value != nullptr) {
            strncpy(m_data.getBuffer().data, value, maxlen);
            m_data.getBuffer().data[maxlen] = 0;
        } else {
            m_data.getBuffer().data[0] = 0;
            dtype |= VAR_NULL;
        }
    } else {
        releaseBuffers();

        if (value != nullptr) {
            if (maxlen != 0) {
                dataSize(maxlen);
                m_data.getBuffer().size = maxlen + 1;
                m_data.getBuffer().data = new char[m_data.getBuffer().size];
                if (m_data.getBuffer().data != nullptr) {
                    strncpy(m_data.getBuffer().data, value, maxlen);
                    m_data.getBuffer().data[maxlen] = 0;
                }
            } else {
                dataSize(strlen(value));
                dtype &= VAR_TYPES | VAR_EXTERNAL_BUFFER;
                m_data.getBuffer().size = dataSize() + 1;
                m_data.getBuffer().data = new char[m_data.getBuffer().size];
                strncpy(m_data.getBuffer().data, value, m_data.getBuffer().size);
            }
        } else {
            m_data.getBuffer().data = nullptr;
            dataSize(0);
            m_data.getBuffer().size = 0;
            dtype |= VAR_NULL;
        }
    }

    setDataType(dtype);
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setString(const String& value)
{
    setString(value.c_str(), value.length());
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setText(const String& value)
{
    releaseBuffers();
    setDataType(VAR_TEXT);
    size_t vlen = value.length();

    if (vlen != 0) {
        dataSize(vlen);
        m_data.getBuffer().size = vlen + 1;
        m_data.getBuffer().data = new char[m_data.getBuffer().size];
        strncpy(m_data.getBuffer().data, value.c_str(), m_data.getBuffer().size);
    } else {
        m_dataType |= VAR_NULL;
        m_data.getBuffer().data = nullptr;
        m_data.getBuffer().size = 0;
        dataSize(0);
    }
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setBuffer(const void* value, size_t sz, VariantType type, bool externalBuffer)
{
    if ((type & BUFFER_TYPES) == 0)
        throw Exception("Invalid buffer type");

    releaseBuffers();
    setDataType(type);

    if (value != nullptr || sz != 0) {
        if (externalBuffer) {
            m_data.getBuffer().size = sz;
            m_data.getBuffer().data = (char*) value;
            dataSize(sz);
            setDataType(type | VAR_EXTERNAL_BUFFER);
        } else {
            m_data.getBuffer().size = sz;
            dataSize(sz);
            m_data.getBuffer().data = new char[sz];
            if (m_data.getBuffer().data != nullptr && value != nullptr)
                memcpy(m_data.getBuffer().data, value, sz);
        }
    } else
        setNull();
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setDateTime(DateTime value, bool dateOnly)
{
    if ((m_dataType & (VAR_DATE|VAR_DATE_TIME)) == 0) {
        releaseBuffers();
        dataSize(sizeof(value));
    }

    DateTime::duration sinceEpoch = value.timePoint().time_since_epoch();
    if (dateOnly) {
        setDataType(VAR_DATE);
        int64_t days = chrono::duration_cast<chrono::hours>(sinceEpoch).count() / 24;
        m_data.getInt64() = days * 86400 * 1000000;
    } else {
        setDataType(VAR_DATE_TIME);
        m_data.getInt64() = chrono::duration_cast<chrono::microseconds>(sinceEpoch).count();
    }
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setImagePtr(const void* value)
{
    if (m_dataType != VAR_IMAGE_PTR) {
        releaseBuffers();
        setDataType(VAR_IMAGE_PTR);
        dataSize(sizeof(value));
    }

    m_data.setImagePtr((void*) value);
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setImageNdx(uint32_t value)
{
    if (dataType() != VAR_IMAGE_NDX) {
        releaseBuffers();
        setDataType(VAR_IMAGE_NDX);
    }

    dataSize(sizeof(value));
    m_data.getInteger() = (int32_t) value;
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setMoney(const MoneyData& value)
{
    if (dataType() != VAR_MONEY) {
        releaseBuffers();
        setDataType(VAR_MONEY);
    }

    dataSize(sizeof(value));
    m_data.getMoneyData() = value;
}

//---------------------------------------------------------------------------
void Variant_SetMethods::setData(const BaseVariant& C)
{
    switch (C.dataType()) {
        case VAR_BOOL:
            setInteger(C.getBool() ? 1 : 0);
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
            setMoney(C.m_data.getMoneyData().quantity, C.m_data.getMoneyData().scale);
            break;

        case VAR_STRING:
            setString(C.getString(), C.dataSize());
            break;

        case VAR_TEXT:
            setText(C.getText());
            break;

        case VAR_BUFFER:
            setBuffer(C.getBuffer(), C.dataSize(), VAR_DATE_TIME, false);
            break;

        case VAR_DATE:
            setDateTime(C.getDateTime(), true);
            break;

        case VAR_DATE_TIME:
            setDateTime(C.getDateTime(), false);
            break;

        case VAR_IMAGE_PTR:
            setImagePtr(C.getImagePtr());
            break;

        case VAR_IMAGE_NDX:
            setImageNdx(C.getImageNdx());
            break;

        default:
            break;
    }

    m_dataType = C.m_dataType;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const Variant& C)
{
    if (this == &C)
        return *this;
    setData(C);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(int32_t value)
{
    setInteger(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(int64_t value)
{
    setInt64(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(double value)
{
    setFloat(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const MoneyData& value)
{
    setMoney(value.quantity, value.scale);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const char* value)
{
    setString(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const String& value)
{
    setString(value.c_str(), value.length());
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(DateTime value)
{
    setDateTime(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const void* value)
{
    setImagePtr(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const Buffer& value)
{
    setBuffer(value.data(), value.bytes(), VAR_DATE_TIME, false);
    return *this;
}

//---------------------------------------------------------------------------
bool BaseVariant::getBool() const
{
    return m_data.getBool();
}

//---------------------------------------------------------------------------
const int32_t& BaseVariant::getInteger() const
{
    return m_data.getInteger();
}

//---------------------------------------------------------------------------
const int64_t& BaseVariant::getInt64() const
{
    return m_data.getInt64();
}

//---------------------------------------------------------------------------
const double& BaseVariant::getFloat() const
{
    return m_data.getFloat();
}

//---------------------------------------------------------------------------
const MoneyData& BaseVariant::getMoney() const
{
    return m_data.getMoneyData();
}

//---------------------------------------------------------------------------
const char* BaseVariant::getString() const
{
    return m_data.getBuffer().data;
}

//---------------------------------------------------------------------------
const char* BaseVariant::getBuffer() const
{
    return m_data.getBuffer().data;
}

//---------------------------------------------------------------------------
const char* BaseVariant::getText() const
{
    return m_data.getBuffer().data;
}

//---------------------------------------------------------------------------
DateTime BaseVariant::getDateTime() const
{
    return DateTime(DateTime::time_point(chrono::microseconds(m_data.getInt64())));
}

//---------------------------------------------------------------------------
DateTime BaseVariant::getDate() const
{
    int64_t days = m_data.getInt64() / 1000000 / 86400;
    return DateTime(DateTime::time_point(chrono::hours(days * 24)));
}

//---------------------------------------------------------------------------
const void* BaseVariant::getImagePtr() const
{
    return m_data.getImagePtr();
}

//---------------------------------------------------------------------------
uint32_t BaseVariant::getImageNdx() const
{
    return (uint32_t) m_data.getInteger();
}

//---------------------------------------------------------------------------
size_t BaseVariant::dataSize() const
{
    return m_dataSize;
}

//---------------------------------------------------------------------------
size_t BaseVariant::bufferSize() const
{
    return m_data.getBuffer().size;
}

//---------------------------------------------------------------------------
void* BaseVariant::dataBuffer() const
{
    return (void*) &m_data;
}

//---------------------------------------------------------------------------
Variant::operator bool() const
{
    return asBool();
}

//---------------------------------------------------------------------------
Variant::operator int() const
{
    return asInteger();
}

//---------------------------------------------------------------------------
Variant::operator unsigned() const
{
    return (unsigned) asInteger();
}

//---------------------------------------------------------------------------
Variant::operator int64_t() const
{
    return asInt64();
}

//---------------------------------------------------------------------------
Variant::operator uint64_t() const
{
    return (uint64_t) asInt64();
}

//---------------------------------------------------------------------------
Variant::operator double() const
{
    return asFloat();
}

//---------------------------------------------------------------------------
Variant::operator String() const
{
    return asString();
}

//---------------------------------------------------------------------------
Variant::operator DateTime() const
{
    return asDateTime();
}

//---------------------------------------------------------------------------
// convertors
int32_t Variant_Adaptors::asInteger() const
{
    if (isNull())
        return 0;

    switch (dataType()) {
        case VAR_BOOL:
            return m_data.getBool() ? 1 : 0;

        case VAR_INT:
            return m_data.getInteger();

        case VAR_INT64:
            return (int32_t) m_data.getInt64();

        case VAR_MONEY:
            return (int32_t) m_data.getMoneyData();

        case VAR_FLOAT:
            return (int32_t) m_data.getFloat();

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return (int32_t) string2int(m_data.getBuffer().data);

        case VAR_DATE:
        case VAR_DATE_TIME:
            // Time is in microseconds since epoch
            // - returning seconds since epoch
            return int32_t(m_data.getInt64() / 1000000);

        default:
            throw Exception("Can't convert field for that type");
    }
}

int64_t Variant_Adaptors::asInt64() const
{
    if (isNull())
        return 0;

    switch (dataType()) {
        case VAR_BOOL:
            return m_data.getBool() ? 1 : 0;

        case VAR_INT:
            return m_data.getInteger();

        case VAR_INT64:
            return m_data.getInt64();

        case VAR_MONEY:
            return (int64_t) m_data.getMoneyData();

        case VAR_FLOAT:
            return (int64_t) m_data.getFloat();

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
#ifdef _MSC_VER
            return _atoi64 (m_data.getBuffer().data);
#else
            return string2int64(m_data.getBuffer().data);
#endif

        case VAR_DATE:
        case VAR_DATE_TIME:
            // Time is in microseconds since epoch
            // - returning seconds since epoch
            return m_data.getInt64() / 1000000;

        case VAR_IMAGE_PTR:
            return int64_t(m_data.getImagePtr());

        case VAR_IMAGE_NDX:
            return int64_t(m_data.getInteger());

        default:
            throw Exception("Can't convert field for that type");
    }
}

bool Variant_Adaptors::asBool() const
{
    char ch;

    if (isNull())
        return false;

    switch (dataType()) {
        case VAR_BOOL:
            return m_data.getBool();

        case VAR_INT:
            return (m_data.getInteger() > 0);

        case VAR_INT64:
            return (m_data.getInt64() > 0);

        case VAR_MONEY:
            return (m_data.getMoneyData().quantity > 0);

        case VAR_FLOAT:
            return (m_data.getFloat() > 0);

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            ch = m_data.getBuffer().data[0];
            return (strchr("YyTt1", ch) != nullptr);

        case VAR_DATE:
        case VAR_DATE_TIME:
            return bool(m_data.getInt64() != 0);

        case VAR_IMAGE_PTR:
            return bool(m_data.getImagePtr() != nullptr);

        case VAR_IMAGE_NDX:
            return bool(m_data.getInteger() != 0);

        default:
            throw Exception("Can't convert field for that type");
    }
}

double Variant_Adaptors::asFloat() const
{
    if (isNull())
        return 0;

    switch (dataType()) {
        case VAR_BOOL:
            return m_data.getBool() ? 1 : 0;

        case VAR_INT:
            return m_data.getInteger();

        case VAR_INT64:
            return (double) m_data.getInt64();

        case VAR_MONEY:
            return (double) m_data.getMoneyData();

        case VAR_FLOAT:
            return m_data.getFloat();

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return strtod(m_data.getBuffer().data, nullptr);

        case VAR_DATE:
        case VAR_DATE_TIME:
            // Time is in microseconds since epoch
            // - returning seconds since epoch
            return m_data.getInt64() / 1000000.0;

        default:
            throw Exception("Can't convert field for that type");
    }
}

String Variant_Adaptors::asString() const
{
    if (isNull())
        return "";

    char print_buffer[64];
    int len;

    switch (dataType()) {
        case VAR_BOOL:
            if (m_data.getBool())
                return "t";
            return "f";

        case VAR_INT:
            return int2string(m_data.getInteger());

        case VAR_INT64:
            return int2string(m_data.getInt64());

        case VAR_MONEY:
            return getMoneyString(print_buffer, sizeof(print_buffer));

        case VAR_FLOAT:
            return double2string(m_data.getFloat());

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (m_data.getBuffer().data != nullptr)
                return m_data.getBuffer().data;
            else
                return "";

        case VAR_DATE:
            return DateTime(chrono::microseconds(m_data.getInt64())).dateString();

        case VAR_DATE_TIME:
            return (String) DateTime(chrono::microseconds(m_data.getInt64()));

        case VAR_IMAGE_PTR:
            len = snprintf(print_buffer, sizeof(print_buffer), "%p", m_data.getImagePtr());
            return String(print_buffer, len);

        case VAR_IMAGE_NDX:
            return int2string(m_data.getInteger());

        default:
            return "";
    }
}

String BaseVariant::getMoneyString(char* printBuffer, size_t printBufferSize) const
{
    char format[64];
    int64_t absValue;
    char* formatPtr = format;

    if (m_data.getMoneyData().quantity < 0) {
        *formatPtr = '-';
        formatPtr++;
        absValue = -m_data.getMoneyData().quantity;
    } else
        absValue = m_data.getMoneyData().quantity;

    snprintf(formatPtr, sizeof(format) - 2, "%%Ld.%%0%dLd", m_data.getMoneyData().scale);
    int64_t intValue = absValue / MoneyData::dividers[m_data.getMoneyData().scale];
    int64_t fraction = absValue % MoneyData::dividers[m_data.getMoneyData().scale];
    int len = snprintf(printBuffer, printBufferSize, format, intValue, fraction);
    return String(printBuffer, len);
}

DateTime Variant_Adaptors::asDate() const
{
    if (isNull())
        return DateTime();

    switch (dataType()) {
        case VAR_BOOL:
        case VAR_MONEY:
        case VAR_FLOAT:
            return DateTime();

        case VAR_INT:
            return DateTime(chrono::seconds(m_data.getInt64())).date();

        case VAR_INT64:
            return DateTime(chrono::microseconds(m_data.getInt64())).date();

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return DateTime(m_data.getBuffer().data).date();

        case VAR_DATE:
        case VAR_DATE_TIME:
            return DateTime(chrono::microseconds(m_data.getInt64())).date();

        default:
            throw Exception("Can't convert field for that type");
    }
}

DateTime Variant_Adaptors::asDateTime() const
{
    if (isNull())
        return DateTime();

    switch (dataType()) {
        case VAR_BOOL:
        case VAR_MONEY:
        case VAR_FLOAT:
            return DateTime();

        case VAR_INT:
            return DateTime(chrono::seconds(m_data.getInteger()));

        case VAR_INT64:
            return DateTime(chrono::microseconds(m_data.getInt64()));

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return DateTime(m_data.getBuffer().data);

        case VAR_DATE:
        case VAR_DATE_TIME:
            return DateTime(chrono::microseconds(m_data.getInt64()));

        default:
            throw Exception("Can't convert field for that type");
    }
}

const void* Variant_Adaptors::asImagePtr() const
{
    if (isNull())
        return nullptr;

    if (dataType() == VAR_IMAGE_PTR)
        return m_data.getImagePtr();

    throw Exception("Can't convert field for that type");
}

void Variant_SetMethods::setNull(VariantType vtype)
{
    if (vtype != sptk::VAR_NONE) {
        releaseBuffers();
        setDataType(vtype);
    }

    switch (dataType()) {
        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (isExternalBuffer())
                m_data.getBuffer().data = nullptr;
            else if (m_data.getBuffer().data != nullptr)
                m_data.getBuffer().data[0] = 0;

            break;

        default:
            m_data.getInt64() = 0;
            break;
    }

    m_dataType |= VAR_NULL;
}

bool BaseVariant::isNull() const
{
    return (m_dataType & VAR_NULL) != 0 || (m_dataType & VAR_TYPES) == VAR_NONE;
}

void BaseVariant::setNotNull()
{
    if ((m_dataType & VAR_NULL) != 0)
        m_dataType -= VAR_NULL;
}

String BaseVariant::typeName(VariantType type)
{
    switch (type) {
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

        default:
            return "undefined";
    }
}

VariantType BaseVariant::nameType(const char* name)
{
    static std::map<string, VariantType> nameToTypeMap;

    if (nameToTypeMap.empty()) {
        for (unsigned type = VAR_INT; type <= VAR_BOOL; type *= 2) {
            auto vtype = (VariantType) type;
            nameToTypeMap[typeName(vtype)] = vtype;
        }
    }

    if (name == nullptr || name[0] == 0)
        name = "string";

    auto itor = nameToTypeMap.find(lowerCase(name));
    if (itor == nameToTypeMap.end())
        throw Exception("Type name " + string(name) + " isn't recognized", __FILE__, __LINE__);

    return itor->second;
}

void Variant::load(const xml::Node& node)
{
    const String& ntype = node.getAttribute("type").asString();
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
            break;
    }
}

void Variant::load(const xml::Node* node)
{
    load(*node);
}

void Variant::save(xml::Node& node) const
{
    String stringValue(asString());
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
                new xml::Text(node, stringValue);
                break;

            case VAR_TEXT:
            case VAR_BUFFER:
                new xml::CDataSection(node, asString());
                break;

            default:
                break;
        }
    }
}

void Variant::save(xml::Node* node) const
{
    save(*node);
}

#if USE_GTEST

TEST(SPTK_Variant, ctors)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(2.22);
    Variant v3("Test");
    Variant v4(testDate);

    EXPECT_EQ(1, v1.asInteger());
    EXPECT_DOUBLE_EQ(2.22, v2.asFloat());
    EXPECT_STREQ("Test", v3.asString().c_str());
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v4.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
}

TEST(SPTK_Variant, assigns)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v;

    v = 1;
    EXPECT_EQ(1, v.asInteger());

    v = 2.22;
    EXPECT_DOUBLE_EQ(2.22, v.asFloat());

    v = "Test";
    EXPECT_STREQ("Test", v.asString().c_str());

    v = testDate;

    EXPECT_STREQ("2018-02-01T09:11:14.345Z", v.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());

    v.setDateTime(testDate, true);
    EXPECT_STREQ("2018-02-01T00:00:00.000Z", v.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
}

TEST(SPTK_Variant, copy)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v;
    Variant v0(12345);
    Variant v1(testDate);
    Variant v2(1.2345);
    Variant v3("Test");

    Variant v4;
    v4.setDateTime(testDate, true);

    v = v0;
    EXPECT_EQ(12345, v.asInteger());

    v = v1;
    EXPECT_STREQ("2018-02-01T09:11:14.345Z", v.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());

    v = v2;
    EXPECT_DOUBLE_EQ(1.2345, v.asFloat());

    v = v3;
    EXPECT_STREQ("Test", v.asString().c_str());

    v = v4;
    EXPECT_STREQ("2018-02-01T00:00:00.000Z", v.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
}

TEST(SPTK_Variant, toString)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(2.22);
    Variant v3("Test");
    Variant v4(testDate);

    EXPECT_STREQ("1", v1.asString().c_str());
    EXPECT_STREQ("2.22", v2.asString().c_str());
    EXPECT_STREQ("Test", v3.asString().c_str());
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v4.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
}

#endif
