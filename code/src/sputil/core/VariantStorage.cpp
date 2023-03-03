#include <sptk5/VariantStorage.h>

using namespace std;
using namespace sptk;

BaseVariantStorage::BaseVariantStorage(const BaseVariantStorage& other, int)
    : m_type(other.m_type)
{
    switch (m_type.type)
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
{
    m_value.asInt64 = value != 0 ? 1 : 0;
    m_type.type = VariantDataType::VAR_BOOL;
    m_type.size = sizeof(value);
}

BaseVariantStorage::BaseVariantStorage(int value)
{
    m_value.asInt64 = value;
    m_type.type = VariantDataType::VAR_INT;
    m_type.size = sizeof(value);
}

BaseVariantStorage::BaseVariantStorage(int64_t value)
{
    m_value.asInt64 = value;
    m_type.type = VariantDataType::VAR_INT64;
    m_type.size = sizeof(value);
}

BaseVariantStorage::BaseVariantStorage(double value)
{
    m_value.asDouble = value;
    m_type.type = VariantDataType::VAR_FLOAT;
    m_type.size = sizeof(value);
}

BaseVariantStorage::BaseVariantStorage(Buffer&& value)
{
    auto buffer = make_shared<Buffer>();
    *buffer = std::move(value);
    m_class = buffer;
    m_type.type = VariantDataType::VAR_BUFFER;
    m_type.size = sizeof(value.size());
}

BaseVariantStorage::BaseVariantStorage(const uint8_t* value, size_t dataSize, bool externalBuffer)
{
    if (externalBuffer || value == nullptr)
    {
        m_value.asBytePointer = value;
        m_type.type = VariantDataType::VAR_BYTE_POINTER;
        m_type.isNull = value == nullptr;
    }
    else
    {
        m_class = make_shared<Buffer>(value, dataSize);
        m_type.type = VariantDataType::VAR_BUFFER;
    }
    m_type.size = m_type.isNull ? 0 : dataSize;
    m_type.isExternalBuffer = externalBuffer;
}

void BaseVariantStorage::setNull()
{
    if (m_class)
    {
        m_class.reset();
    }
    m_value.asInt64 = 0;
    m_type.isNull = true;
    m_type.size = 0;
    m_type.isExternalBuffer = false;
}

VariantStorage::operator bool() const
{
    if (type().type == VariantDataType::VAR_BOOL)
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
    if (type().type == VariantDataType::VAR_INT || type().type == VariantDataType::VAR_INT64)
    {
        return value().asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator double() const
{
    if (type().type == VariantDataType::VAR_FLOAT)
    {
        return value().asDouble;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator bool&()
{
    if (type().type == VariantDataType::VAR_BOOL)
    {
        return value().asBool;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int&()
{
    if (type().type == VariantDataType::VAR_INT || type().type == VariantDataType::VAR_INT64)
    {
        return value().asInt;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator int64_t&()
{
    if (type().type == VariantDataType::VAR_INT || type().type == VariantDataType::VAR_INT64)
    {
        return value().asInt64;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator double&()
{
    if (type().type == VariantDataType::VAR_FLOAT)
    {
        return value().asDouble;
    }
    throw invalid_argument("Invalid type");
}

VariantStorage::operator const uint8_t*() const
{
    if (type().type == VariantDataType::VAR_BYTE_POINTER)
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

    switch (type().type)
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
    setSize(sizeof(aValue));
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
    setSize(sizeof(aValue));
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
    setSize(sizeof(aValue));
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
    setSize(sizeof(aValue));
    value().asDouble = aValue;
    return *this;
}

VariantStorage& VariantStorage::operator=(Buffer&& aValue)
{
    if (type().type != VariantDataType::VAR_BUFFER || !storageClient())
    {
        auto buffer = make_shared<Buffer>();
        *buffer = std::move(aValue);
        setStorageClient(buffer);
    }
    else
    {
        *dynamic_pointer_cast<Buffer>(storageClient()) = std::move(aValue);
    }
    setType(VariantDataType::VAR_BUFFER);
    setNull(false);
    setSize(sizeof(aValue));
    return *this;
}

void VariantStorage::setExternalPointer(const uint8_t* aValue, size_t dataSize)
{
    if (storageClient())
    {
        storageClient().reset();
    }
    setType(VariantDataType::VAR_BYTE_POINTER);
    setNull(false);
    setSize(dataSize);
    value().asBytePointer = aValue;
}

VariantStorage& VariantStorage::operator=(VariantStorage&& other) noexcept
{
    if (this != &other)
    {
        value() = other.value();
        setStorageClient(other.storageClient());
        setType(other.type());
        setSize(other.size());
        other.value().asInt64 = 0;
        other.storageClient().reset();
        other.setType(VariantDataType::VAR_NONE);
        other.setNull(true);
    }
    return *this;
}
