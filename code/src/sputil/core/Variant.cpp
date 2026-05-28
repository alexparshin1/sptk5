/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
#include <format>

#include <sptk5/Field.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

using enum VariantDataType;

static constexpr int BUFFER_TYPES =
    static_cast<int>(VAR_STRING) | static_cast<int>(VAR_TEXT) | static_cast<int>(VAR_BUFFER);

//---------------------------------------------------------------------------
void BaseVariant::dataSize(const size_t newDataSize)
{
    if (dataType() == VAR_BUFFER && !isExternalBuffer())
    {
        m_data.get<Buffer>().bytes(newDataSize);
    }

    m_data.setSize(newDataSize);

    if (m_data.size() > 0)
    {
        m_data.setNull(false, m_data.type().type);
    }
}

//---------------------------------------------------------------------------
void BaseVariant::dataType(const VariantDataType newDataType)
{
    m_data.type(newDataType);
}

//---------------------------------------------------------------------------
void BaseVariant::dataType(const VariantType newDataType)
{
    m_data.type(newDataType);
}

//---------------------------------------------------------------------------
Variant::Variant(const bool value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
Variant::Variant(const int32_t value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
Variant::Variant(const int64_t value, const unsigned scale)
{
    if (scale > 0)
    {
        m_data.set(MoneyData(value, static_cast<uint8_t>(scale)));
    }
    else
    {
        m_data = value;
    }
}

//---------------------------------------------------------------------------
Variant::Variant(const double value)
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
Variant::Variant(const DateTime& dateTime)
{
    m_data = dateTime;
}

//---------------------------------------------------------------------------
Variant::Variant(const uint8_t* value, const size_t valueSize)
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
void VariantAdaptors::setBool(const bool value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setInteger(const int32_t value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setInt64(const int64_t value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setFloat(const double value)
{
    m_data = value;
}

//---------------------------------------------------------------------------
void VariantAdaptors::setMoney(const int64_t value, const unsigned scale)
{
    m_data = MoneyData(value, static_cast<uint8_t>(scale));
}

//---------------------------------------------------------------------------
void VariantAdaptors::setString(const String& value)
{
    m_data = value;
}

//---------------------------------------------------------------------------

void VariantAdaptors::setBuffer(const uint8_t* value, const size_t valueSize, VariantDataType type)
{
    if ((static_cast<int>(type) & BUFFER_TYPES) == 0)
    {
        throw Exception("Invalid buffer type");
    }

    switch (type)
    {
        case VAR_STRING:
            if (value == nullptr)
            {
                setNull(type);
            }
            else
            {
                m_data = String(reinterpret_cast<const char*>(value), valueSize);
            }
            break;

        case VAR_BUFFER:
        case VAR_TEXT: {
            if (value == nullptr)
            {
                Buffer buffer(valueSize);
                m_data = std::move(buffer);
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
void VariantAdaptors::setExternalBuffer(uint8_t* value, const size_t valueSize, const VariantDataType type)
{
    m_data.setExternalBuffer(value, valueSize, type);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setDateTime(const DateTime& value, const bool dateOnly)
{
    if (dateOnly)
    {
        m_data.set(value.date());
        dataType(VAR_DATE);
    }
    else
    {
        m_data.set(value);
    }
}

//---------------------------------------------------------------------------
void VariantAdaptors::setImagePtr(const uint8_t* value)
{
    m_data.setExternalBuffer(value, 0, VAR_IMAGE_PTR);
}

//---------------------------------------------------------------------------
void VariantAdaptors::setImageNdx(const uint32_t value)
{
    constexpr VariantType variantType {VAR_IMAGE_NDX, false, false};
    dataType(variantType);
    dataSize(sizeof(value));
    m_data.set(static_cast<int32_t>(value));
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
Variant& Variant::operator=(const bool value)
{
    setBool(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const int32_t value)
{
    setInteger(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const int64_t value)
{
    setInt64(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const double value)
{
    setFloat(value);
    return *this;
}

//---------------------------------------------------------------------------
Variant& Variant::operator=(const MoneyData& value)
{
    setMoney(value);
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
        return reinterpret_cast<const char*>(static_cast<const uint8_t*>(m_data));
    }

    if (m_data.type().type == VAR_STRING)
    {
        return m_data.get<String>().c_str();
    }

    if (m_data.type().type == VAR_BUFFER || m_data.type().type == VAR_TEXT)
    {
        return m_data.get<Buffer>().c_str();
    }

    return "";
}

//---------------------------------------------------------------------------
const uint8_t* BaseVariant::getExternalBuffer() const
{
    return static_cast<const uint8_t*>(m_data);
}

//---------------------------------------------------------------------------
const char* BaseVariant::getText() const
{
    return getString();
}

//---------------------------------------------------------------------------
const uint8_t* BaseVariant::getImagePtr() const
{
    return static_cast<const uint8_t*>(m_data);
}

//---------------------------------------------------------------------------
uint32_t BaseVariant::getImageNdx() const
{
    return static_cast<uint32_t>(m_data.get<int32_t>());
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
    if (m_data.type().type == VAR_BUFFER || m_data.type().type == VAR_TEXT)
    {
        return m_data.get<Buffer>().capacity();
    }
    return 0;
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
    return static_cast<uint64_t>(asInt64());
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
Variant::operator Buffer() const
{
    return asBuffer();
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
        case VAR_BOOL:
            return m_data.get<bool>() ? 1 : 0;

        case VAR_INT:
            return static_cast<int>(m_data);

        case VAR_INT64:
            return static_cast<int32_t>(m_data.get<int64_t>());

        case VAR_MONEY:
            return static_cast<int32_t>(m_data.get<MoneyData>());

        case VAR_FLOAT:
            return static_cast<int32_t>(m_data.get<double>());

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return string2int(asString());

        case VAR_DATE:
            return static_cast<int32_t>(chrono::duration_cast<chrono::seconds>(m_data.get<DateTime>().date().sinceEpoch()).count());

        case VAR_DATE_TIME:
            return static_cast<int32_t>(chrono::duration_cast<chrono::seconds>(m_data.get<DateTime>().sinceEpoch()).count());

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
        case VAR_BOOL:
            return m_data.get<bool>() ? 1 : 0;

        case VAR_INT:
            return m_data.get<int32_t>();

        case VAR_INT64:
            return m_data.get<int64_t>();

        case VAR_MONEY:
            return static_cast<int64_t>(m_data.get<MoneyData>());

        case VAR_FLOAT:
            return static_cast<int64_t>(m_data.get<double>());

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return string2int64(string(getBufferPtr()));

        case VAR_DATE:
            return chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().date().sinceEpoch()).count();

        case VAR_DATE_TIME:
            return chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().sinceEpoch()).count();

        case VAR_IMAGE_PTR:
            return reinterpret_cast<int64_t>(static_cast<const uint8_t*>(m_data));

        case VAR_IMAGE_NDX:
            return m_data.get<int32_t>();

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
        case VAR_BOOL:
            return m_data.get<bool>();

        case VAR_INT:
            return (m_data.get<int32_t>() > 0);

        case VAR_INT64:
            return (m_data.get<int64_t>() > 0);

        case VAR_MONEY:
            return (m_data.get<MoneyData>().quantity() > 0);

        case VAR_FLOAT:
            return (m_data.get<double>() > 0);

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (m_data.size() > 0)
            {
                return (strchr("YyTt1", getString()[0]) != nullptr);
            }
            return false;

        case VAR_DATE:
        case VAR_DATE_TIME:
            return !m_data.get<DateTime>().zero();

        case VAR_IMAGE_PTR:
            return static_cast<const uint8_t*>(m_data) != nullptr;

        case VAR_IMAGE_NDX:
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
        case VAR_BOOL:
            result = m_data.get<bool>() ? 1 : 0;
            break;

        case VAR_INT:
            result = m_data.get<int32_t>();
            break;

        case VAR_INT64:
            result = static_cast<double>(m_data.get<int64_t>());
            break;

        case VAR_MONEY:
            result = static_cast<double>(m_data.get<MoneyData>());
            break;

        case VAR_FLOAT:
            result = m_data.get<double>();
            break;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            result = strtod(getString(), nullptr);
            break;

        case VAR_DATE:
            result = static_cast<double>(chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().date().sinceEpoch()).count());
            break;

        case VAR_DATE_TIME:
            result = static_cast<double>(chrono::duration_cast<chrono::microseconds>(m_data.get<DateTime>().sinceEpoch()).count());
            break;

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
        case VAR_BOOL:
            return m_data.get<bool>() ? "true" : "false";

        case VAR_INT:
            return to_string(m_data.get<int32_t>());

        case VAR_INT64:
            return to_string(m_data.get<int64_t>());

        case VAR_MONEY:
            return moneyDataToString();

        case VAR_FLOAT:
            return double2string(m_data.get<double>());

        case VAR_STRING:
            if (isExternalBuffer())
            {
                const auto* ptr = static_cast<const uint8_t*>(m_data);
                if (ptr != nullptr)
                {
                    return {reinterpret_cast<const char*>(ptr), m_data.size()};
                }
                return {""};
            }
            return m_data.get<String>();

        case VAR_TEXT:
        case VAR_BUFFER:
            if (isExternalBuffer())
            {
                const auto* ptr = static_cast<const uint8_t*>(m_data);
                if (ptr != nullptr)
                {
                    return {reinterpret_cast<const char*>(ptr), m_data.size()};
                }
                return {""};
            }
            return m_data.get<Buffer>().c_str();

        case VAR_DATE:
            return m_data.get<DateTime>().date().dateString(DateTime::PF_RFC_DATE);

        case VAR_DATE_TIME:
            return m_data.get<DateTime>().isoDateTimeString();

        case VAR_IMAGE_PTR: {
            const auto* ptr = static_cast<const void*>(static_cast<const uint8_t*>(m_data));
            return ptr ? std::format("{:p}", ptr) : "null";
        }

        case VAR_IMAGE_NDX:
            return to_string(m_data.get<int32_t>());

        default:
            break;
    }

    return {};
}

Buffer VariantAdaptors::asBuffer() const
{
    if (isNull())
    {
        return Buffer();
    }

    switch (dataType())
    {
        case VAR_BOOL:
            return Buffer(m_data.get<bool>() ? "true" : "false");

        case VAR_INT:
            return Buffer(to_string(m_data.get<int32_t>()));

        case VAR_INT64:
            return Buffer(to_string(m_data.get<int64_t>()));

        case VAR_MONEY:
            return Buffer(moneyDataToString());

        case VAR_FLOAT:
            return Buffer(double2string(m_data.get<double>()));

        case VAR_STRING:
            if (m_data.type().isExternalBuffer)
            {
                return Buffer(static_cast<const uint8_t*>(m_data), m_data.size());
            }
            return Buffer(m_data.get<String>());

        case VAR_TEXT:
        case VAR_BUFFER:
            if (m_data.type().isExternalBuffer)
            {
                return Buffer(static_cast<const uint8_t*>(m_data), m_data.size());
            }
            return m_data.get<Buffer>();

        case VAR_DATE:
            return Buffer(m_data.get<DateTime>().date().dateString(DateTime::PF_RFC_DATE));

        case VAR_DATE_TIME:
            return Buffer(m_data.get<DateTime>().isoDateTimeString());

        case VAR_IMAGE_PTR: {
            const auto* ptr = static_cast<const void*>(static_cast<const uint8_t*>(m_data));
            return Buffer(ptr ? std::format("{:p}", ptr) : "null");
        }

        case VAR_IMAGE_NDX:
            return Buffer(to_string(m_data.get<int32_t>()));

        default:
            break;
    }

    return Buffer();
}

String BaseVariant::moneyDataToString() const
{
    const auto& moneyData = m_data.get<MoneyData>();
    const auto  scale = moneyData.scale();
    const auto  absQty = std::abs(moneyData.quantity());
    const auto  divider = MoneyData::divider(scale);
    return std::format("{}{}.{:0{}}",
                       moneyData.quantity() < 0 ? "-" : "",
                       absQty / divider,
                       absQty % divider,
                       static_cast<int>(scale));
}

DateTime VariantAdaptors::asDate() const
{
    if (isNull())
    {
        return DateTime();
    }

    switch (dataType())
    {
        case VAR_BOOL:
        case VAR_MONEY:
        case VAR_FLOAT:
            return DateTime();

        case VAR_INT:
            return DateTime(chrono::seconds(m_data.get<int32_t>())).date();

        case VAR_INT64:
            return DateTime(chrono::microseconds(m_data.get<int64_t>())).date();

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return DateTime(asString().c_str()).date();

        case VAR_DATE:
        case VAR_DATE_TIME:
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
        case VAR_BOOL:
        case VAR_MONEY:
        case VAR_FLOAT:
            return DateTime();

        case VAR_INT:
            return DateTime(chrono::seconds(m_data.get<int32_t>()));

        case VAR_INT64:
            return DateTime(chrono::microseconds(m_data.get<int64_t>()));

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            return DateTime(getString());

        case VAR_DATE:
        case VAR_DATE_TIME:
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

    if (dataType() == VAR_IMAGE_PTR)
    {
        return static_cast<const uint8_t*>(m_data);
    }

    throw Exception("Can't convert field for that type");
}

void VariantAdaptors::setNull(const VariantDataType variantDataType)
{
    m_data.setNull(true, variantDataType);
}

string_view VariantAdaptors::getBufferPtr() const
{
    if (isExternalBuffer())
    {
        return {static_cast<const char*>(m_data), m_data.size()};
    }

    const auto type = m_data.type().type;
    if (type == VAR_STRING)
    {
        auto& buffer = m_data.get<String>();
        return {buffer.c_str(), buffer.size()};
    }

    if (type == VAR_TEXT || type == VAR_BUFFER)
    {
        auto& buffer = m_data.get<Buffer>();
        return {buffer.c_str(), buffer.size()};
    }

    throw Exception("Can't get buffer for that type");
}

bool BaseVariant::isNull() const
{
    return m_data.type().isNull || m_data.type().type == VAR_NONE;
}

String BaseVariant::typeName(const VariantDataType type)
{
    switch (type)
    {
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

VariantDataType BaseVariant::nameType(const char* name)
{
    static const std::map<string, VariantDataType, less<>> nameToTypeMap {
        {typeName(VAR_NONE), VAR_NONE},
        {typeName(VAR_INT), VAR_INT},
        {typeName(VAR_FLOAT), VAR_FLOAT},
        {typeName(VAR_MONEY), VAR_MONEY},
        {typeName(VAR_STRING), VAR_STRING},
        {typeName(VAR_TEXT), VAR_TEXT},
        {typeName(VAR_BUFFER), VAR_BUFFER},
        {typeName(VAR_DATE), VAR_DATE},
        {typeName(VAR_DATE_TIME), VAR_DATE_TIME},
        {typeName(VAR_IMAGE_PTR), VAR_IMAGE_PTR},
        {typeName(VAR_IMAGE_NDX), VAR_IMAGE_NDX},
        {typeName(VAR_INT64), VAR_INT64},
        {typeName(VAR_BOOL), VAR_BOOL},
    };

    if (name == nullptr || name[0] == 0)
    {
        name = "string";
    }

    const auto itor = nameToTypeMap.find(lowerCase(name));
    if (itor == nameToTypeMap.end())
    {
        throw Exception("Type name " + string(name) + " isn't recognized");
    }

    return itor->second;
}

void Variant::load(const SNode& element)
{
    using enum Node::Type;
    switch (element->type())
    {
        case Number:
            *this = element->getNumber();
            break;
        case Boolean:
            *this = element->getBoolean();
            break;
        case Null:
            setNull();
            break;
        case Text:
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
            case VAR_BOOL:
                node->type(Node::Type::Boolean);
                break;
            case VAR_INT:
            case VAR_INT64:
            case VAR_FLOAT:
            case VAR_IMAGE_NDX:
                node->type(Node::Type::Number);
                break;
            case VAR_MONEY:
            case VAR_STRING:
            case VAR_DATE:
            case VAR_DATE_TIME:
                node->type(Node::Type::Text);
                break;

            case VAR_TEXT:
            case VAR_BUFFER:
                node->type(Node::Type::CData);
                break;

            default:
                break;
        }
    }
}
