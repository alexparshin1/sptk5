/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       VariantData.h - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 2 2018                                 ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_VARIANT_DATA_H__
#define __SPTK_VARIANT_DATA_H__

#include <sptk5/sptk.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Variant data buffer (internal).
 *
 * A buffer for data with the variable length like strings, or just generic buffers
 */
struct VariantDataBuffer
{
    char*        data;      ///< String or buffer pointer
    size_t       size;      ///< Allocated buffer size
};

/**
 * Money data (internal).
 *
 * A combination of integer quantity and scale - positive integer presenting power of ten for divider.
 * A money value is quantity / 10^(scale)
 */
class MoneyData
{
public:

    static int64_t dividers[16];        ///< Dividers that help formatting money data
    int64_t      quantity;              ///< Integer value
    uint8_t      scale:4;               ///< Scale (1..15)

    /**
     * Convert to double value
     */
    explicit operator double () const;

    /**
     * Convert to integer value
     */
    explicit operator int64_t () const;

    /**
     * Convert to integer value
     */
    explicit operator size_t () const;

    /**
     * Convert to integer value
     */
    explicit operator int32_t () const;

    /**
     * Convert to bool value
     */
    explicit operator bool () const;

};

class SP_EXPORT VariantData
{
    friend class Variant_SetMethods;

    uint8_t     m_data[32];         ///< Variant data BLOB

public:

    /**
     * Default constructor
     */
    VariantData()
    {
    }

    /**
     * Copy constructor
     * @param other             Other object
     */
    VariantData(const VariantData& other)
    {
        memcpy(m_data, other.m_data, sizeof(m_data));
    }

    /**
     * Move constructor
     * @param other             Other object
     */
    VariantData(VariantData&& other)
    {
        memcpy(m_data, other.m_data, sizeof(m_data));
        memset(other.m_data, 0, sizeof(m_data));
    }

    /**
     * Copy assigment
     * @param other             Other object
     */
    VariantData& operator = (const VariantData& other)
    {
        if (&other != this)
            memcpy(m_data, other.m_data, sizeof(m_data));
        return *this;
    }

    /**
     * Move assignment
     * @param other             Other object
     */
    VariantData& operator = (VariantData&& other)
    {
        if (&other != this) {
            memcpy(m_data, other.m_data, sizeof(m_data));
            memset(other.m_data, 0, sizeof(m_data));
        }
        return *this;
    }

    bool& getBool()                 ///< Boolean data
    {
        return *(bool*) m_data;
    }

    int32_t& getInteger()           ///< Integer data
    {
        return *(int32_t*) m_data;
    }

    int64_t& getInt64()             ///< 64 bit integer data
    {
        return *(int64_t*) m_data;
    }

    double& getFloat()              ///< Floating point data
    {
        return *(double*) m_data;
    }

    int64_t& getTime()              ///< DateTime data
    {
        return *(int64_t*) m_data;
    }

    VariantDataBuffer& getBuffer()  ///< A buffer for data with the variable length like strings, or just generic buffers
    {
        return *(VariantDataBuffer*) m_data;
    }

    void setImagePtr(void* ptr)             ///< Image pointer
    {
        size_t ptrSize = sizeof(ptr);
        memcpy(m_data, ptr, ptrSize);
    }

    MoneyData& getMoneyData()               ///< Money data
    {
        return *(MoneyData*) m_data;
    }


    const bool& getBool() const             ///< Boolean data
    {
        return *(bool*) m_data;
    }

    const int32_t& getInteger() const       ///< Integer data
    {
        return *(int32_t*) m_data;
    }

    const int64_t& getInt64() const         ///< 64 bit integer data
    {
        return *(int64_t*) m_data;
    }

    const double& getFloat() const          ///< Floating point data
    {
        return *(double*) m_data;
    }

    const int64_t getTime() const           ///< DateTime data
    {
        return *(int64_t*) m_data;
    }

    const VariantDataBuffer&getBuffer() const ///< A buffer for data with the variable length like strings, or just generic buffers
    {
        return *(VariantDataBuffer*) m_data;
    }

    const void* getImagePtr() const         ///< Image pointer
    {
        return (void*) m_data;
    }

    const MoneyData& getMoneyData() const     ///< Money data
    {
        return *(MoneyData*) m_data;
    }
};

/**
 * @}
 */
}
#endif
