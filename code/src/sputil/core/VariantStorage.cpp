#include <sptk5/VariantStorage.h>

using namespace std;
using namespace sptk;

BaseVariantStorage::BaseVariantStorage(const BaseVariantStorage& other, int)
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

BaseVariantStorage::BaseVariantStorage(bool value)
    : m_type(VariantDataType::VAR_BOOL)
    , m_null(false)
{
    m_value.asInt64 = value != 0 ? 1 : 0;
}

BaseVariantStorage::BaseVariantStorage(int value)
    : m_type(VariantDataType::VAR_INT)
    , m_null(false)
{
    m_value.asInt64 = value;
}

BaseVariantStorage::BaseVariantStorage(int64_t value)
    : m_type(VariantDataType::VAR_INT64)
    , m_null(false)
{
    m_value.asInt64 = value;
}

BaseVariantStorage::BaseVariantStorage(double value)
    : m_type(VariantDataType::VAR_FLOAT)
    , m_null(false)
{
    m_value.asDouble = value;
}

BaseVariantStorage::BaseVariantStorage(Buffer&& value)
    : m_type(VariantDataType::VAR_BUFFER)
    , m_null(false)
{
    auto buffer = make_shared<Buffer>();
    *buffer = std::move(value);
    m_class = buffer;
}

BaseVariantStorage::BaseVariantStorage(const uint8_t* value)
    : m_type(VariantDataType::VAR_BYTE_POINTER)
    , m_null(false)
{
    m_value.asBytePointer = value;
}

void BaseVariantStorage::setNull()
{
    if (m_class)
    {
        m_class.reset();
    }
    m_value.asInt64 = 0;
    m_null = true;
}

VariantStorage::operator bool() const
{
    if (type() == VariantDataType::VAR_BOOL)
    {
        return (bool) value().asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int() const
{
    return (int) operator int64_t();
}

VariantStorage::operator int64_t() const
{
    if (type() == VariantDataType::VAR_INT || type() == VariantDataType::VAR_INT64)
    {
        return value().asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator double() const
{
    if (type() == VariantDataType::VAR_FLOAT)
    {
        return value().asDouble;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator bool&()
{
    if (type() == VariantDataType::VAR_BOOL)
    {
        return value().asBool;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int&()
{
    if (type() == VariantDataType::VAR_INT || type() == VariantDataType::VAR_INT64)
    {
        return value().asInt;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int64_t&()
{
    if (type() == VariantDataType::VAR_INT || type() == VariantDataType::VAR_INT64)
    {
        return value().asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator double&()
{
    if (type() == VariantDataType::VAR_FLOAT)
    {
        return value().asDouble;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const uint8_t*() const
{
    if (type() == VariantDataType::VAR_BYTE_POINTER)
    {
        return value().asBytePointer;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage& VariantStorage::operator=(const VariantStorage& other)
{
    if (this == &other)
    {
        return *this;
    }

    setType(other.type());
    setNull(other.isNull());

    switch (type())
    {
        case VariantDataType::VAR_BUFFER:
            setStorageClient(make_shared<Buffer>(*dynamic_pointer_cast<Buffer>(other.storageClient())));
            break;
        case VariantDataType::VAR_STRING:
            setStorageClient(make_shared<String>(*dynamic_pointer_cast<String>(other.storageClient())));
            break;
        case VariantDataType::VAR_DATE_TIME:
            setStorageClient(make_shared<DateTime>(*dynamic_pointer_cast<DateTime>(other.storageClient())));
            break;
        case VariantDataType::VAR_MONEY:
            setStorageClient(make_shared<MoneyData>(*dynamic_pointer_cast<MoneyData>(other.storageClient())));
            break;
        default:
            value().asInt64 = other.value().asInt64;
            break;
    }

    return *this;
}

VariantStorage& VariantStorage::operator=(bool aValue)
{
    if (storageClient())
    {
        storageClient().reset();
    }
    setType(VariantDataType::VAR_BOOL);
    setNull(false);
    value().asInt64 = aValue != 0 ? 1 : 0;
    return *this;
}

VariantStorage& VariantStorage::operator=(int aValue)
{
    if (storageClient())
    {
        storageClient().reset();
    }
    setNull(false);
    setType(VariantDataType::VAR_INT);
    value().asInt64 = aValue;
    return *this;
}

VariantStorage& VariantStorage::operator=(int64_t aValue)
{
    if (storageClient())
    {
        storageClient().reset();
    }
    setNull(false);
    setType(VariantDataType::VAR_INT64);
    value().asInt64 = aValue;
    return *this;
}

VariantStorage& VariantStorage::operator=(double aValue)
{
    if (storageClient())
    {
        storageClient().reset();
    }
    setNull(false);
    setType(VariantDataType::VAR_FLOAT);
    value().asDouble = aValue;
    return *this;
}

VariantStorage& VariantStorage::operator=(Buffer&& aValue)
{
    if (type() != VariantDataType::VAR_BUFFER || !storageClient())
    {
        auto buffer = make_shared<Buffer>();
        *buffer = std::move(aValue);
        setStorageClient(buffer);
        setType(VariantDataType::VAR_BUFFER);
    }
    else
    {
        *dynamic_pointer_cast<Buffer>(storageClient()) = std::move(aValue);
    }
    setNull(false);
    return *this;
}

VariantStorage& VariantStorage::operator=(const uint8_t* aValue)
{
    if (storageClient())
    {
        storageClient().reset();
    }
    setType(VariantDataType::VAR_BYTE_POINTER);
    setNull(false);
    value().asBytePointer = aValue;
    return *this;
}

VariantStorage& VariantStorage::operator=(VariantStorage&& other) noexcept
{
    if (this != &other)
    {
        value() = other.value();
        setStorageClient(other.storageClient());
        setType(other.type());
        other.value().asInt64 = 0;
        other.storageClient().reset();
        other.setType(VariantDataType::VAR_NONE);
        other.setNull(true);
    }
    return *this;
}
