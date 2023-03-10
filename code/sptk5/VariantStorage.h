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

#pragma once

#include <sptk5/Buffer.h>
#include <sptk5/DateTime.h>
#include <sptk5/MoneyData.h>
#include <sptk5/VariantDataType.h>

#include <memory>

namespace sptk {

/**
 * @brief Base class for variant storage
 */
class SP_EXPORT BaseVariantStorage
{
public:
    /**
     * @brief Constructor
     */
    BaseVariantStorage()
    {
        m_type.isNull = true;
    }

    /**
     * @brief Copy constructor
     * @param other             The other object
     */
    BaseVariantStorage(const BaseVariantStorage& other, int);

    /**
     * @brief Move constructor
     * @param other             The other object
     */
    BaseVariantStorage(BaseVariantStorage&& other) noexcept = default;

    /**
     * @brief Constructor
     * @param value             Value to store
     */
    explicit BaseVariantStorage(bool value);

    /**
     * @brief Constructor
     * @param value             Value to store
     */
    explicit BaseVariantStorage(int value);

    /**
     * @brief Constructor
     * @param value             Value to store
     */
    explicit BaseVariantStorage(int64_t value);

    /**
     * @brief Constructor
     * @param value             Value to store
     */
    explicit BaseVariantStorage(double value);

    /**
     * @brief Constructor
     * @param value             Data to store
     * @param dataSize          Data size
     * @param externalBuffer    Flag if data should not be copied inside
     */
    BaseVariantStorage(const uint8_t* value, size_t dataSize, bool externalBuffer = false);

    /**
     * @brief Constructor
     * @tparam T                Data type, derived from VariantStorageClient
     * @param value             Data
     */
    template<typename T, typename std::enable_if_t<std::is_base_of_v<VariantStorageClient, T>, int> = 0>
    explicit BaseVariantStorage(const T& value)
        : m_class(std::make_shared<T>(value))
    {
        m_type.type = T::variantDataType();
        m_type.size = value.dataSize();
    }

    /**
     * @brief Constructor
     * @param buffer            Buffer moved inside this object
     */
    explicit BaseVariantStorage(Buffer&& buffer);

    /**
     * @brief Destructor
     */
    virtual ~BaseVariantStorage() = default;

    /**
     * @brief Return data type
     * @return data type
     */
    [[nodiscard]] const VariantType& type() const
    {
        return m_type;
    }

    /**
     * @brief Set data type
     * @param type              Data type
     */
    void type(const VariantType& type)
    {
        m_type = type;
    }

    /**
     * @brief Set data type
     * @param type              Data type
     */
    void type(const VariantDataType& type)
    {
        m_type.type = type;
    }

    /**
     * @brief Check if the data is null
     * @return true if data is null
     */
    [[nodiscard]] bool isNull() const
    {
        return m_type.isNull;
    }

    /**
     * @brief Set data is null state
     */
    void setNull();

    /**
     * @brief Set data is null state
     * @param isNull                Data is null flag
     * @param type                  Data type
     * @param clearStorageClient    If true then reset storage client object
     */
    void setNull(bool isNull, VariantDataType type, bool clearStorageClient = true)
    {
        m_type.type = type;
        if (isNull)
        {
            setNull();
            if (clearStorageClient)
            {
                setStorageClient(nullptr);
            }
        }
        else
        {
            m_type.isNull = false;
            m_type.type = type;
        }
    }

    /**
     * @brief Set data size
     * @param size              Data size
     */
    void setSize(size_t size)
    {
        m_type.size = size;
    }

    /**
     * @brief Get data size
     * @return data size
     */
    [[nodiscard]] size_t size() const
    {
        return m_type.size;
    }

protected:
    union VariantValue
    {
        bool asBool;
        int asInt;
        int64_t asInt64;
        double asDouble;
        const uint8_t* asBytePointer;
    };

    [[nodiscard]] std::shared_ptr<VariantStorageClient> storageClient()
    {
        return m_class;
    }

    [[nodiscard]] const std::shared_ptr<VariantStorageClient>& storageClient() const
    {
        return m_class;
    }

    void setStorageClient(const std::shared_ptr<VariantStorageClient>& storageClient)
    {
        m_class = storageClient;
    }

    void setType(VariantType dataType)
    {
        m_type = dataType;
    }

    [[maybe_unused]] void setType(VariantDataType dataType)
    {
        m_type.type = dataType;
    }

    [[nodiscard]] VariantValue& value()
    {
        return m_value;
    }

    [[nodiscard]] const VariantValue& value() const
    {
        return m_value;
    }

private:
    VariantValue m_value {};
    std::shared_ptr<VariantStorageClient> m_class;
    VariantType m_type {};
};

/// @brief Compact variant data storage
///
/// Unlike std::variant that uses combined space for all variant types,
/// this class shares the space for included data types, taking just 16 bytes of memory
/// (dynamically allocated memory for Buffer or DateTime is not included)
class SP_EXPORT VariantStorage : public BaseVariantStorage
{
public:
    using BaseVariantStorage::BaseVariantStorage;

    /**
     * @brief Copy constructor
     * @param other
     */
    VariantStorage(const VariantStorage& other)
        : BaseVariantStorage(other, 0)
    {
    }

    /**
     * @brief Move constructor
     * @param other
     */
    VariantStorage(VariantStorage&& other)
        : BaseVariantStorage(std::move(other))
    {
    }

    /**
     * @brief Destructor
     */
    ~VariantStorage() override = default;

    /**
     * @brief Explicit const conversion
     */
    explicit operator bool() const;

    /**
     * @brief Explicit const conversion
     */
    explicit operator int() const;

    /**
     * @brief Explicit const conversion
     */
    explicit operator int64_t() const;

    /**
     * @brief Explicit const conversion
     */
    explicit operator double() const;

    /**
     * @brief Explicit const conversion
     */
    explicit operator const uint8_t*() const;

    /**
     * @brief Explicit conversion for types derived from VariantStorageClient
     */
    template<typename T, typename std::enable_if_t<std::is_base_of_v<VariantStorageClient, T>, int> = 0>
    operator const T&() const
    {

        if (const auto data = dynamic_pointer_cast<T>(storageClient());
            data)
        {
            return *data;
        }
        throw std::invalid_argument("Invalid type");
    }

    /**
     * @brief Get data for types that are not a pointer
     * @tparam T                Data type
     * @return reference to data
     */
    template<typename T, typename std::enable_if_t<!std::is_pointer_v<T>, int> = 0>
    T& get()
    {
        return (T&) *this;
    }

    /**
     * @brief Get data for double
     * @tparam T                Data type
     * @return reference to data
     */
    template<typename T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
    T get() const
    {
        return (T) * this;
    }

    /**
     * @brief Get data for types that are derived from VariantStorageClient
     * @tparam T                Data type
     * @return reference to data
     */
    template<typename T, typename std::enable_if_t<std::is_base_of_v<VariantStorageClient, T>, int> = 0>
    const T& get() const
    {
        return (const T&) *this;
    }

    /**
     * Set internal data
     * @param value             Internal data
     */
    template<typename T>
    void set(const T& value, typename std::enable_if_t<!std::is_pointer_v<T>, int> = 0)
    {
        *this = value;
    }

    /**
     * @brief Explicit conversion
     * @return Data reference
     */
    explicit operator bool&();

    /**
     * @brief Explicit conversion
     * @return Data reference
     */
    explicit operator int&();

    /**
     * @brief Explicit conversion
     * @return Data reference
     */
    explicit operator int64_t&();

    /**
     * @brief Explicit conversion
     * @return Data reference
     */
    explicit operator double&();

    /**
     * @brief Explicit conversion for types derived from VariantStorageClient
     * @return Data reference
     */
    template<typename T, typename std::enable_if_t<std::is_base_of_v<VariantStorageClient, T>, int> = 0>
    operator T&()
    {
        if (std::dynamic_pointer_cast<T>(storageClient()))
        {
            return *dynamic_pointer_cast<T>(storageClient());
        }
        throw std::invalid_argument("Invalid type");
    }

    /**
     * Copy assigment
     * @param other             Other object
     */
    VariantStorage& operator=(const VariantStorage& other);

    /**
     * Move assigment
     * @param other             Other object
     */
    VariantStorage& operator=(VariantStorage&& other) noexcept;

    /**
     * @brief Assignment
     * @param value             Data to assign
     * @return self
     */
    VariantStorage& operator=(bool value);

    /**
     * @brief Assignment
     * @param value             Data to assign
     * @return self
     */
    VariantStorage& operator=(int value);

    /**
     * @brief Assignment
     * @param value             Data to assign
     * @return self
     */
    VariantStorage& operator=(int64_t value);

    /**
     * @brief Assignment
     * @param value             Data to assign
     * @return self
     */
    VariantStorage& operator=(double value);

    /**
     * @brief Assignment
     * @param value             Data to assign
     * @return self
     */
    VariantStorage& operator=(const uint8_t*) = delete;

    /**
     * @brief Assignment for types derived from VariantStorageClient
     * @param value             Data to assign
     * @return self
     */
    template<typename T, typename std::enable_if_t<std::is_base_of_v<VariantStorageClient, T>, int> = 0>
    VariantStorage& operator=(const T& value)
    {
        if (type().type != T::variantDataType() || !storageClient())
        {
            setStorageClient(std::make_shared<T>(value));
        }
        else
        {
            *dynamic_pointer_cast<T>(storageClient()) = value;
        }
        setNull(false, T::variantDataType());
        setSize(value.dataSize());
        return *this;
    }

    /**
     * @brief Move assignment for Buffer
     * @param buffer             Data to assign
     * @return self
     */
    VariantStorage& operator=(Buffer&& buffer);

    void setExternalBuffer(const uint8_t* aValue, size_t dataSize, VariantDataType type = VariantDataType::VAR_BYTE_POINTER);
};

} // namespace sptk
