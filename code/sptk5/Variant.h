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
#include <sptk5/Exception.h>
#include <sptk5/VariantData.h>
#include <sptk5/sptk.h>

#include <utility>

namespace sptk {

namespace xdoc {
class Node;
}

/**
 * @addtogroup utility Utility Classes
 * @{
 */

class Field;

class SP_EXPORT BaseVariant
{
    friend class VariantAdaptors;

public:
    /**
     * @brief Default constructor
     */
    BaseVariant() = default;

    /**
     * @brief Copy constructor
     * @param other             The other object
     */
    BaseVariant(const BaseVariant& other) = default;

    /**
     * @brief Move constructor
     * @param other             The other object
     */
    BaseVariant(BaseVariant&& other) noexcept = default;

    /**
     * @brief Destructor
     */
    virtual ~BaseVariant() = default;

    /**
     * Returns the data type
     */
    [[nodiscard]] VariantDataType dataType() const;

    /**
     * Returns the data size
     */
    [[nodiscard]] size_t dataSize() const;

    /**
     * Sets the data size
     * @param newDataSize                Data size (in bytes).
     */
    void dataSize(size_t newDataSize);

    /**
     * Returns the allocated buffer size
     */
    [[nodiscard]] size_t bufferSize() const;

    /**
     * Null flag
     *
     * Returns true if the NULL state is set
     */
    [[nodiscard]] bool isNull() const;

    /**
     * Returns a name for a particular variant type
     * @param type              Variant type
     */
    [[nodiscard]] static String typeName(VariantDataType type);

    /**
     * Returns a type for a particular variant type name
     * @param name              Variant type name
     */
    [[nodiscard]] static VariantDataType nameType(const char* name);

    /**
     * @brief Direct and fast const access to variant data
     * @tparam T variant data type
     * @return const refernce to variant data
     */
    template<typename T>
    const T& get() const
    {
        return m_data.get<T>();
    }

    /**
     * @brief Direct and fast access to variant data
     * @tparam T variant data type
     * @return refernce to variant data
     */
    template<typename T>
    T& get()
    {
        return m_data.get<T>();
    }

    /**
     * Directly reads the internal data
     */
    [[nodiscard]] virtual const MoneyData& getMoney() const;

    /**
     * Directly reads the internal data
     */
    [[nodiscard]] virtual const char* getString() const;

    /**
     * Directly reads the internal data
     */
    [[nodiscard]] virtual const uint8_t* getExternalBuffer() const;

    /**
     * Directly reads the internal data
     */
    [[nodiscard]] virtual const char* getText() const;

    /**
     * Directly reads the internal data
     */
    [[nodiscard]] virtual const uint8_t* getImagePtr() const;

    /**
     * Directly reads the internal data
     */
    [[nodiscard]] virtual uint32_t getImageNdx() const;

protected:
    /**
     * Sets the data type
     */
    void dataType(VariantType newDataType);

    /**
     * Sets the data type
     */
    void dataType(VariantDataType newDataType);

    /**
     * @return True if current data type is external buffer
     */
    [[nodiscard]] bool isExternalBuffer() const
    {
        return m_data.type().isExternalBuffer;
    }

    /**
     * Return money data as string
     * @return
     */
    [[nodiscard]] virtual String moneyDataToString() const;

    VariantData m_data; ///< Internal variant data storage
};

/**
 * Variant set methods and adaptors
 * 22 methods
 */
class SP_EXPORT VariantAdaptors
    : public BaseVariant
{
public:
    /**
     * @brief Default constructor
     */
    VariantAdaptors() = default;

    /**
     * @brief Copy constructor
     * @param other             The other object
     */
    VariantAdaptors(const VariantAdaptors& other) = default;

    /**
     * @brief Move constructor
     * @param other             The other object
     */
    VariantAdaptors(VariantAdaptors&& other) noexcept = default;

    /**
     * @brief Default destructor
     */
    ~VariantAdaptors() override = default;

    /**
     * Assignment method
     */
    virtual void setBool(bool value);

    /**
     * Assignment method
     */
    virtual void setInteger(int32_t value);

    /**
     * Assignment method
     */
    virtual void setInt64(int64_t value);

    /**
     * Assignment method
     */
    virtual void setFloat(double value);

    /**
     * Assignment method
     */
    virtual void setMoney(int64_t value, unsigned scale);

    /**
     * Assignment method
     */
    virtual void setString(const String& value);

    /**
     * Assignment method
     */
    virtual void setBuffer(const uint8_t* value, size_t valueSize, VariantDataType type = VariantDataType::VAR_BUFFER);

    /**
     * Assignment method
     */
    virtual void setExternalBuffer(uint8_t* value, size_t valueSize, VariantDataType type = VariantDataType::VAR_BUFFER);

    /**
     * Assignment method
     */
    virtual void setDateTime(DateTime value, bool dateOnly = false);

    /**
     * Assignment method
     */
    virtual void setImagePtr(const uint8_t* value);

    /**
     * Assignment method
     */
    virtual void setImageNdx(uint32_t value);

    /**
     * Assignment method
     */
    virtual void setMoney(const MoneyData& value);

    /**
     * Sets the NULL state
     *
     * Useful for the database operations.
     * Releases the memory allocated for string/text/blob types.
     * Sets the data to zero(s).
     * @param vtype             Optional variant type to enforce
     */
    virtual void setNull(VariantDataType vtype = VariantDataType::VAR_NONE);

    /**
     * Conversion method
     *
     * Converts variant value to double.
     */
    [[nodiscard]] int asInteger() const;

    /**
     * Conversion method
     *
     * Converts variant value to double.
     */
    [[nodiscard]] int64_t asInt64() const;

    /**
     * Conversion to bool
     *
     * Converts variant string value with first char one of 'Y','y','T','t' to true,
     * and one of 'N','n','F','f' to false.
     * For the integer and float values, the value <=0 is false, and > 0 is true.
     */
    [[nodiscard]] bool asBool() const;

    /**
     * Conversion to double
     *
     * Converts variant value to double.
     */
    [[nodiscard]] double asFloat() const;

    /**
     * Conversion to string
     *
     * Converts variant value to string.
     */
    [[nodiscard]] virtual String asString() const;

    /**
     * Conversion method
     *
     * Converts variant value to DateTime. The time part of CDdatetime is empty.
     */
    [[nodiscard]] DateTime asDate() const;

    /**
     * Conversion method
     *
     * Converts variant value to DateTime.
     */
    [[nodiscard]] DateTime asDateTime() const;

    /**
     * Conversion method
     *
     * Simply returns the internal data pointer for string/text/blob types.
     * For incompatible types throws an exception.
     */
    const uint8_t* asImagePtr() const;

protected:
    /**
     * Copies data from another CVariant
     */
    void setData(const BaseVariant& other);
    [[nodiscard]] const char* getBufferPtr() const;
};

/**
 * Universal data storage.
 *
 * Reasonably compact an fast class what allows storing data of different
 * types. It also allows conversions to and from supported types.
 */
class SP_EXPORT Variant
    : public VariantAdaptors
{
public:
    /**
     * Constructor
     */
    Variant();

    /**
     * Constructor
     */
    Variant(bool value);

    /**
     * Constructor
     */
    Variant(int32_t value);

    /**
     * Constructor
     */
    Variant(int64_t value, unsigned scale = 1);

    /**
     * Constructor
     */
    Variant(double value);

    /**
     * Constructor
     */
    Variant(const char* value);

    /**
     * Constructor
     */
    Variant(const String& value);

    /**
     * Constructor
     */
    Variant(const DateTime& v);

    /**
     * Constructor
     * @param value             Buffer to copy from
     * @param valueSize                Buffer size
     */
    Variant(const uint8_t* value, size_t valueSize);

    /**
     * Constructor
     * @param value             Buffer to copy from
     */
    Variant(const Buffer& value);

    /**
     * Copy constructor
     * @param other             Other object
     */
    explicit Variant(const Variant& other) = default;

    /**
     * Move constructor
     * @param other             Other object
     */
    Variant(Variant&& other) noexcept = default;

    /**
     * Destructor
     */
    ~Variant() override;

    /**
     * Assignment operator
     * @param other             Other object
     */
    Variant& operator=(const Variant& other);

    /**
     * Assignment operator
     * @param other             Other object
     */
    Variant& operator=(Variant&& other) noexcept;

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(bool value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(int32_t value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(int64_t value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(double value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(const MoneyData& value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(const char* value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(const String& value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(DateTime value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(const uint8_t* value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator=(const Buffer& value);

    /**
     * Conversion operator
     */
    virtual explicit operator bool() const;

    /**
     * Conversion operator
     */
    virtual explicit operator int() const;

    /**
     * Conversion operator
     */
    virtual explicit operator int64_t() const;

    /**
     * Conversion operator
     */
    virtual explicit operator uint64_t() const;

    /**
     * Conversion operator
     */
    virtual explicit operator double() const;

    /**
     * Conversion operator
     */
    virtual explicit operator String() const;

    /**
     * Conversion operator
     */
    virtual explicit operator DateTime() const;

    /**
     * Loads the data from XML node
     * @param element              XML node to load data from
     */
    virtual void load(const std::shared_ptr<xdoc::Node>& element);

    /**
     * Saves the data into XML node
     * @param node              XML node to save data into
     */
    void save(const std::shared_ptr<xdoc::Node>& node) const;
};

/**
 * Vector of Variant objects
 */
using VariantVector = std::vector<Variant>;

/**
 * @}
 */
} // namespace sptk
