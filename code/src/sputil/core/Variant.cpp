/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <iomanip>

#include <sptk5/Field.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

static constexpr int BUFFER_TYPES =
    (int) VariantDataType::VAR_STRING | (int) VariantDataType::VAR_TEXT | (int) VariantDataType::VAR_BUFFER;

//---------------------------------------------------------------------------
void BaseVariant::dataSize(size_t newDataSize)
{
    if (dataType() == VariantDataType::VAR_BUFFER && !isExternalBuffer())
    {
        m_data.get<Buffer>().bytes(newDataSize);
    }

    m_data.size(newDataSize);

    if (m_data.size() > 0)
    {
        m_data.setNull(false, m_data.type().type);
    }
}

//---------------------------------------------------------------------------
void BaseVariant::dataType(VariantDataType newDataType)
{
    m_data.type(newDataType);
}

//---------------------------------------------------------------------------
void BaseVariant::dataType(VariantType newDataType)
{
    m_data.type(newDataType);
}

//---------------------------------------------------------------------------
Variant::Variant()
{
}

//---------------------------------------------------------------------------
Variant::Variant(bool value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
Variant::Variant(int32_t value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
Variant::Variant(int64_t value, unsigned scale)
{
    if (scale > 1)
    {
        m_data.set(MoneyData(value, (uint8_t) scale));
    }
    else
    {
        m_data = value;
    }
}

//---------------------------------------------------------------------------
Variant::Variant(double value)
{
    m_data.set(value);
}

//---------------------------------------------------------------------------
Variant::Variant(const char* value)
{
    Variant::setString(value);
}

//---------------------------------------------------------------------------
Variant::Variant(const String& value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
Variant::Variant(const DateTime& value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
Variant::Variant(const uint8_t* value, size_t valueSize)
{
    Buffer buffer(value, valueSize);
    m_data = std::move(buffer);
}

//---------------------------------------------------------------------------
Variant::Variant(const Buffer& value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
Variant::~Variant() = default;

//---------------------------------------------------------------------------
void VariantAdaptors::setBool(bool value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setInteger(int32_t value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setInt64(int64_t value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setFloat(double value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setMoney(int64_t value, unsigned scale)
{
    m_data = MoneyData(value, (uint8_t) scale);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setString(const String& value)
{
    m_data = value;
}

//---------------------------------------------------------------------------

void VariantAdaptors::setBuffer(const uint8_t* value, size_t valueSize, VariantDataType type)
{
    if (((int) type & BUFFER_TYPES) == 0)
    {
        throw Exception("Invalid buffer type");
    }

    switch (type)
    {
        case VariantDataType::VAR_STRING:
            if (value == nullptr)
            {
                setNull(type);
            }
            else
            {
                m_data = String((const char*) value, valueSize);
            }
            break;

        case VariantDataType::VAR_BUFFER:
        case VariantDataType::VAR_TEXT: {
            if (value == nullptr)
            {
                Buffer buffer(valueSize);
                m_data = buffer;
            }
            else
            {
                Buffer buffer(value, valueSize);
                m_data = std::move(buffer);
            }
            dataType(type);
        }
        break;

        default:
            m_data.setNull(true, type);
            break;
    }
}

//---------------------------------------------------------------------------
void VariantAdaptors::setExternalBuffer(uint8_t* value, size_t valueSize, VariantDataType type)
{
    m_data.setExternalBuffer(value, valueSize, type);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setDateTime(const DateTime& value, bool dateOnly)
{
    if (dateOnly)
    {
        m_data.set(value.date());
        dataType(VariantDataType::VAR_DATE);
    }
    else
    {
        m_data.set(value);
    }
}

//---------------------------------------------------------------------------
void VariantAdaptors::setImagePtr(const uint8_t* value)
{
    m_data.setExternalBuffer(value, 0, VariantDataType::VAR_IMAGE_PTR);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setImageNdx(uint32_t value)
{
    const VariantType vtype {VariantDataType::VAR_IMAGE_NDX, false, false};
    dataType(vtype);
    dataSize(sizeof(value));
    m_data.set((int32_t) value);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setMoney(const MoneyData& value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setData(const BaseVariant& other)
{
    if (this == &other)
    {
        return;
    }
    m_data = other.m_data;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const Variant& other)
{
    if (this == &other)
    {
        return *this;
    }
    m_data = other.m_data;
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(Variant&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    m_data = std::move(other.m_data);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(bool value)
{
    setBool(value);
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
    m_data = value;
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const DateTime& value)
{
    setDateTime(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const uint8_t* value)
{
    setImagePtr(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const Buffer& value)
{
    m_data = value;
    return *this;
}

//---------------------------------------------------------------------------
const MoneyData& BaseVariant::getMoney() const
{
    return m_data.get<MoneyData>();
}

//---------------------------------------------------------------------------
const char* BaseVariant::getString() const
{
    if (isNull())
    {
        return nullptr;
    }

    if (isExternalBuffer())
    {
        return (const char*) (const uint8_t*) m_data;
    }

    if (m_data.type().type == VariantDataType::VAR_STRING)
    {
        return m_data.get<String>().c_str();
    }

    return m_data.get<Buffer>().c_str();
}

//---------------------------------------------------------------------------
const uint8_t* BaseVariant::getExternalBuffer() const
{
    return (const uint8_t*) m_data;
}

//---------------------------------------------------------------------------
const char* BaseVariant::getText() const
{
    return getString();
}

//---------------------------------------------------------------------------
const uint8_t* BaseVariant::getImagePtr() const
{
    return (const uint8_t*) m_data;
}

//---------------------------------------------------------------------------
uint32_t BaseVariant::getImageNdx() const
{
    return (uint32_t) m_data.get<int32_t>();
}

//---------------------------------------------------------------------------
VariantDataType BaseVariant::dataType() const
{
    return m_data.type().type;
}

//---------------------------------------------------------------------------
size_t BaseVariant::dataSize() const
{
    return m_data.size();
}

//---------------------------------------------------------------------------
size_t BaseVariant::bufferSize() const
{
    return m_data.get<Buffer>().capacity();
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
    {
        return 0;
    }

    switch (dataType())
    {
        case VariantDataType::VAR_BOOL:
            return m_data.get<bool>() ? 1 : 0;

        case VariantDataType::VAR_INT:
            return (int) m_data;

        case VariantDataType::VAR_INT64:
            return (int32_t) m_data.get<int64_t>();

        case VariantDataType::VAR_MONEY:
            return (int32_t) m_data.get<MoneyData>();

        case VariantDataType::VAR_FLOAT:
            return (int32_t) m_data.get<double>();

        case VariantDataType::VAR_STRING:
        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            return string2int(asString());

        case VariantDataType::VAR_DATE:
            return (int32_t) chrono::duration_cast<chrono::seconds>(m_data.get<DateTime>().date().sinceEpoch()).count();

        case VariantDataType::VAR_DATE_TIME:
            return (int32_t) chrono::duration_cast<chrono::seconds>(m_data.get<DateTime>().sinceEpoch()).count();

        default:
            throw Exception("Can't convert field for that type");
    }
}

int64_t VariantAdaptors::asInt64() const
{
    if (isNull())
    {
        return 0;
    }

    switch (dataType())
    {
        case VariantDataType::VAR_BOOL:
            return m_data.get<bool>() ? 1 : 0;

        case VariantDataType::VAR_INT:
            return m_data.get<int32_t>();

        case VariantDataType::VAR_INT64:
            return m_data.get<int64_t>();

        case VariantDataType::VAR_MONEY:
            return (int64_t) m_data.get<MoneyData>();

        case VariantDataType::VAR_FLOAT:
            return (int64_t) m_data.get<double>();

        case VariantDataType::VAR_STRING:
        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            return string2int64(getBufferPtr());

        case VariantDataType::VAR_DATE:
            return chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().date().sinceEpoch()).count();

        case VariantDataType::VAR_DATE_TIME:
            return chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().sinceEpoch()).count();

        case VariantDataType::VAR_IMAGE_PTR:
            return int64_t((const uint8_t*) m_data);

        case VariantDataType::VAR_IMAGE_NDX:
            return int64_t(m_data.get<int32_t>());

        default:
            throw Exception("Can't convert field for that type");
    }
}

bool VariantAdaptors::asBool() const
{
    if (isNull())
    {
        return false;
    }

    switch (dataType())
    {
        case VariantDataType::VAR_BOOL:
            return m_data.get<bool>();

        case VariantDataType::VAR_INT:
            return (m_data.get<int32_t>() > 0);

        case VariantDataType::VAR_INT64:
            return (m_data.get<int64_t>() > 0);

        case VariantDataType::VAR_MONEY:
            return (m_data.get<MoneyData>().quantity > 0);

        case VariantDataType::VAR_FLOAT:
            return (m_data.get<double>() > 0);

        case VariantDataType::VAR_STRING:
        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            return (strchr("YyTt1", asString()[0]) != nullptr);

        case VariantDataType::VAR_DATE:
        case VariantDataType::VAR_DATE_TIME:
            return !m_data.get<DateTime>().zero();

        case VariantDataType::VAR_IMAGE_PTR:
            return (const uint8_t*) m_data != nullptr;

        case VariantDataType::VAR_IMAGE_NDX:
            return m_data.get<int32_t>() != 0;

        default:
            throw Exception("Can't convert field for that type");
    }
}

double VariantAdaptors::asFloat() const
{
    double result = 0;

    if (isNull())
    {
        return result;
    }

    switch (dataType())
    {
        case VariantDataType::VAR_BOOL:
            result = m_data.get<bool>() ? 1 : 0;
            break;

        case VariantDataType::VAR_INT:
            result = m_data.get<int32_t>();
            break;

        case VariantDataType::VAR_INT64:
            result = (double) m_data.get<int64_t>();
            break;

        case VariantDataType::VAR_MONEY:
            result = (double) m_data.get<MoneyData>();
            break;

        case VariantDataType::VAR_FLOAT:
            result = m_data.get<double>();
            break;

        case VariantDataType::VAR_STRING:
        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            result = strtod(asString().c_str(), nullptr);
            break;

        case VariantDataType::VAR_DATE:
            return (double) chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().date().sinceEpoch()).count();

        case VariantDataType::VAR_DATE_TIME:
            return (double) chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().sinceEpoch()).count();

        default:
            throw Exception("Can't convert field for that type");
    }

    return result;
}

String VariantAdaptors::asString() const
{
    if (isNull())
    {
        return {};
    }

    switch (dataType())
    {
        case VariantDataType::VAR_BOOL:
            return m_data.get<bool>() ? "true" : "false";

        case VariantDataType::VAR_INT:
            return int2string(m_data.get<int32_t>());

        case VariantDataType::VAR_INT64:
            return int2string(m_data.get<int64_t>());

        case VariantDataType::VAR_MONEY:
            return moneyDataToString();

        case VariantDataType::VAR_FLOAT:
            return double2string(m_data.get<double>());

        case VariantDataType::VAR_STRING:
            return m_data.get<String>();

        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            return m_data.get<Buffer>().c_str();

        case VariantDataType::VAR_DATE:
            return m_data.get<DateTime>().date().dateString(DateTime::PF_RFC_DATE);

        case VariantDataType::VAR_DATE_TIME:
            return m_data.get<DateTime>().isoDateTimeString();

        case VariantDataType::VAR_IMAGE_PTR:
            if ((const uint8_t*) m_data != nullptr)
            {
                stringstream str;
                str << hex << (const uint8_t*) m_data;
                return str.str();
            }
            return "null";

        case VariantDataType::VAR_IMAGE_NDX:
            return int2string(m_data.get<int32_t>());

        default:
            break;
    }

    return {};
}

String BaseVariant::moneyDataToString() const
{
    stringstream output;
    const auto& moneyData = m_data.get<MoneyData>();
    auto scale = moneyData.scale;
    auto divider = MoneyData::dividers[scale];
    auto value = moneyData.quantity / divider;
    auto decimal = abs(moneyData.quantity) % divider;
    output << fixed << value << "." << setfill('0') << setw(scale) << decimal;
    return output.str();
}

DateTime VariantAdaptors::asDate() const
{
    if (isNull())
    {
        return DateTime();
    }

    switch (dataType())
    {
        case VariantDataType::VAR_BOOL:
        case VariantDataType::VAR_MONEY:
        case VariantDataType::VAR_FLOAT:
            return DateTime();

        case VariantDataType::VAR_INT:
            return DateTime(chrono::seconds(m_data.get<int32_t>())).date();

        case VariantDataType::VAR_INT64:
            return DateTime(chrono::microseconds(m_data.get<int64_t>())).date();

        case VariantDataType::VAR_STRING:
        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            return DateTime(asString().c_str()).date();

        case VariantDataType::VAR_DATE:
        case VariantDataType::VAR_DATE_TIME:
            return m_data.get<DateTime>().date();

        default:
            throw Exception("Can't convert field for that type");
    }
}

DateTime VariantAdaptors::asDateTime() const
{
    if (isNull())
    {
        return DateTime();
    }

    switch (dataType())
    {
        case VariantDataType::VAR_BOOL:
        case VariantDataType::VAR_MONEY:
        case VariantDataType::VAR_FLOAT:
            return DateTime();

        case VariantDataType::VAR_INT:
            return DateTime(chrono::seconds(m_data.get<int32_t>()));

        case VariantDataType::VAR_INT64:
            return DateTime(chrono::microseconds(m_data.get<int64_t>()));

        case VariantDataType::VAR_STRING:
        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            return DateTime(asString().c_str());

        case VariantDataType::VAR_DATE:
        case VariantDataType::VAR_DATE_TIME:
            return m_data.get<DateTime>();

        default:
            throw Exception("Can't convert field for that type");
    }
}

const uint8_t* VariantAdaptors::asImagePtr() const
{
    if (isNull())
    {
        return nullptr;
    }

    if (dataType() == VariantDataType::VAR_IMAGE_PTR)
    {
        return (const uint8_t*) m_data;
    }

    throw Exception("Can't convert field for that type");
}

void VariantAdaptors::setNull(VariantDataType vtype)
{
    m_data.setNull(true, vtype);
}

const char* VariantAdaptors::getBufferPtr() const
{
    if (isExternalBuffer())
    {
        return (const char*) (const uint8_t*) m_data;
    }
    return m_data.get<Buffer>().c_str();
}

bool BaseVariant::isNull() const
{
    return m_data.type().isNull || m_data.type().type == VariantDataType::VAR_NONE;
}

String BaseVariant::typeName(VariantDataType type)
{
    switch (type)
    {
        case VariantDataType::VAR_BOOL:
            return "bool";

        case VariantDataType::VAR_INT:
            return "int";

        case VariantDataType::VAR_INT64:
            return "int64";

        case VariantDataType::VAR_FLOAT:
            return "double";

        case VariantDataType::VAR_MONEY:
            return "money";

        case VariantDataType::VAR_STRING:
            return "string";

        case VariantDataType::VAR_TEXT:
            return "text";

        case VariantDataType::VAR_BUFFER:
            return "blob";

        case VariantDataType::VAR_DATE:
            return "date";

        case VariantDataType::VAR_DATE_TIME:
            return "datetime";

        case VariantDataType::VAR_IMAGE_PTR:
            return "imageptr";

        case VariantDataType::VAR_IMAGE_NDX:
            return "imagendx";

        default:
            return "undefined";
    }
}

VariantDataType BaseVariant::nameType(const char* name)
{
    static const std::map<string, VariantDataType, less<>> nameToTypeMap {
        {typeName(VariantDataType::VAR_NONE), VariantDataType::VAR_NONE},
        {typeName(VariantDataType::VAR_INT), VariantDataType::VAR_INT},
        {typeName(VariantDataType::VAR_FLOAT), VariantDataType::VAR_FLOAT},
        {typeName(VariantDataType::VAR_MONEY), VariantDataType::VAR_MONEY},
        {typeName(VariantDataType::VAR_STRING), VariantDataType::VAR_STRING},
        {typeName(VariantDataType::VAR_TEXT), VariantDataType::VAR_TEXT},
        {typeName(VariantDataType::VAR_BUFFER), VariantDataType::VAR_BUFFER},
        {typeName(VariantDataType::VAR_DATE), VariantDataType::VAR_DATE},
        {typeName(VariantDataType::VAR_DATE_TIME), VariantDataType::VAR_DATE_TIME},
        {typeName(VariantDataType::VAR_IMAGE_PTR), VariantDataType::VAR_IMAGE_PTR},
        {typeName(VariantDataType::VAR_IMAGE_NDX), VariantDataType::VAR_IMAGE_NDX},
        {typeName(VariantDataType::VAR_INT64), VariantDataType::VAR_INT64},
        {typeName(VariantDataType::VAR_BOOL), VariantDataType::VAR_BOOL},
    };

    if (name == nullptr || name[0] == 0)
    {
        name = "string";
    }

    auto itor = nameToTypeMap.find(lowerCase(name));
    if (itor == nameToTypeMap.end())
    {
        throw Exception("Type name " + string(name) + " isn't recognized", __FILE__, __LINE__);
    }

    return itor->second;
}

void Variant::load(const SNode& element)
{
    switch (element->type())
    {
        case Node::Type::Number:
            *this = element->getNumber();
            break;
        case Node::Type::Boolean:
            *this = element->getBoolean();
            break;
        case Node::Type::Null:
            setNull();
            break;
        case Node::Type::Text:
            *this = element->getString();
            break;
        default:
            break;
    }
}

void Variant::save(const SNode& node) const
{
    const String stringValue(asString());

    node->clear();
    node->attributes().set("type", typeName(dataType()));
    node->set(*this);

    if (!stringValue.empty())
    {
        switch (dataType())
        {
            case VariantDataType::VAR_BOOL:
                node->type(Node::Type::Boolean);
                break;
            case VariantDataType::VAR_INT:
            case VariantDataType::VAR_INT64:
            case VariantDataType::VAR_FLOAT:
            case VariantDataType::VAR_IMAGE_NDX:
                node->type(Node::Type::Number);
                break;
            case VariantDataType::VAR_MONEY:
            case VariantDataType::VAR_STRING:
            case VariantDataType::VAR_DATE:
            case VariantDataType::VAR_DATE_TIME:
                node->type(Node::Type::Text);
                break;

            case VariantDataType::VAR_TEXT:
            case VariantDataType::VAR_BUFFER:
                node->type(Node::Type::CData);
                break;

            default:
                break;
        }
    }
}
