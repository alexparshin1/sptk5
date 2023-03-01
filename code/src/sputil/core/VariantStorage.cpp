#include <sptk5/VariantStorage.h>

using namespace std;
using namespace sptk;

VariantStorage::VariantStorage(const VariantStorage& other)
    : m_type(other.m_type)
    , m_null(other.m_null)
{
    switch (m_type)
    {
        case VariantDataType::VAR_BUFFER:
            m_class = make_shared<Buffer>(*dynamic_pointer_cast<Buffer>(other.m_class));
            break;
        case VariantDataType::VAR_STRING:
            m_class = make_shared<String>(*dynamic_pointer_cast<String>(other.m_class));
            break;
        case VariantDataType::VAR_DATE_TIME:
            m_class = make_shared<DateTime>(*dynamic_pointer_cast<DateTime>(other.m_class));
            break;
        case VariantDataType::VAR_MONEY:
            m_class = make_shared<MoneyData>(*dynamic_pointer_cast<MoneyData>(other.m_class));
            break;
        default:
            m_value.asInt64 = other.m_value.asInt64;
            break;
    }
}

VariantStorage::VariantStorage(bool value)
    : m_type(VariantDataType::VAR_BOOL)
    , m_null(false)
{
    m_value.asInt64 = value != 0 ? 1 : 0;
}

VariantStorage::VariantStorage(int value)
    : m_type(VariantDataType::VAR_INT)
    , m_null(false)
{
    m_value.asInt64 = value;
}

VariantStorage::VariantStorage(int64_t value)
    : m_type(VariantDataType::VAR_INT64)
    , m_null(false)
{
    m_value.asInt64 = value;
}

VariantStorage::VariantStorage(double value)
    : m_type(VariantDataType::VAR_FLOAT)
    , m_null(false)
{
    m_value.asDouble = value;
}

VariantStorage::VariantStorage(const Buffer& value)
    : m_type(VariantDataType::VAR_BUFFER)
    , m_null(false)
{
    m_class = make_shared<Buffer>(value);
}

VariantStorage::VariantStorage(Buffer&& value)
    : m_type(VariantDataType::VAR_BUFFER)
    , m_null(false)
{
    auto buffer = make_shared<Buffer>();
    *buffer = std::move(value);
    m_class = buffer;
}

VariantStorage::VariantStorage(const String& value)
    : m_type(VariantDataType::VAR_STRING)
    , m_null(false)
{
    m_class = make_shared<String>(value);
}

VariantStorage::VariantStorage(const DateTime& value)
    : m_type(VariantDataType::VAR_DATE_TIME)
    , m_null(false)
{
    m_class = make_shared<DateTime>(value);
}

VariantStorage::VariantStorage(const MoneyData& value)
    : m_type(VariantDataType::VAR_MONEY)
    , m_null(false)
{
    m_class = make_shared<MoneyData>(value);
}

VariantStorage::VariantStorage(const uint8_t* value)
    : m_type(VariantDataType::VAR_BYTE_POINTER)
    , m_null(false)
{
    m_value.asBytePointer = value;
}

VariantStorage::VariantStorage(const char* value)
    : m_type(VariantDataType::VAR_CHAR_POINTER)
    , m_null(false)
{
    m_value.asCharPointer = value;
}

VariantStorage::operator bool() const
{
    if (m_type == VariantDataType::VAR_BOOL)
    {
        return (bool) m_value.asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int() const
{
    return (int) operator int64_t();
}

VariantStorage::operator int64_t() const
{
    if (m_type == VariantDataType::VAR_INT || m_type == VariantDataType::VAR_INT64)
    {
        return m_value.asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator double() const
{
    if (m_type == VariantDataType::VAR_FLOAT)
    {
        return m_value.asDouble;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const Buffer&() const
{
    if (m_type == VariantDataType::VAR_BUFFER)
    {
        return *dynamic_pointer_cast<Buffer>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const String&() const
{
    if (m_type == VariantDataType::VAR_STRING)
    {
        return *dynamic_pointer_cast<String>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const DateTime&() const
{
    if (m_type == VariantDataType::VAR_DATE_TIME)
    {
        return *dynamic_pointer_cast<DateTime>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const MoneyData&() const
{
    if (m_type == VariantDataType::VAR_MONEY)
    {
        return *dynamic_pointer_cast<MoneyData>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator bool&()
{
    if (m_type == VariantDataType::VAR_BOOL)
    {
        return m_value.asBool;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int&()
{
    if (m_type == VariantDataType::VAR_INT || m_type == VariantDataType::VAR_INT64)
    {
        return m_value.asInt;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int64_t&()
{
    if (m_type == VariantDataType::VAR_INT || m_type == VariantDataType::VAR_INT64)
    {
        return m_value.asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator double&()
{
    if (m_type == VariantDataType::VAR_FLOAT)
    {
        return m_value.asDouble;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const uint8_t*() const
{
    if (m_type == VariantDataType::VAR_BYTE_POINTER)
    {
        return m_value.asBytePointer;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const char*() const
{
    if (m_type == VariantDataType::VAR_CHAR_POINTER)
    {
        return m_value.asCharPointer;
    }
    throw invalid_argument("Invalid type");
}

void VariantStorage::setNull()
{
    if (m_class)
    {
        m_class.reset();
    }
    m_value.asInt64 = 0;
    m_null = true;
}

VariantStorage& VariantStorage::operator=(const VariantStorage& other)
{
    if (this == &other)
    {
        return *this;
    }

    m_type = other.m_type;
    m_null = other.m_null;

    switch (m_type)
    {
        case VariantDataType::VAR_BUFFER:
            m_class = make_shared<Buffer>(*dynamic_pointer_cast<Buffer>(other.m_class));
            break;
        case VariantDataType::VAR_STRING:
            m_class = make_shared<String>(*dynamic_pointer_cast<String>(other.m_class));
            break;
        case VariantDataType::VAR_DATE_TIME:
            m_class = make_shared<DateTime>(*dynamic_pointer_cast<DateTime>(other.m_class));
            break;
        case VariantDataType::VAR_MONEY:
            m_class = make_shared<MoneyData>(*dynamic_pointer_cast<MoneyData>(other.m_class));
            break;
        default:
            m_value.asInt64 = other.m_value.asInt64;
            break;
    }

    return *this;
}

VariantStorage& VariantStorage::operator=(bool value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_type = VariantDataType::VAR_BOOL;
    m_null = false;
    m_value.asInt64 = value != 0 ? 1 : 0;
    return *this;
}

VariantStorage& VariantStorage::operator=(int value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_null = false;
    m_type = VariantDataType::VAR_INT;
    m_value.asInt64 = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(int64_t value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_null = false;
    m_type = VariantDataType::VAR_INT64;
    m_value.asInt64 = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(double value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_null = false;
    m_type = VariantDataType::VAR_FLOAT;
    m_value.asDouble = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(const Buffer& value)
{
    if (m_type != VariantDataType::VAR_BUFFER || !m_class)
    {
        m_class = make_shared<Buffer>(value);
        m_type = VariantDataType::VAR_BUFFER;
    }
    else
    {
        *dynamic_pointer_cast<Buffer>(m_class) = value;
    }
    m_null = false;
    return *this;
}

VariantStorage& VariantStorage::operator=(Buffer&& value)
{
    if (m_type != VariantDataType::VAR_BUFFER || !m_class)
    {
        auto buffer = make_shared<Buffer>();
        *buffer = std::move(value);
        m_class = buffer;
        m_type = VariantDataType::VAR_BUFFER;
    }
    else
    {
        *dynamic_pointer_cast<Buffer>(m_class) = std::move(value);
    }
    m_null = false;
    return *this;
}

VariantStorage& VariantStorage::operator=(const String& value)
{
    if (m_type != VariantDataType::VAR_STRING || !m_class)
    {
        m_class = make_shared<String>(value);
        m_type = VariantDataType::VAR_STRING;
    }
    else
    {
        *dynamic_pointer_cast<String>(m_class) = value;
    }
    m_null = false;
    return *this;
}

VariantStorage& VariantStorage::operator=(const DateTime& value)
{
    if (m_type != VariantDataType::VAR_DATE_TIME || !m_class)
    {
        m_class = make_shared<DateTime>(value);
        m_type = VariantDataType::VAR_DATE_TIME;
    }
    else
    {
        *dynamic_pointer_cast<DateTime>(m_class) = value;
    }
    m_null = false;
    return *this;
}

VariantStorage& VariantStorage::operator=(const MoneyData& value)
{
    if (m_type != VariantDataType::VAR_MONEY || !m_class)
    {
        m_class = make_shared<MoneyData>(value);
        m_type = VariantDataType::VAR_MONEY;
    }
    else
    {
        *dynamic_pointer_cast<MoneyData>(m_class) = value;
    }
    m_null = false;
    return *this;
}

VariantStorage& VariantStorage::operator=(const uint8_t* value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_type = VariantDataType::VAR_BYTE_POINTER;
    m_null = false;
    m_value.asBytePointer = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(const char* value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_type = VariantDataType::VAR_CHAR_POINTER;
    m_null = false;
    m_value.asCharPointer = value;
    return *this;
}

VariantStorage::VariantStorage(VariantStorage&& other) noexcept
    : m_value(other.m_value)
    , m_class(other.m_class)
    , m_type(other.m_type)
    , m_null(other.m_null)
{
    other.m_class.reset();
    other.m_value.asInt64 = 0;
    other.m_type = VariantDataType::VAR_NONE;
    other.m_null = true;
}

VariantStorage& VariantStorage::operator=(VariantStorage&& other) noexcept
{
    if (this != &other)
    {
        m_value = other.m_value;
        m_class = other.m_class;
        m_type = other.m_type;
        other.m_value.asInt64 = 0;
        other.m_class.reset();
        other.m_type = VariantDataType::VAR_NONE;
        other.m_null = true;
    }
    return *this;
}
