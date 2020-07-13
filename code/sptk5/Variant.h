/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_VARIANT_H__
#define __SPTK_VARIANT_H__

#include <sptk5/sptk.h>
#include <sptk5/DateTime.h>
#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/VariantData.h>

namespace sptk {

namespace xml {
    class Node;
}

namespace json {
    class Element;
}

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Variant types
 */
enum VariantType : uint32_t {
    /**
     * Undefined
     */
    VAR_NONE      = 0,

    /**
     * Integer
     */
    VAR_INT       = 1,

    /**
     * Floating-point (double)
     */
    VAR_FLOAT     = 2,

    /**
     * Special (integer quantity and scale) money
     */
    VAR_MONEY     = 4,

    /**
     * String pointer
     */
    VAR_STRING    = 8,

    /**
     * String pointer, corresponding to BLOBS in database
     */
    VAR_TEXT      = 16,

    /**
     * Data pointer, corresponding to BLOBS in database
     */
    VAR_BUFFER    = 32,

    /**
     * DateTime (double)
     */
    VAR_DATE      = 64,

    /**
     * DateTime (double)
     */
    VAR_DATE_TIME = 128,

    /**
     * Image pointer
     */
    VAR_IMAGE_PTR = 256,

    /**
     * Image index in object-specific table of image pointers
     */
    VAR_IMAGE_NDX = 512,

    /**
     * 64bit integer
     */
    VAR_INT64     = 1024,

    /**
     * Boolean
     */
    VAR_BOOL      = 2048

};

/**
 * FLAG: External const memory buffer, memory isn't managed
 */
constexpr int VAR_EXTERNAL_BUFFER = 16384;

/**
 * FLAG: Data is NULL
 */
constexpr int VAR_NULL = 32768;

/**
 * MASK: All the known field types w/o flags
 */
constexpr int VAR_TYPES = 16383;

class Field;

class SP_EXPORT BaseVariant
{
    friend class VariantAdaptors;

public:

    /**
     * Returns the data type
     */
    VariantType dataType() const;

    /**
     * Returns the data size
     */
    size_t dataSize() const;

    /**
     * Sets the data size
     * @param ds                Data size (in bytes).
     */
    void dataSize(size_t ds);

    /**
     * Returns the allocated buffer size
     */
    size_t bufferSize() const;

    /**
     * Returns the internal buffer
     */
    void* dataBuffer() const;

    /**
     * Null flag
     *
     * Returns true if the NULL state is set
     */
    bool isNull() const;
    /**
     * Returns a name for a particular variant type
     * @param type              Variant type
     */
    static String typeName(VariantType type);

    /**
     * Returns a type for a particular variant type name
     * @param name              Variant type name
     */
    static VariantType nameType(const char* name);

    /**
     * Directly reads the internal data
     */
    virtual bool getBool() const;

    /**
     * Directly reads the internal data
     */
    virtual const int32_t& getInteger() const;

    /**
     * Directly reads the internal data
     */
    virtual const int64_t& getInt64() const;

    /**
     * Directly reads the internal data
     */
    virtual const double& getFloat() const;

    /**
     * Directly reads the internal data
     */
    virtual const MoneyData& getMoney() const;

    /**
     * Directly reads the internal data
     */
    virtual const char* getString() const;

    /**
     * Directly reads the internal data
     */
    virtual char* getBuffer() const;

    /**
     * Directly reads the internal data
     */
    virtual const char* getText() const;

    /**
     * Directly reads the internal data
     */
    virtual DateTime getDateTime() const;

    /**
     * Directly reads the internal data
     */
    virtual DateTime getDate() const;

    /**
     * Directly reads the internal data
     */
    virtual const void* getImagePtr() const;

    /**
     * Directly reads the internal data
     */
    virtual uint32_t getImageNdx() const;

protected:

    VariantData             m_data;                 ///< Internal variant data storage
    size_t                  m_dataSize {0};         ///< Data size
    uint16_t                m_dataType {VAR_NONE};  ///< Data type

    /**
     * Releases allocated buffer (if any)
     */
    void releaseBuffers();

    /**
     * Sets the data type
     */
    void dataType(uint32_t dt);

    /**
     * @return True if current data type is external buffer
     */
    bool isExternalBuffer() const
    {
        return (m_dataType & VAR_EXTERNAL_BUFFER) != 0;
    }

    /**
     * Return money data as string
     * @param printBuffer      Internal print buffer
     * @param printBufferSize   Internal print buffer size
     * @return
     */
    String getMoneyString(char* printBuffer, size_t printBufferSize) const;
};

/**
 * Variant set methods and adaptors
 * 22 methods
 */
class SP_EXPORT VariantAdaptors : public BaseVariant
{
public:

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
    virtual void setBuffer(const void* value, size_t sz, VariantType type=VAR_BUFFER);

    /**
     * Assignment method
     */
    virtual void setExternalBuffer(void* value, size_t sz, VariantType type=VAR_BUFFER);

    /**
     * Assignment method
     */
    virtual void setDateTime(DateTime value, bool dateOnly=false);

    /**
     * Assignment method
     */
    virtual void setImagePtr(const void *value);

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
    virtual void setNull(VariantType vtype=VAR_NONE);

    /**
     * Conversion method
     *
     * Converts variant value to double.
     */
    int asInteger() const;

    /**
     * Conversion method
     *
     * Converts variant value to double.
     */
    int64_t asInt64() const;

    /**
     * Conversion to bool
     *
     * Converts variant string value with first char one of 'Y','y','T','t' to true,
     * and one of 'N','n','F','f' to false.
     * For the integer and float values, the value <=0 is false, and > 0 is true.
     */
    bool asBool() const;

    /**
     * Conversion to double
     *
     * Converts variant value to double.
     */
    double asFloat() const;

    /**
     * Conversion to string
     *
     * Converts variant value to string.
     */
    virtual String asString() const;

    /**
     * Conversion method
     *
     * Converts variant value to DateTime. The time part of CDdatetime is empty.
     */
    DateTime asDate() const;

    /**
     * Conversion method
     *
     * Converts variant value to DateTime.
     */
    DateTime asDateTime() const;

    /**
     * Conversion method
     *
     * Simply returns the internal data pointer for string/text/blob types.
     * For incompatible types throws an exception.
     */
    const void *asImagePtr() const;

protected:

    /**
     * Copies data from another CVariant
     */
    void setData(const BaseVariant& C);
};

/**
 * Universal data storage.
 *
 * Reasonably compact an fast class what allows storing data of different
 * types. It also allows conversions to and from supported types.
 */
class SP_EXPORT Variant : public VariantAdaptors
{
public:

    /**
     * Constructor
     */
    Variant();

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
    Variant(const char * value);

    /**
     * Constructor
     */
    Variant(const String& v);

    /**
     * Constructor
     */
    Variant(const DateTime& v);

    /**
     * Constructor
     * @param value             Buffer to copy from
     * @param sz                Buffer size
     */
    Variant(const void * value, size_t sz);

    /**
     * Constructor
     * @param value             Buffer to copy from
     */
    Variant(const Buffer& value);

    /**
     * Copy constructor
     * @param other             Other object
     */
    explicit Variant(const Variant& other);

    /**
     * Move constructor
     * @param other             Other object
     */
    Variant(Variant&& other) noexcept;

    /**
     * Destructor
     */
    virtual ~Variant();

    /**
     * Assignment operator
     * @param other             Other object
     */
    Variant& operator =(const Variant& other);

    /**
     * Assignment operator
     * @param other             Other object
     */
    Variant& operator =(Variant&& other) noexcept;

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(int32_t value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(int64_t value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(double value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(const MoneyData& value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(const char * value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(const String& value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(DateTime value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(const void *value);

    /**
     * Assignment operator
     * @param value             Value to assign
     */
    virtual Variant& operator =(const Buffer& value);

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
     * @param node              XML node to load data from
     */
    virtual void load(const xml::Node* node);

    /**
     * Loads the data from JSON element
     * @param node              JSON element to load data from
     */
    virtual void load(const json::Element* node);

    /**
     * Saves the data into XML node
     * @param node              XML node to save data into
     */
    void save(xml::Node* node) const;

    /**
     * Saves the data into JSON element
     * @param node              JSON element to save data into
     */
    void save(json::Element* node) const;
};

/**
 * Vector of Variant objects
 */
typedef std::vector<Variant>    VariantVector;

/**
 * @}
 */
}
#endif
