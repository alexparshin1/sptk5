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
│   You shouldint have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <cmath>
#include <iomanip>

#include <sptk5/Field.h>
#include <sptk5/json/JsonDocument.h>

using namespace std;
using namespace sptk;

constexpr int BUFFER_TYPES = VAR_STRING|VAR_TEXT|VAR_BUFFER;

int64_t MoneyData::dividers[16] = {1, 10, 100, 1000, 10000, 100000, 1000000L, 10000000L, 100000000LL, 1000000000LL,
                                   10000000000LL, 100000000000LL,
                                   1000000000000LL, 10000000000000LL, 100000000000000LL, 1000000000000000LL};

MoneyData::operator double() const
{
    return double(quantity) / double(dividers[scale]);
}

MoneyData::operator int64_t() const
{
    return quantity / dividers[scale];
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
    auto& buffer = m_data.getBuffer();
    if ((m_data.type() & (VAR_STRING | VAR_BUFFER | VAR_TEXT)) != 0 &&
        buffer.data != nullptr) {
        if (!isExternalBuffer())
            delete [] buffer.data;

        buffer.data = nullptr;
        buffer.size = 0;
    }
}

//---------------------------------------------------------------------------
void BaseVariant::dataSize(size_t ds)
{
    m_data.size(ds);
    if (m_data.size() > 0)
        m_data.type(uint16_t(m_data.type() & (VAR_TYPES | VAR_EXTERNAL_BUFFER)));
}

//---------------------------------------------------------------------------
void BaseVariant::dataType(uint32_t dt)
{
    m_data.type((uint16_t) dt);
}

//---------------------------------------------------------------------------
Variant::Variant()
{
    m_data.type(VAR_NONE | VAR_NULL);
    m_data.getInt64() = 0;
    m_data.getBuffer().size = 0;
}

//---------------------------------------------------------------------------
Variant::Variant(int32_t value)
{
    m_data.type(VAR_INT);
    m_data.getInteger() = value;
}

//---------------------------------------------------------------------------
Variant::Variant(int64_t value, unsigned scale)
{
    if (scale > 1) {
        m_data.type(VAR_MONEY);
        m_data.getMoneyData().quantity = value;
        m_data.getMoneyData().scale = (uint8_t) scale;
    } else {
        m_data.type(VAR_INT64);
        m_data.getInt64() = value;
    }
}

//---------------------------------------------------------------------------
Variant::Variant(double value)
{
    m_data.type(VAR_FLOAT);
    m_data.getFloat() = value;
}

//---------------------------------------------------------------------------
Variant::Variant(const char* value)
{
    Variant::setString(value);
}

//---------------------------------------------------------------------------
Variant::Variant(const String& v)
{
    Variant::setBuffer(v.c_str(), v.length(), VAR_STRING);
}

//---------------------------------------------------------------------------
Variant::Variant(const DateTime& v)
{
    m_data.type(VAR_DATE_TIME);
    DateTime::duration sinceEpoch = v.timePoint().time_since_epoch();
    m_data.getInt64() = chrono::duration_cast<chrono::microseconds>(sinceEpoch).count();
}

//---------------------------------------------------------------------------
Variant::Variant(const void* value, size_t sz)
{
    Variant::setBuffer(value, sz, VAR_BUFFER);
}

//---------------------------------------------------------------------------
Variant::Variant(const Buffer& value)
{
    m_data.type(VAR_NONE);
    Variant::setBuffer(value.data(), value.bytes(), VAR_BUFFER);
}

//---------------------------------------------------------------------------
Variant::Variant(const Variant& value)
{
    m_data.type(VAR_NONE);
    setData(value);
}

//---------------------------------------------------------------------------
Variant::Variant(Variant&& other) noexcept
{
    m_data = move(other.m_data);
}

//---------------------------------------------------------------------------
Variant::~Variant()
{
    releaseBuffers();
}

//---------------------------------------------------------------------------
void VariantAdaptors::setBool(bool value)
{
    if (m_data.type() != VAR_BOOL) {
        releaseBuffers();
        m_data.type(VAR_BOOL);
        m_data.size(sizeof(value));
    }

    m_data.getBool() = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setInteger(int32_t value)
{
    if (m_data.type() != VAR_INT) {
        releaseBuffers();
        m_data.type(VAR_INT);
        m_data.size(sizeof(value));
    }

    m_data.getInteger() = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setInt64(int64_t value)
{
    if (m_data.type() != VAR_INT64) {
        releaseBuffers();
        m_data.type(VAR_INT64);
        m_data.size(sizeof(value));
    }

    m_data.getInt64() = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setFloat(double value)
{
    if (m_data.type() != VAR_FLOAT) {
        releaseBuffers();
        m_data.type(VAR_FLOAT);
        m_data.size(sizeof(value));
    }

    m_data.getFloat() = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setMoney(int64_t value, unsigned scale)
{
    if (m_data.type() != VAR_MONEY) {
        releaseBuffers();
        m_data.type(VAR_MONEY);
        m_data.size(sizeof(MoneyData));
    }

    m_data.getMoneyData().quantity = value;
    m_data.getMoneyData().scale = (uint8_t) scale;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setString(const String& value)
{
    setBuffer(value.c_str(), value.length(), VAR_STRING);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setBuffer(const void* value, size_t sz, VariantType type)
{
    if ((type & BUFFER_TYPES) == 0)
        throw Exception("Invalid buffer type");

    releaseBuffers();

    if (value != nullptr || sz != 0) {
        auto& buffer = m_data.getBuffer();

        buffer.size = sz + 1;
        dataSize(sz);
        auto* data = new char[sz + 1];
        if (data != nullptr && value != nullptr) {
            memcpy(data, value, sz);
            data[sz] = 0;
        }
        buffer.data = data;

        dataType(type);

    } else
        setNull(type);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setExternalBuffer(void* value, size_t sz, VariantType type)
{
    if ((type & BUFFER_TYPES) == 0)
        throw Exception("Invalid buffer type");

    releaseBuffers();

    if (value != nullptr || sz != 0) {
        auto& buffer = m_data.getBuffer();
        buffer.size = sz;
        buffer.data = (char*) value;
        dataSize(sz);
        dataType(type | VAR_EXTERNAL_BUFFER);
    } else
        setNull(type);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setDateTime(DateTime value, bool dateOnly)
{
    if ((m_data.type() & (VAR_DATE|VAR_DATE_TIME)) == 0) {
        releaseBuffers();
        dataSize(sizeof(value));
    }

    DateTime::duration sinceEpoch = value.timePoint().time_since_epoch();
    if (dateOnly) {
        m_data.type(VAR_DATE);
        int64_t days = chrono::duration_cast<chrono::hours>(sinceEpoch).count() / 24;
        m_data.getInt64() = days * 86400 * 1000000;
    } else {
        m_data.type(VAR_DATE_TIME);
        m_data.getInt64() = chrono::duration_cast<chrono::microseconds>(sinceEpoch).count();
    }
}

//---------------------------------------------------------------------------
void VariantAdaptors::setImagePtr(const void* value)
{
    if (m_data.type() != VAR_IMAGE_PTR) {
        releaseBuffers();
        dataType(VAR_IMAGE_PTR);
        dataSize(sizeof(value));
    }

    m_data.setImagePtr(value);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setImageNdx(uint32_t value)
{
    if (dataType() != VAR_IMAGE_NDX) {
        releaseBuffers();
        dataType(VAR_IMAGE_NDX);
    }

    dataSize(sizeof(value));
    m_data.getInteger() = (int32_t) value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setMoney(const MoneyData& value)
{
    if (dataType() != VAR_MONEY) {
        releaseBuffers();
        dataType(VAR_MONEY);
    }

    dataSize(sizeof(value));
    m_data.getMoneyData() = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setData(const BaseVariant& C)
{
    if (C.isNull()) {
        setNull(C.dataType());
        return;
    }
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
            setBuffer(C.getString(), C.dataSize(), VAR_STRING);
            break;

        case VAR_TEXT:
            setBuffer(C.getText(), strlen(C.getText()), VAR_TEXT);
            break;

        case VAR_BUFFER:
            setBuffer(C.getBuffer(), C.dataSize(), VAR_BUFFER);
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

    m_data.type(C.m_data.type());
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const Variant& other)
{
    if (this == &other)
        return *this;
    setData(other);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(Variant&& other) noexcept
{
    if (this == &other)
        return *this;
    m_data = move(other.m_data);
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
    setBuffer(value.c_str(), value.length(), VAR_STRING);
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
    setBuffer(value.data(), value.bytes(), VAR_BUFFER);
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
char* BaseVariant::getBuffer() const
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
VariantType BaseVariant::dataType() const
{
    return (VariantType) (m_data.type() & VAR_TYPES);
}

//---------------------------------------------------------------------------
size_t BaseVariant::dataSize() const
{
    return m_data.size();
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
int32_t VariantAdaptors::asInteger() const
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
            return string2int(m_data.getBuffer().data);

        case VAR_DATE:
        case VAR_DATE_TIME:
            // Time is in microseconds since epoch
            // - returning seconds since epoch
            return int32_t(m_data.getInt64() / 1000000);

        default:
            throw Exception("Can't convert field for that type");
    }
}

int64_t VariantAdaptors::asInt64() const
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

bool VariantAdaptors::asBool() const
{
    char ch = 0;

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

double VariantAdaptors::asFloat() const
{
    double result = 0;

    if (isNull())
        return result;

    switch (dataType()) {
        case VAR_BOOL:
            result = m_data.getBool() ? 1 : 0;
            break;

        case VAR_INT:
            result = m_data.getInteger();
            break;

        case VAR_INT64:
            result = (double) m_data.getInt64();
            break;

        case VAR_MONEY:
            result = (double) m_data.getMoneyData();
            break;

        case VAR_FLOAT:
            result = m_data.getFloat();
            break;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            result = strtod(m_data.getBuffer().data, nullptr);
            break;

        case VAR_DATE:
        case VAR_DATE_TIME:
            // Time is in microseconds since epoch
            // - returning seconds since epoch
            result = double(m_data.getInt64()) / 1000000.0;
            break;

        default:
            throw Exception("Can't convert field for that type");
    }

    return result;
}

String VariantAdaptors::asString() const
{
    if (isNull())
        return "";

    String output;

    switch (dataType()) {
        case VAR_BOOL:
            output = m_data.getBool() ? "true" : "false";
            break;

        case VAR_INT:
            output = int2string(m_data.getInteger());
            break;

        case VAR_INT64:
            output = int2string(m_data.getInt64());
            break;

        case VAR_MONEY:
            output = moneyDataToString();
            break;

        case VAR_FLOAT:
            output = double2string(m_data.getFloat());
            break;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            output = m_data.getBuffer().data != nullptr ? m_data.getBuffer().data : "";
            break;

        case VAR_DATE:
            output = DateTime(chrono::microseconds(m_data.getInt64())).dateString();
            break;

        case VAR_DATE_TIME:
            output = (String) DateTime(chrono::microseconds(m_data.getInt64()));
            break;

        case VAR_IMAGE_PTR:
            if (m_data.getImagePtr() != nullptr) {
                stringstream str;
                str << hex << m_data.getImagePtr();
                output = str.str();
            } else
                output = "null";
            break;

        case VAR_IMAGE_NDX:
            output = int2string(m_data.getInteger());
            break;

        default:
            break;
    }

    return output;
}

String BaseVariant::moneyDataToString() const
{
    stringstream output;
    auto scale = m_data.getMoneyData().scale;
    auto divider = MoneyData::dividers[scale];
    auto value = m_data.getMoneyData().quantity / divider;
    auto decimal = abs(m_data.getMoneyData().quantity) % divider;
    output << fixed << value << "." << setfill('0') << setw(scale) << decimal;
    return output.str();
}

DateTime VariantAdaptors::asDate() const
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

DateTime VariantAdaptors::asDateTime() const
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

const void* VariantAdaptors::asImagePtr() const
{
    if (isNull())
        return nullptr;

    if (dataType() == VAR_IMAGE_PTR)
        return m_data.getImagePtr();

    throw Exception("Can't convert field for that type");
}

void VariantAdaptors::setNull(VariantType vtype)
{
    releaseBuffers();
    dataType(vtype | VAR_NULL);

    switch (dataType()) {
        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            m_data.getBuffer().data = nullptr;
            m_data.getBuffer().size = 0;
            break;

        default:
            m_data.getInt64() = 0;
            break;
    }
}

bool BaseVariant::isNull() const
{
    return (m_data.type() & VAR_NULL) != 0 || (m_data.type() & VAR_TYPES) == VAR_NONE;
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

void Variant::load(const xml::Node* node)
{
    const String& ntype = node->getAttribute("type").asString();
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
        case VAR_TEXT:
        case VAR_BUFFER:
            *this = node->text();
            break;

        default:
            break;
    }
}

void Variant::load(const json::Element* element)
{
    switch (element->type()) {
        case json::JDT_NUMBER:
            *this = element->getNumber();
            break;
        case json::JDT_BOOLEAN:
            *this = element->getBoolean();
            break;
        case json::JDT_NULL:
            setNull();
            break;
        case json::JDT_STRING:
            *this = element->getString();
            break;
        default:
            break;
    }
}

void Variant::save(xml::Node* node) const
{
    String stringValue(asString());

    node->clear();
    node->setAttribute("type", typeName(dataType()));

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
                new xml::Text(*node, stringValue);
                break;

            case VAR_TEXT:
            case VAR_BUFFER:
                new xml::CDataSection(*node, asString());
                break;

            default:
                break;
        }
    }
}

void Variant::save(json::Element* node) const
{
    *node = asString();
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

TEST(SPTK_Variant, copy_ctors)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(2.22);
    Variant v3("Test");
    Variant v4(testDate);
    Variant v5;
    Variant v6(Buffer("A test", 6));
    Variant v7(int64_t(1));

    v5.setNull(VAR_STRING);

    Variant v1c(v1);
    Variant v2c(v2);
    Variant v3c(v3);
    Variant v4c(v4);
    Variant v5c(v5);
    Variant v6c(v6);
    Variant v7c(v7);

    EXPECT_EQ(1, v1c.asInteger());
    EXPECT_DOUBLE_EQ(2.22, v2c.asFloat());
    EXPECT_STREQ("Test", v3c.asString().c_str());
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v4c.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
    EXPECT_EQ(v5c.isNull(), true);
    EXPECT_EQ(v5c.dataType(), VAR_STRING);
    EXPECT_STREQ(v6c.asString().c_str(), "A test");
    EXPECT_EQ(v6c.dataType(), VAR_BUFFER);
    EXPECT_EQ(int64_t(1), v1c.asInt64());
}

TEST(SPTK_Variant, move_ctors)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v1(1);
    Variant v2(2.22);
    Variant v3("Test");
    Variant v4(testDate);
    Variant v5;
    Variant v6(Buffer("A test", 6));

    v5.setNull(VAR_STRING);

    Variant v1m(move(v1));
    Variant v2m(move(v2));
    Variant v3m(move(v3));
    Variant v4m(move(v4));
    Variant v5m(move(v5));
    Variant v6m(move(v6));

    EXPECT_EQ(VAR_NONE, v1.dataType());
    EXPECT_EQ(v1.isNull(), true);
    EXPECT_EQ(1, v1m.asInteger());

    EXPECT_EQ(VAR_NONE, v2.dataType());
    EXPECT_EQ(v2.isNull(), true);
    EXPECT_DOUBLE_EQ(2.22, v2m.asFloat());

    EXPECT_EQ(VAR_NONE, v3.dataType());
    EXPECT_EQ(v3.isNull(), true);
    EXPECT_STREQ("Test", v3m.asString().c_str());

    EXPECT_EQ(VAR_NONE, v4.dataType());
    EXPECT_EQ(v4.isNull(), true);
    EXPECT_STREQ("2018-02-01T09:11:14.345Z",
                 v4m.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());

    EXPECT_EQ(VAR_NONE, v5.dataType());
    EXPECT_EQ(v5.isNull(), true);
    EXPECT_EQ(v5m.isNull(), true);
    EXPECT_EQ(v5m.dataType(), VAR_STRING);

    EXPECT_EQ(VAR_NONE, v6.dataType());
    EXPECT_EQ(v6.isNull(), true);
    EXPECT_STREQ(v6m.asString().c_str(), "A test");
    EXPECT_EQ(v6m.dataType(), VAR_BUFFER);
}

TEST(SPTK_Variant, assigns)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v;
    Variant v1;

    v = 1;
    EXPECT_EQ(1, v.asInteger());

    v = 2.22;
    EXPECT_DOUBLE_EQ(2.22, v.asFloat());

    v = "Test";
    EXPECT_STREQ("Test", v.asString().c_str());

    v1 = v;
    EXPECT_STREQ("Test", v1.asString().c_str());

    v = testDate;

    EXPECT_STREQ("2018-02-01T09:11:14.345Z", v.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());

    v.setDateTime(testDate, true);
    EXPECT_STREQ("2018-02-01T00:00:00.000Z", v.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
}

TEST(SPTK_Variant, move_assigns)
{
    DateTime testDate("2018-02-01 09:11:14.345Z");

    Variant v;
    Variant vm;

    v = 1;
    vm = move(v);
    EXPECT_EQ(1, vm.asInteger());
    EXPECT_EQ(true, v.isNull());
    EXPECT_EQ(VAR_NONE, v.dataType());

    v = 2.22;
    vm = move(v);
    EXPECT_DOUBLE_EQ(2.22, vm.asFloat());
    EXPECT_EQ(true, v.isNull());
    EXPECT_EQ(VAR_NONE, v.dataType());

    v = "Test";
    vm = move(v);
    EXPECT_STREQ("Test", vm.asString().c_str());
    EXPECT_EQ(true, v.isNull());
    EXPECT_EQ(VAR_NONE, v.dataType());

    v = testDate;
    vm = move(v);
    EXPECT_STREQ("2018-02-01T09:11:14.345Z", vm.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
    EXPECT_EQ(true, v.isNull());
    EXPECT_EQ(VAR_NONE, v.dataType());

    v.setDateTime(testDate, true);
    vm = move(v);
    EXPECT_STREQ("2018-02-01T00:00:00.000Z", vm.asDateTime().isoDateTimeString(DateTime::PA_MILLISECONDS, true).c_str());
    EXPECT_EQ(true, v.isNull());
    EXPECT_EQ(VAR_NONE, v.dataType());
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
    Variant v5;
	DateTime dt;
	String dtStr;

    v5.setDateTime(testDate, true);

    EXPECT_STREQ("1", v1.asString().c_str());
    EXPECT_STREQ("2.22", v2.asString().c_str());
    EXPECT_STREQ("Test", v3.asString().c_str());
    
	dt = v4.asDateTime();
	dtStr = dt.dateString() + " " + dt.timeString();
	EXPECT_STREQ(dtStr.c_str(), v4.asString().c_str());

	dt = v5.asDateTime();
	dtStr = dt.dateString();
    EXPECT_STREQ(dtStr.c_str(), v5.asString().c_str());
}

TEST(SPTK_Variant, money)
{
    Variant money(10001234,4);
    EXPECT_DOUBLE_EQ((double) money.getMoney(), 1000.1234);
    EXPECT_EQ((int) money.getMoney(), 1000);
    EXPECT_EQ((int64_t) money.getMoney(), 1000);
    EXPECT_TRUE((bool) money.getMoney());
    EXPECT_STREQ(money.asString().c_str(), "1000.1234");

    money.setMoney(200055,2);
    EXPECT_STREQ(money.asString().c_str(), "2000.55");
    EXPECT_EQ(VAR_MONEY, Variant::nameType("money"));
    EXPECT_STREQ("money", Variant::typeName(VAR_MONEY).c_str());

    MoneyData value { 12345678, 4 };
    money.setMoney(value);
    EXPECT_DOUBLE_EQ((double) money, 1234.5678);
    EXPECT_TRUE(money.dataType() == VAR_MONEY);

    Variant s("test", 4);
    s.setMoney(1234567, 4);
    EXPECT_DOUBLE_EQ((double) s.getMoney(), 123.4567);
}

TEST(SPTK_Variant, externalBuffer)
{
    Buffer externalBuffer("External Data");
    Variant v;
    v.setExternalBuffer((void*)externalBuffer.c_str(), externalBuffer.length());
    EXPECT_STREQ(externalBuffer.c_str(), v.asString().c_str());
    externalBuffer[1] = 'X';
    EXPECT_STREQ(externalBuffer.c_str(), v.asString().c_str());
}

TEST(SPTK_Variant, json)
{
    constexpr int testInteger1 = 12345;
    const char* json = R"({ "value": 12345 })";
    json::Document document;
    document.load(json);
    auto& node = document.root()["value"];

    Variant v;
    v.load(&node);
    EXPECT_EQ(v.asInteger(), testInteger1);

    constexpr int testInteger2 = 123456;
    v = testInteger2;
    v.save(&node);
    EXPECT_STREQ(node.getString().c_str(), "123456");
}

TEST(SPTK_Variant, bool)
{
    Variant v("test", 4);
    v.setBool(true);
    EXPECT_TRUE(v);
}

TEST(SPTK_Variant, xml)
{
    const char* xml = "<value>12345</value>";
    xml::Document document;
    document.load(xml, false);
    auto* node = document.findFirst("value");

    Variant v;
    v.load(node);
    EXPECT_EQ(v.asInteger(), 12345);

    v = 123456;
    v.save(node);
    EXPECT_STREQ(node->text().c_str(), "123456");
}
#endif
