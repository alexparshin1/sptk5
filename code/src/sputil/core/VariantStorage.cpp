#include <sptk5/VariantStorage.h>

using namespace std;
using namespace sptk;

VariantStorage::VariantStorage(const VariantStorage& other)
    : m_type(other.m_type)
{
    switch (m_type)
    {
        case Type::Buffer:
            m_value.asBuffer = new Buffer(*other.m_value.asBuffer);
            break;
        case Type::DateTime:
            m_value.asDateTime = new DateTime(*other.m_value.asDateTime);
            break;
        case Type::Money:
            m_value.asMoneyData = new MoneyData(*other.m_value.asMoneyData);
            break;
        default:
            m_value.asInteger = other.m_value.asInteger;
            break;
    }
}

VariantStorage::VariantStorage(int value)
    : m_type(Type::Integer)
{
    m_value.asInteger = value;
}

VariantStorage::VariantStorage(int64_t value)
    : m_type(Type::Integer)
{
    m_value.asInteger = value;
}

VariantStorage::VariantStorage(double value)
    : m_type(Type::Double)
{
    m_value.asDouble = value;
}

VariantStorage::VariantStorage(const Buffer& value)
    : m_type(Type::Buffer)
{
    m_value.asBuffer = new Buffer(value);
}

VariantStorage::VariantStorage(const DateTime& value)
    : m_type(Type::DateTime)
{
    m_value.asDateTime = new DateTime(value);
}

VariantStorage::VariantStorage(const MoneyData& value)
    : m_type(Type::Money)
{
    m_value.asMoneyData = new MoneyData(value);
}

VariantStorage::VariantStorage(const uint8_t* value)
    : m_type(Type::BytePointer)
{
    m_value.asBytePointer = value;
}

VariantStorage::VariantStorage(const char* value)
    : m_type(Type::CharPointer)
{
    m_value.asCharPointer = value;
}

VariantStorage::~VariantStorage()
{
    reset();
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
        return *m_value.asBuffer;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const DateTime&() const
{
    if (m_type == Type::DateTime)
    {
        return *m_value.asDateTime;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const MoneyData&() const
{
    if (m_type == Type::Money)
    {
        return *m_value.asMoneyData;
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

void VariantStorage::reset()
{
    switch (m_type)
    {
        case Type::Buffer:
            delete m_value.asBuffer;
            break;
        case Type::DateTime:
            delete m_value.asDateTime;
            break;
        case Type::Money:
            delete m_value.asMoneyData;
            break;
        default:
            break;
    }
    m_value.asInteger = 0;
    m_type = Type::Null;
}

VariantStorage& VariantStorage::operator=(const VariantStorage& other)
{
    if (this == &other)
    {
        return *this;
    }

    if (m_type != other.m_type)
    {
        reset();
    }

    m_type = other.m_type;

    switch (m_type)
    {
        case Type::Buffer:
            m_value.asBuffer = new Buffer(*other.m_value.asBuffer);
            break;
        case Type::DateTime:
            m_value.asDateTime = new DateTime(*other.m_value.asDateTime);
            break;
        case Type::Money:
            m_value.asMoneyData = new MoneyData(*other.m_value.asMoneyData);
            break;
        default:
            m_value.asInteger = other.m_value.asInteger;
            break;
    }

    return *this;
}

VariantStorage& VariantStorage::operator=(int value)
{
    operator=((int64_t) value);
    return *this;
}

VariantStorage& VariantStorage::operator=(int64_t value)
{
    if (m_type != Type::Integer)
    {
        reset();
    }
    m_type = Type::Integer;
    m_value.asInteger = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(double value)
{
    if (m_type != Type::Double)
    {
        reset();
    }
    m_type = Type::Double;
    m_value.asDouble = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(const Buffer& value)
{
    if (m_type != Type::Buffer)
    {
        reset();
        m_value.asBuffer = new Buffer(value);
        m_type = Type::Buffer;
    }
    else
    {
        *m_value.asBuffer = value;
    }
    return *this;
}

VariantStorage& VariantStorage::operator=(const DateTime& value)
{
    switch (m_type)
    {
        case Type::Buffer:
        case Type::Money:
            reset();
            m_value.asDateTime = new DateTime(value);
            break;
        case Type::DateTime:
            *m_value.asDateTime = value;
            break;
        default:
            m_value.asDateTime = new DateTime(value);
            break;
    }
    m_type = Type::DateTime;
    return *this;
}

VariantStorage& VariantStorage::operator=(const MoneyData& value)
{
    switch (m_type)
    {
        case Type::Buffer:
        case Type::DateTime:
            reset();
            m_value.asMoneyData = new MoneyData(value);
            break;
        case Type::Money:
            *m_value.asMoneyData = value;
            break;
        default:
            m_value.asMoneyData = new MoneyData(value);
            break;
    }
    m_type = Type::Money;
    return *this;
}

VariantStorage& VariantStorage::operator=(const uint8_t* value)
{
    switch (m_type)
    {
        case Type::Buffer:
        case Type::DateTime:
        case Type::Money:
            reset();
            break;
        default:
            break;
    }
    m_type = Type::BytePointer;
    m_value.asBytePointer = value;
    return *this;
}

VariantStorage& VariantStorage::operator=(const char* value)
{
    switch (m_type)
    {
        case Type::Buffer:
        case Type::DateTime:
        case Type::Money:
            reset();
            break;
        default:
            break;
    }
    m_type = Type::CharPointer;
    m_value.asCharPointer = value;
    return *this;
}

VariantStorage::Type VariantStorage::type() const
{
    return m_type;
}

VariantStorage::VariantStorage(VariantStorage&& other) noexcept
    : m_value(other.m_value)
    , m_type(other.m_type)
{
    other.m_type = Type::Null;
    other.m_value.asInteger = 0;
}

VariantStorage& VariantStorage::operator=(VariantStorage&& other) noexcept
{
    if (this != &other)
    {
        reset();
        m_value = other.m_value;
        m_type = other.m_type;
        other.m_type = Type::Null;
        other.m_value.asInteger = 0;
    }
    return *this;
}
