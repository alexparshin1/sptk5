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

#include <sptk5/VariantStorageClient.h>

namespace sptk {

/**
 * Money data (internal).
 *
 * A combination of integer quantity and scale - positive integer presenting power of ten for divider.
 * A money value is quantity / 10^(scale)
 */
class SP_EXPORT MoneyData : public VariantStorageClient
{
public:
    static const std::array<int64_t, 16> dividers; ///< Dividers that help formatting money data
    int64_t quantity;                              ///< Integer value
    uint8_t scale;                                 ///< Scale

    /**
     * Constructor
     * @param quantity          Money value
     * @param scale             Money value scale (signs after decimal point)
     */
    explicit MoneyData(int64_t quantity = 0, uint8_t scale = 0)
        : VariantStorageClient(VariantDataType::VAR_MONEY)
        , quantity(quantity)
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

} // namespace sptk
