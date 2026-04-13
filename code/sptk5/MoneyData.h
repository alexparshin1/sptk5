/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/VariantStorageClient.h>

namespace sptk {

/**
 * @brief Money data (internal).
 *
 * A combination of integer quantity and scale - positive integer presenting power of ten for divider.
 * A money value is quantity / 10^(scale).
 */
class SP_EXPORT MoneyData : public VariantStorageClient
{
public:
    /**
     * @brief Constructor.
     * @param quantity          Money value.
     * @param scale             Money value scale (signs after decimal point).
     */
    explicit MoneyData(int64_t quantity = 0, uint8_t scale = 0)
        : m_quantity(quantity)
        , m_scale(scale)
    {
        if (scale >= m_dividers.size())
        {
            throw std::out_of_range("MoneyData: scale is out of range");
        }
    }

    static VariantDataType variantDataType()
    {
        return VariantDataType::VAR_MONEY;
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
     * @brief Convert to integer value.
     */
    explicit operator int32_t() const;

    /**
     * @brief Convert to bool value.
     */
    explicit operator bool() const;

    size_t dataSize() const override
    {
        return sizeof(MoneyData);
    }

    int64_t quantity() const
    {
        return m_quantity;
    }

    void setQuantity(int64_t quantity)
    {
        m_quantity = quantity;
    }

    uint8_t scale() const
    {
        return m_scale;
    }

    void setScale(uint8_t scale)
    {
        if (scale >= m_dividers.size())
        {
            throw std::out_of_range("MoneyData: scale is out of range");
        }
        m_scale = scale;
    }

    static int64_t divider(uint8_t scale)
    {
        if (scale >= m_dividers.size())
        {
            throw std::out_of_range("MoneyData: scale is out of range");
        }
        return m_dividers[scale];
    }

private:
    static const std::array<int64_t, 16> m_dividers; ///< Dividers for formatting money data.
    int64_t                              m_quantity; ///< Integer value.
    uint8_t                              m_scale;    ///< Scale.
};

} // namespace sptk
