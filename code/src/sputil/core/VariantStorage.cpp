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
    switch (m_type)
    {
        case Type::Integer:
            return m_value.asInteger;
        case Type::Null:
            return 0;
        default:
            throw std::runtime_error("Invalid type");
    }
}

VariantStorage::operator double() const
{
    switch (m_type)
    {
        case Type::Double:
            return m_value.asDouble;
        case Type::Null:
            return 0;
        default:
            throw std::runtime_error("Invalid type");
    }
}

VariantStorage::operator const Buffer&() const
{
    static const Buffer empty;
    switch (m_type)
    {
        case Type::Buffer:
            return *m_value.asBuffer;
        case Type::Null:
            return empty;
        default:
            throw std::runtime_error("Invalid type");
    }
}

VariantStorage::operator const DateTime&() const
{
    static const DateTime empty;
    switch (m_type)
    {
        case Type::DateTime:
            return *m_value.asDateTime;
        case Type::Null:
            return empty;
        default:
            throw std::runtime_error("Invalid type");
    }
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

VariantStorage::Type VariantStorage::type() const
{
    return m_type;
}

VariantStorage::VariantStorage(VariantStorage&& other)
    : m_value(other.m_value)
    , m_type(other.m_type)
{
    other.m_type = Type::Null;
    other.m_value.asInteger = 0;
}

VariantStorage& VariantStorage::operator=(VariantStorage&& other)
{
    if (this != &other)
    {
        m_value = other.m_value;
        m_type = other.m_type;
        other.m_type = Type::Null;
        other.m_value.asInteger = 0;
    }
    return *this;
}
