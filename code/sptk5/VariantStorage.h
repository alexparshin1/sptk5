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

class SP_EXPORT BaseVariantStorage
{
public:
    /**
     * @brief Constructor
     */
    BaseVariantStorage() = default;

    BaseVariantStorage(const BaseVariantStorage& other, int);
    BaseVariantStorage(BaseVariantStorage&& other) noexcept = default;

    explicit BaseVariantStorage(bool value);
    explicit BaseVariantStorage(int value);
    explicit BaseVariantStorage(int64_t value);
    explicit BaseVariantStorage(double value);
    explicit BaseVariantStorage(const uint8_t* value);

    template<typename T, typename std::enable_if_t<std::is_class_v<T>, int> = 0>
    explicit BaseVariantStorage(const T& value)
        : m_class(std::make_shared<T>(value))
        , m_type(T::variantDataType())
        , m_null(false)
    {
    }

    explicit BaseVariantStorage(Buffer&& value);

    /**
     * @brief Destructor
     */
    virtual ~BaseVariantStorage() = default;

    [[nodiscard]] VariantDataType type() const
    {
        return m_type;
    }

    [[nodiscard]] bool isNull() const
    {
        return m_null;
    }

    void setNull();

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

    void setType(VariantDataType dataType)
    {
        m_type = dataType;
    }

    void setNull(bool isNull)
    {
        m_null = isNull;
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
    VariantDataType m_type {VariantDataType::VAR_NONE};
    bool m_null {true};
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

    VariantStorage(const VariantStorage& other)
        : BaseVariantStorage(other, 0)
    {
    }

    ~VariantStorage() override = default;

    explicit operator bool() const;
    explicit operator int() const;
    explicit operator int64_t() const;
    explicit operator double() const;
    explicit operator const uint8_t*() const;

    template<typename T, typename std::enable_if_t<std::is_class_v<T>, int> = 0>
    operator const T&() const
    {
        if (type() == T::variantDataType())
        {
            return *dynamic_pointer_cast<T>(storageClient());
        }
        throw std::invalid_argument("Invalid type");
    }

    template<typename T>
    T get() const
    {
        return (T) * this;
    }

    template<typename T>
    T& get()
    {
        return (T&) *this;
    }

    template<typename T>
    const T& get() const
    {
        return (const T&) *this;
    }

    /**
     * Set internal data
     * @param value             Internal data
     */
    template<typename T>
    void set(const T& value)
    {
        *this = value;
    }

    explicit operator bool&();
    explicit operator int&();
    explicit operator int64_t&();
    explicit operator double&();

    template<typename T, typename std::enable_if_t<std::is_class_v<T>, int> = 0>
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

    VariantStorage& operator=(bool value);
    VariantStorage& operator=(int value);
    VariantStorage& operator=(int64_t value);
    VariantStorage& operator=(double value);
    VariantStorage& operator=(const uint8_t* value);

    template<typename T, typename std::enable_if_t<std::is_class_v<T>, int> = 0>
    VariantStorage& operator=(const T& value)
    {
        if (type() != T::variantDataType() || !storageClient())
        {
            setStorageClient(std::make_shared<T>(value));
            setType(T::variantDataType());
        }
        else
        {
            *dynamic_pointer_cast<T>(storageClient()) = value;
        }
        setNull(false);
        return *this;
    }

    VariantStorage& operator=(Buffer&& value);
};

} // namespace sptk
