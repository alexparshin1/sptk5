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

#include <memory>

namespace sptk {

/// @brief Compact variant data storage
///
/// Unlike std::variant that uses combined space for all variant types,
/// this class shares the space for included data types, taking just 16 bytes of memory
/// (dynamically allocated memory for Buffer or DateTime is not included)
class SP_EXPORT VariantStorage
{
public:
    enum class Type
    {
        VAR_NONE = 0,
        VAR_INT = 1,
        VAR_FLOAT = 2,
        VAR_BUFFER = 4,
        VAR_DATE_TIME = 8,
        VAR_MONEY = 16,
        VAR_BYTE_POINTER = 32,
        VAR_CHAR_POINTER = 64,
        VAR_STRING = 128,
        VAR_BOOL = 256,
        VAR_INT64 = 512
    };

    /**
     * @brief Constructor
     */
    VariantStorage() = default;

    VariantStorage(const VariantStorage& other);
    VariantStorage(VariantStorage&& other) noexcept;

    explicit VariantStorage(bool value);
    explicit VariantStorage(int value);
    explicit VariantStorage(int64_t value);
    explicit VariantStorage(double value);
    explicit VariantStorage(const Buffer& value);
    explicit VariantStorage(const String& value);
    explicit VariantStorage(const DateTime& value);
    explicit VariantStorage(const MoneyData& value);
    explicit VariantStorage(const uint8_t* value);
    explicit VariantStorage(const char* value);

    explicit VariantStorage(Buffer&& value);

    /**
     * @brief Destructor
     */
    ~VariantStorage() = default;

    [[nodiscard]] Type type() const {return m_type;}
    [[nodiscard]] bool isNull() const {return m_null;}

    void setNull();

    explicit operator bool() const;
    explicit operator int() const;
    explicit operator int64_t() const;
    explicit operator double() const;
    explicit operator const Buffer&() const;
    explicit operator const String&() const;
    explicit operator const DateTime&() const;
    explicit operator const MoneyData&() const;
    explicit operator const uint8_t*() const;
    explicit operator const char*() const;

    VariantStorage& operator=(const VariantStorage& other);
    VariantStorage& operator=(VariantStorage&& other) noexcept;
    VariantStorage& operator=(bool value);
    VariantStorage& operator=(int value);
    VariantStorage& operator=(int64_t value);
    VariantStorage& operator=(double value);
    VariantStorage& operator=(const Buffer& value);
    VariantStorage& operator=(const String& value);
    VariantStorage& operator=(const DateTime& value);
    VariantStorage& operator=(const MoneyData& value);
    VariantStorage& operator=(const uint8_t* value);
    VariantStorage& operator=(const char* value);

private:
    union VariantValue
    {
        int64_t asInteger;
        double asDouble;
        const uint8_t* asBytePointer;
        const char* asCharPointer;
    };

    VariantValue m_value {};
    std::shared_ptr<VariantStorageClient> m_class;
    Type m_type {Type::VAR_NONE};
    bool m_null {true};
};

} // namespace sptk
