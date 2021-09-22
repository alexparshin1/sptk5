/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/sptk.h>
#include <variant>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Variant types
 */
enum class VariantDataType : uint16_t
{
    VAR_NONE = 0,        ///< Undefined
    VAR_INT = 1,         ///< Integer
    VAR_FLOAT = 2,       ///< Floating-point (double)
    VAR_MONEY = 4,       ///< Special (integer quantity and scale) money
    VAR_STRING = 8,      ///< String pointer
    VAR_TEXT = 16,       ///< String pointer, corresponding to BLOBS in database
    VAR_BUFFER = 32,     ///< Data pointer, corresponding to BLOBS in database
    VAR_DATE = 64,       ///< DateTime (double)
    VAR_DATE_TIME = 128, ///< DateTime (double)
    VAR_IMAGE_PTR = 256, ///< Image pointer
    VAR_IMAGE_NDX = 512, ///< Image index in object-specific table of image pointers
    VAR_INT64 = 1024,    ///< 64bit integer
    VAR_BOOL = 2048      ///< Boolean
};

struct VariantType {
    VariantDataType type : 12;
    bool isNull : 1;
    bool isExternalBuffer : 1;
};

/**
 * Money data (internal).
 *
 * A combination of integer quantity and scale - positive integer presenting power of ten for divider.
 * A money value is quantity / 10^(scale)
 */
class SP_EXPORT MoneyData
{
public:
    static std::array<int64_t, 16> dividers; ///< Dividers that help formatting money data
    int64_t quantity;                        ///< Integer value
    uint8_t scale;                           ///< Scale

    /**
     * Constructor
     * @param quantity          Money value
     * @param scale             Money value scale (signs after decimal point)
     */
    MoneyData(int64_t quantity, uint8_t scale)
        : quantity(quantity)
        , scale(scale)
    {
    }

    /**
     * Convert to double value
     */
    explicit operator double() const;

    /**
     * Convert to integer value
     */
    explicit operator int64_t() const;

    /**
     * Convert to integer value
     */
    explicit operator int32_t() const;

    /**
     * Convert to bool value
     */
    explicit operator bool() const;
};

class SP_EXPORT VariantData
{
    friend class Variant_SetMethods;

public:
    using Storage = std::variant<bool, int32_t, int64_t, double, const uint8_t*, const void*, DateTime, Buffer, MoneyData>;

    /**
     * Default constructor
     */
    VariantData() = default;

    /**
     * Copy constructor
     * @param other             Other object
     */
    VariantData(const VariantData& other) = default;

    /**
     * Move constructor
     * @param other             Other object
     */
    VariantData(VariantData&& other) noexcept
        : m_storage(std::move(other.m_storage))
        , m_dataType(std::exchange(other.m_dataType, emptyType))
        , m_dataSize(std::exchange(other.m_dataSize, 0))
    {
    }

    virtual ~VariantData() noexcept = default;

    /**
     * Copy assigment
     * @param other             Other object
     */
    VariantData& operator=(const VariantData& other) = default;

    /**
     * Move assignment
     * @param other             Other object
     */
    VariantData& operator=(VariantData&& other) noexcept = default;

    /**
     * @brief Access to variant data
     * @return variant data reference
     */
    template<typename T>
    T& get()
    {
        return std::get<T>(m_storage);
    }

    /**
     * @brief Access to variant data
     * @return variant data reference
     */
    template<typename T>
    const T& get() const
    {
        return std::get<T>(m_storage);
    }

    /**
     * Set image pointer
     * @param ptr               Image pointer
     */
    template<typename T>
    void set(const T& value)
    {
        m_storage = value;
    }

    void type(VariantType dataType)
    {
        m_dataType = dataType;
    }

    void type(VariantDataType dataType)
    {
        m_dataType.type = dataType;
    }

    void setNull(bool isNull)
    {
        m_dataType.isNull = isNull;
    }

    VariantType type() const
    {
        return m_dataType;
    }

    void size(size_t dataSize)
    {
        m_dataSize = dataSize;
    }

    size_t size() const
    {
        return m_dataSize;
    }

private:
    static constexpr VariantType emptyType {VariantDataType::VAR_NONE, true, false};

    Storage m_storage;                  ///< Variant data
    VariantType m_dataType {emptyType}; ///< Variant type
    size_t m_dataSize {0};              ///< Data size
};

/**
 * @}
 */
} // namespace sptk
