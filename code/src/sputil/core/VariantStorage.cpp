#include <sptk5/VariantStorage.h>

using namespace std;
using namespace sptk;

VariantStorage::VariantStorage(const VariantStorage& other)
    : m_type(other.m_type)
{
    if (m_type == Type::Buffer)
    {
        m_value.asBuffer = new Buffer(*other.m_value.asBuffer);
    }
    else
    {
        m_value.asInteger = other.m_value.asInteger;
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

void VariantStorage::reset()
{
    if (m_type == Type::Buffer)
    {
        delete m_value.asBuffer;
        m_value.asBuffer = nullptr;
    }
    else
    {
        m_value.asInteger = 0;
    }
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

    if (m_type == Type::Buffer)
    {
        m_value.asBuffer = new Buffer(*other.m_value.asBuffer);
    }
    else
    {
        m_value.asInteger = other.m_value.asInteger;
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

VariantStorage::Type VariantStorage::type() const
{
    return m_type;
}
