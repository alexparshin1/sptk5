#include <sptk5/VariantStorage.h>

using namespace std;
using namespace sptk;

VariantStorage::VariantStorage(const VariantStorage& other)
    : m_type(other.m_type), m_null(other.m_null)
{
    switch (m_type)
    {
        case Type::Buffer:
            m_class = make_shared<Buffer>(*dynamic_pointer_cast<Buffer>(other.m_class));
            break;
        case Type::String:
            m_class = make_shared<String>(*dynamic_pointer_cast<String>(other.m_class));
            break;
        case Type::DateTime:
            m_class = make_shared<DateTime>(*dynamic_pointer_cast<DateTime>(other.m_class));
            break;
        case Type::Money:
            m_class = make_shared<MoneyData>(*dynamic_pointer_cast<MoneyData>(other.m_class));
            break;
        default:
            m_value.asInteger = other.m_value.asInteger;
            break;
    }
}

VariantStorage::VariantStorage(bool value)
    : m_type(Type::Bool), m_null(false)
{
    m_value.asInteger = value != 0 ? 1 : 0;
}

VariantStorage::VariantStorage(int value)
    : m_type(Type::Integer), m_null(false)
{
    m_value.asInteger = value;
}

VariantStorage::VariantStorage(int64_t value)
    : m_type(Type::Integer), m_null(false)
{
    m_value.asInteger = value;
}

VariantStorage::VariantStorage(double value)
    : m_type(Type::Double), m_null(false)
{
    m_value.asDouble = value;
}

VariantStorage::VariantStorage(const Buffer& value)
    : m_type(Type::Buffer), m_null(false)
{
    m_class = make_shared<Buffer>(value);
}

VariantStorage::VariantStorage(Buffer&& value)
    : m_type(Type::Buffer), m_null(false)
{
    auto buffer = make_shared<Buffer>();
    *buffer = std::move(value);
    m_class = buffer;
}

VariantStorage::VariantStorage(const String& value)
    : m_type(Type::String), m_null(false)
{
    m_class = make_shared<String>(value);
}

VariantStorage::VariantStorage(const DateTime& value)
    : m_type(Type::DateTime), m_null(false)
{
    m_class = make_shared<DateTime>(value);
}

VariantStorage::VariantStorage(const MoneyData& value)
    : m_type(Type::Money), m_null(false)
{
    m_class = make_shared<MoneyData>(value);
}

VariantStorage::VariantStorage(const uint8_t* value)
    : m_type(Type::BytePointer), m_null(false)
{
    m_value.asBytePointer = value;
}

VariantStorage::VariantStorage(const char* value)
    : m_type(Type::CharPointer), m_null(false)
{
    m_value.asCharPointer = value;
}

VariantStorage::operator bool() const
{
    if (m_type == Type::Bool)
    {
        return (bool) m_value.asInteger;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int() const
{
    return (int) operator int64_t();
}

VariantStorage::operator int64_t() const
{
    if (m_type == Type::Integer)
    {
        return m_value.asInteger;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator double() const
{
    if (m_type == Type::Double)
    {
        return m_value.asDouble;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const Buffer&() const
{
    if (m_type == Type::Buffer)
    {
        return *dynamic_pointer_cast<Buffer>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const String&() const
{
    if (m_type == Type::String)
    {
        return *dynamic_pointer_cast<String>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const DateTime&() const
{
    if (m_type == Type::DateTime)
    {
        return *dynamic_pointer_cast<DateTime>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const MoneyData&() const
{
    if (m_type == Type::Money)
    {
        return *dynamic_pointer_cast<MoneyData>(m_class);
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const uint8_t*() const
{
    if (m_type == Type::BytePointer)
    {
        return m_value.asBytePointer;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const char*() const
{
    if (m_type == Type::CharPointer)
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
    m_value.asInteger = 0;
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
        case Type::Buffer:
            m_class = make_shared<Buffer>(*dynamic_pointer_cast<Buffer>(other.m_class));
            break;
        case Type::String:
            m_class = make_shared<String>(*dynamic_pointer_cast<String>(other.m_class));
            break;
        case Type::DateTime:
            m_class = make_shared<DateTime>(*dynamic_pointer_cast<DateTime>(other.m_class));
            break;
        case Type::Money:
            m_class = make_shared<MoneyData>(*dynamic_pointer_cast<MoneyData>(other.m_class));
            break;
        default:
            m_value.asInteger = other.m_value.asInteger;
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
    m_type = Type::Bool;
    m_null = false;
    m_value.asInteger = value != 0 ? 1 : 0;
    return *this;
}

VariantStorage& VariantStorage::operator=(int value)
{
    operator=((int64_t) value);
    return *this;
}

VariantStorage& VariantStorage::operator=(int64_t value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_null = false;
    m_type = Type::Integer;
    m_value.asInteger = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(double value)
{
    if (m_class)
    {
        m_class.reset();
    }
    m_null = false;
    m_type = Type::Double;
    m_value.asDouble = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(const Buffer& value)
{
    if (m_type != Type::Buffer)
    {
        m_class = make_shared<Buffer>(value);
        m_type = Type::Buffer;
    }
    else
    {
        *dynamic_pointer_cast<Buffer>(m_class) = value;
    }
    m_null = false;
    return *this;
}

VariantStorage& VariantStorage::operator=(const String& value)
{
    if (m_type != Type::String)
    {
        m_class = make_shared<String>(value);
        m_type = Type::String;
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
    if (m_type != Type::DateTime)
    {
        m_class = make_shared<DateTime>(value);
        m_type = Type::DateTime;
    } else {
        *dynamic_pointer_cast<DateTime>(m_class) = value;
    }
    m_null = false;
    return *this;
}

VariantStorage& VariantStorage::operator=(const MoneyData& value)
{
    if (m_type != Type::Money)
    {
        m_class = make_shared<MoneyData>(value);
        m_type = Type::Money;
    } else {
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
    m_type = Type::BytePointer;
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
    m_type = Type::CharPointer;
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
    other.m_value.asInteger = 0;
    other.m_type = Type::Undefined;
    other.m_null = true;
}

VariantStorage& VariantStorage::operator=(VariantStorage&& other) noexcept
{
    if (this != &other)
    {
        m_value = other.m_value;
        m_class = other.m_class;
        m_type = other.m_type;
        other.m_value.asInteger = 0;
        other.m_class.reset();
        other.m_type = Type::Undefined;
        other.m_null = true;
    }
    return *this;
}
