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
│   You shouldint have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include "DateTime.h"
#include <sptk5/Buffer.h>

namespace sptk {

/// @brief Compact variant data storage
///
/// Unlike std::variant that uses combined space for all variant types,
/// this class shares the space for included data types, taking just 16 bytes of memory
/// (dynamically allocated memory for Buffer or DateTime is not included)
class VariantStorage
{
public:
    enum class Type
    {
        Null,
        Integer,
        Double,
        Buffer,
        DateTime
    };

    /**
     * @brief Constructor
     */
    VariantStorage() = default;

    VariantStorage(const VariantStorage& other);
    VariantStorage(VariantStorage&& other);
    VariantStorage(int value);
    VariantStorage(int64_t value);
    VariantStorage(double value);
    VariantStorage(const Buffer& value);
    VariantStorage(const DateTime& value);

    /**
     * @brief Destructor
     */
    ~VariantStorage();

    Type type() const;

    explicit operator int() const;
    explicit operator int64_t() const;
    explicit operator double() const;
    explicit operator const Buffer&() const;
    explicit operator const DateTime&() const;

    void reset();

    VariantStorage& operator=(const VariantStorage& other);
    VariantStorage& operator=(VariantStorage&& other);
    VariantStorage& operator=(int value);
    VariantStorage& operator=(int64_t value);
    VariantStorage& operator=(double value);
    VariantStorage& operator=(const Buffer& value);
    VariantStorage& operator=(const DateTime& value);

private:
    union VariantValue
    {
        int64_t asInteger;
        double asDouble;
        Buffer* asBuffer;
        DateTime* asDateTime;
    };

    VariantValue m_value {};
    Type m_type {Type::Null};
};

} // namespace sptk
