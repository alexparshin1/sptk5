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

#include "MoneyData.h"


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
 * @addtogroup utility Utility Classes.
 * @{
 */

class Field;

/**
 * @brief Base class for variant storage.
 */
class SP_EXPORT BaseVariant
{
    friend class VariantAdaptors;

public:
    /**
     * @brief Default constructor.
     */
    BaseVariant() = default;

    /**
     * @brief Copy constructor.
     * @param other             The other object.
     */
    BaseVariant(const BaseVariant& other) = default;

    /**
     * @brief Move constructor.
     * @param other             The other object.
     */
    BaseVariant(BaseVariant&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    virtual ~BaseVariant() = default;

    /**
     * @brief Returns the data type.
     */
    [[nodiscard]] VariantDataType dataType() const;

    /**
     * @brief Returns the data size.
     */
    [[nodiscard]] size_t dataSize() const;

    /**
     * @brief Sets the data size.
     * @param newDataSize                Data size (in bytes.).
     */
    void dataSize(size_t newDataSize);

    /**
     * @brief Returns the allocated buffer size.
     */
    [[nodiscard]] size_t bufferSize() const;

    /**
     * @brief Null flag.
     *
     * Returns true if the NULL state is set.
     */
    [[nodiscard]] bool isNull() const;

    /**
     * @brief Returns a name for a particular variant type.
     * @param type              Variant type.
     */
    [[nodiscard]] static String typeName(VariantDataType type);

    /**
     * @brief Returns a type for a particular variant type name.
     * @param name              Variant type name.
     */
    [[nodiscard]] static VariantDataType nameType(const char* name);

    /**
     * @brief Direct and fast const access to variant data.
     * @tparam T variant data type.
     * @return const reference to variant data.
     */
    template<typename T, std::enable_if_t<!std::is_class_v<T>, int> = 0>
    T get() const
    {
        return (T) m_data;
    }

    /**
     * @brief Direct and fast const access to variant data.
     * @tparam T variant data type.
     * @return const reference to variant data.
     */
    template<typename T, typename std::enable_if_t<std::is_class_v<T>, int> = 0>
    const T& get() const
    {
        return (const T&) m_data;
    }

    /**
     * @brief Direct and fast access to variant data.
     * @tparam T variant data type.
     * @return reference to variant data.
     */
    template<typename T>
    T& get()
    {
        return (T&) m_data;
    }

    /**
     * @brief Directly reads the internal data.
     */
    [[nodiscard]] virtual const MoneyData& getMoney() const;

    /**
     * @brief Directly reads the internal data.
     */
    [[nodiscard]] virtual const char* getString() const;

    /**
     * @brief Directly reads the internal data.
     */
    [[nodiscard]] virtual const uint8_t* getExternalBuffer() const;

    /**
     * @brief Directly reads the internal data.
     */
    [[nodiscard]] virtual const char* getText() const;

    /**
     * @brief Directly reads the internal data.
     */
    [[nodiscard]] virtual const uint8_t* getImagePtr() const;

    /**
     * @brief Directly reads the internal data.
     */
    [[nodiscard]] virtual uint32_t getImageNdx() const;

protected:
    /**
     * @brief Sets the data type.
     */
    void dataType(VariantType newDataType);

    /**
     * @brief Sets the data type.
     */
    void dataType(VariantDataType newDataType);

    /**
     * @return True if the current data type is 'external buffer'.
     */
    [[nodiscard]] bool isExternalBuffer() const
    {
        return m_data.type().isExternalBuffer;
    }

    /**
     * @brief Return money data as string.
     * @return String presentation of the money data.
     */
    [[nodiscard]] virtual String moneyDataToString() const;

    VariantData m_data; ///< Internal variant data storage
};

/**
 * @brief Variant set methods and adaptors.
 */
class SP_EXPORT VariantAdaptors
    : public BaseVariant
{
public:
    /**
     * @brief Default constructor.
     */
    VariantAdaptors() = default;

    /**
     * @brief Copy constructor.
     * @param other             The other object.
     */
    VariantAdaptors(const VariantAdaptors& other) = default;

    /**
     * @brief Move constructor.
     * @param other             The other object.
     */
    VariantAdaptors(VariantAdaptors&& other) noexcept = default;

    /**
     * @brief Default destructor.
     */
    ~VariantAdaptors() override = default;

    /**
     * @brief Assignment method.
     */
    virtual void setBool(bool value);

    /**
     * @brief Assignment method.
     */
    virtual void setInteger(int32_t value);

    /**
     * @brief Assignment method.
     */
    virtual void setInt64(int64_t value);

    /**
     * @brief Assignment method.
     */
    virtual void setFloat(double value);

    /**
     * @brief Assignment method.
     */
    virtual void setMoney(int64_t value, unsigned scale);

    /**
     * @brief Assignment method.
     */
    virtual void setString(const String& value);

    /**
     * @brief Assignment method.
     */
    virtual void setBuffer(const uint8_t* value, size_t valueSize, VariantDataType type);

    /**
     * @brief Assignment method.
     */
    virtual void setExternalBuffer(uint8_t* value, size_t valueSize, VariantDataType type);

    /**
     * @brief Assignment method.
     */
    virtual void setDateTime(const DateTime& value, bool dateOnly = false);

    /**
     * @brief Assignment method.
     */
    virtual void setImagePtr(const uint8_t* value);

    /**
     * @brief Assignment method.
     */
    virtual void setImageNdx(uint32_t value);

    /**
     * @brief Assignment method.
     */
    virtual void setMoney(const MoneyData& value);

    /**
     * @brief Sets the NULL state.
     *
     * Useful for the database operations.
     * Releases the memory allocated for string/text/blob types.
     * Sets the data to zero(s).
     * @param variantDataType             Optional variant type to enforce.
     */
    virtual void setNull(VariantDataType variantDataType = VariantDataType::VAR_NONE);

    /**
     * @brief Conversion method.
     *
     * Converts variant value to integer.
     */
    [[nodiscard]] int asInteger() const;

    /**
     * @brief Conversion method.
     *
     * @brief Converts variant value to 64-bit integer.
     */
    [[nodiscard]] int64_t asInt64() const;

    /**
     * @brief Conversion to bool.
     *
     * Converts variant string value with the first char one of 'Y','y','T','t' to true,
     * and one of 'N','n','F','f' to false.
     * For the integer and float values, the value <=0 is false, and > 0 is true.
     */
    [[nodiscard]] bool asBool() const;

    /**
     * @brief Conversion to double.
     *
     * Converts variant value to double.
     */
    [[nodiscard]] double asFloat() const;

    /**
     * @brief Conversion to string.
     *
     * Converts variant value to string.
     */
    [[nodiscard]] virtual String asString() const;

    /**
     * @brief Conversion to string.
     *
     * Converts variant value to string.
     */
    [[nodiscard]] virtual Buffer asBuffer() const;

    /**
     * @brief Conversion method.
     *
     * Converts variant value to DateTime. The time part of datetime is empty.
     */
    [[nodiscard]] DateTime asDate() const;

    /**
     * @brief Conversion method.
     *
     * Converts variant value to DateTime.
     */
    [[nodiscard]] DateTime asDateTime() const;

    /**
     * @brief Conversion method.
     *
     * Simply returns the internal data pointer for string/text/blob types.
     * For incompatible types throws an exception.
     */
    [[nodiscard]] const uint8_t* asImagePtr() const;

protected:
    /**
     * @brief Copies data from another CVariant.
     */
    void                           setData(const BaseVariant& other);
    [[nodiscard]] std::string_view getBufferPtr() const;
};

/**
 * @brief Universal data storage.
 *
 * Reasonably compact class what allows storing data of different
 * types. It also allows conversions to and from supported types.
 */
class SP_EXPORT Variant
    : public VariantAdaptors
{
public:
    /**
     * @brief Constructor.
     */
    Variant() = default;

    /**
     * @brief Constructor.
     */
    Variant(bool value);

    /**
     * @brief Constructor.
     */
    Variant(int32_t value);

    /**
     * @brief Constructor.
     */
    Variant(int64_t value, unsigned scale = 0);

    /**
     * @brief Constructor.
     */
    Variant(double value);

    /**
     * @brief Constructor.
     */
    Variant(const char* value);

    /**
     * @brief Constructor.
     */
    Variant(const String& value);

    /**
     * @brief Constructor.
     */
    Variant(const DateTime& dateTime);

    /**
     * @brief Constructor.
     * @param value             Buffer to copy from.
     * @param valueSize                Buffer size.
     */
    Variant(const uint8_t* value, size_t valueSize);

    /**
     * @brief Constructor.
     * @param value             Buffer to copy from.
     */
    Variant(const Buffer& value);

    /**
     * @brief Copy constructor
     * @param other             Other object.
     */
    Variant(const Variant& other) = default;

    /**
     * @brief Move constructor.
     * @param other             Other object.
     */
    Variant(Variant&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    ~Variant() override;

    /**
     * @brief Assignment operator.
     * @param other             Other object.
     */
    virtual Variant& operator=(const Variant& other);

    /**
     * @brief Assignment operator.
     * @param other             Other object.
     */
    Variant& operator=(Variant&& other) noexcept;

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(bool value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(int32_t value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(int64_t value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(double value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(const MoneyData& value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(const char* value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(const String& value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(const DateTime& value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(const uint8_t* value);

    /**
     * @brief Assignment operator.
     * @param value             Value to assign.
     */
    virtual Variant& operator=(const Buffer& value);

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator bool() const;

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator int() const;

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator int64_t() const;

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator uint64_t() const;

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator double() const;

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator String() const;

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator Buffer() const;

    /**
     * @brief Conversion operator.
     */
    virtual explicit operator DateTime() const;

    /**
     * @brief Loads the data from the XML node.
     * @param element              XML node to load data from.
     */
    virtual void load(const std::shared_ptr<xdoc::Node>& element);

    /**
     * @brief Saves the data into XML node.
     * @param node              XML node to save data into.
     */
    void save(const std::shared_ptr<xdoc::Node>& node) const;
};

/**
 * @brief Vector of Variant objects.
 */
using VariantVector = std::vector<Variant>;

/**
 * @}
 */
} // namespace sptk
