/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Variant.h - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/cxml>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Variant types
 */
enum VariantType {
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
#define VAR_EXTERNAL_BUFFER 16384

/**
 * FLAG: Data is NULL
 */
#define VAR_NULL            32768

/**
 * MASK: All the known field types w/o flags
 */
#define VAR_TYPES           16383

class Field;

/**
 * @brief Variant data buffer (internal).
 *
 * A buffer for data with the variable length like strings, or just generic buffers
 */
struct VariantDataBuffer
{
    /**
     * String or buffer pointer
     */
    char*        data;

    /**
     * Allocated buffer size
     */
    size_t       size;

};

/**
 * @brief Money data (internal).
 *
 * A combination of integer quantity and scale - positive integer presenting power of ten for divider.
 * A money value is quantity / 10^(scale)
 */
struct MoneyData
{
    static int64_t dividers[16];///< Dividers that help formatting money data

    /**
     * Integer value 
     */
    int64_t      quantity;

    /**
     * Scale (1..15)
     */
    uint8_t      scale:4;


    /**
     * Convert to double value
     */
    operator double () const;

    /**
     * Convert to integer value
     */
    operator int64_t () const;

    /**
     * Convert to integer value
     */
    operator int32_t () const;

    /**
     * Convert to bool value
     */
    operator bool () const;

};

/**
 * @brief Universal data storage.
 *
 * Reasonably compact an fast class what allows storing data of different
 * types. It also allows conversions to and from supported types.
 */
class SP_EXPORT Variant
{
protected:
    /**
     * Internal variant data storage type definition
     */
    typedef union variantData
    {
        /**
         * Boolean data
         */
        bool                boolData;

        /**
         * Integer data
         */
        int32_t             intData;

        /**
         * 64 bit integer data
         */
        int64_t             int64Data;

        /**
         * Floating point data
         */
        double              floatData;

        /**
         * DateTime data
         */
        double              timeData;

        /**
         * A buffer for data with the variable length like strings, or just generic buffers
         */
        VariantDataBuffer   buffer;

        /**
         * Image pointer
         */
        void*               imagePtr;

        /**
         * Image index in object-specific table of image pointers
         */
        int32_t             imageNdx;

        /**
         * Money data
         */
        MoneyData           moneyData;

    } VariantData;


    /**
     * Internal variant data storage
     */
    VariantData             m_data;

    /**
     * Data size
     */
    size_t   m_dataSize;

    /**
     * Data type
     */
    uint16_t m_dataType;


    /**
     * @brief Copies data from another CVariant
     */
    void setData(const Variant& C);

    /**
     * @brief Releases allocated buffer (if any)
     */
    void releaseBuffers();

protected:
    /**
     * @brief Sets the data size
     * @param ds size_t, data size (in bytes).
     */
    void dataSize(size_t ds);

    /**
     * @brief Sets the data type
     */
    void dataType(uint32_t dt);

public:

    /**
     * @brief Constructor
     */
    Variant();

    /**
     * @brief Constructor
     */
    Variant(int32_t value);

    /**
     * @brief Constructor
     */
    Variant(uint32_t value);

    /**
     * @brief Constructor
     */
    Variant(int64_t value, unsigned scale = 1);

    /**
     * @brief Constructor
     */
    Variant(uint64_t value);

    /**
     * @brief Constructor
     */
    Variant(float value);

    /**
     * @brief Constructor
     */
    Variant(double value);

    /**
     * @brief Constructor
     */
    Variant(const char * value);

    /**
     * @brief Constructor
     */
    Variant(const std::string& v);

    /**
     * @brief Constructor
     */
    Variant(DateTime v);

    /**
     * @brief Constructor
     */
    Variant(const void * value, size_t sz);

    /**
     * @brief Constructor
     */
    Variant(const Buffer& value);

    /**
     * @brief Constructor
     */
    Variant(const Variant& value);

    /**
     * @brief Destructor
     */
    virtual ~Variant();

    /**
     * @brief Assignment method
     */
    virtual void setBool(bool value);

    /**
     * @brief Assignment method
     */
    virtual void setInteger(int32_t value);

    /**
     * @brief Assignment method
     */
    virtual void setInt64(int64_t value);

    /**
     * @brief Assignment method
     */
    virtual void setFloat(double value);

    /**
     * @brief Assignment method
     */
    virtual void setMoney(int64_t value, unsigned scale);

    /**
     * @brief Assignment method
     */
    virtual void setString(const char * value, size_t maxlen = 0);

    /**
     * @brief Assignment method
     */
    virtual void setString(const std::string& value);

    /**
     * @brief Assignment method
     */
    void setExternalString(const char * value, int length = -1);

    /**
     * @brief Assignment method
     */
    void setExternalString(const std::string& value);

    /**
     * @brief Assignment method
     */
    virtual void setText(const char * value);

    /**
     * @brief Assignment method
     */
    virtual void setText(const std::string& str);

    /**
     * @brief Assignment method
     */
    virtual void setExternalText(const char * value);

    /**
     * @brief Assignment method
     */
    virtual void setBuffer(const void * value, size_t sz);

    /**
     * @brief Assignment method
     */
    virtual void setBuffer(const Buffer& value);

    /**
     * @brief Assignment method
     */
    virtual void setExternalBuffer(const void * value, size_t sz);

    /**
     * @brief Assignment method
     */
    virtual void setBuffer(const std::string& str);

    /**
     * @brief Assignment method
     */
    virtual void setDate(DateTime value);

    /**
     * @brief Assignment method
     */
    virtual void setDateTime(DateTime value);

    /**
     * @brief Assignment method
     */
    virtual void setImagePtr(const void *value);

    /**
     * @brief Assignment method
     */
    virtual void setImageNdx(uint32_t value);

    /**
     * @brief Assignment method
     */
    virtual void setMoney(const MoneyData& value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(const Variant &C);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(int32_t value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(int64_t value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(uint32_t value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(uint64_t value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(int16_t value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(uint16_t value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(float value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(double value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(const MoneyData& value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(const char * value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(const std::string& value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(DateTime value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(const void *value);

    /**
     * @brief Assignment operator
     */
    virtual Variant& operator =(const Buffer& value);

    /**
     * @brief Directly reads the internal data
     */
    virtual bool getBool() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual const int32_t& getInteger() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual const int64_t& getInt64() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual const double& getFloat() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual const MoneyData& getMoney() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual const char* getString() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual const char* getBuffer() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual const char* getText() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual DateTime getDateTime() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual DateTime getDate() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual void* getImagePtr() const;

    /**
     * @brief Directly reads the internal data
     */
    virtual uint32_t getImageNdx() const;

    /**
     * @brief Returns the data type
     */
    VariantType dataType() const;

    /**
     * @brief Returns the data size
     */
    size_t dataSize() const;

    /**
     * @brief Returns the allocated buffer size
     */
    size_t bufferSize() const;

    /**
     * @brief Returns the internal buffer
     */
    void* dataBuffer() const;

    /**
     * @brief Conversion operator
     */
    operator bool() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator int32_t() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator uint32_t() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator int64_t() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator uint64_t() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator float() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator double() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator std::string() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator String() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion operator
     */
    operator DateTime() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion method
     *
     * Converts variant value to double.
     */
    int32_t asInteger() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion method
     *
     * Converts variant value to double.
     */
    int64_t asInt64() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion to bool
     *
     * Converts variant string value with first char one of 'Y','y','T','t' to true,
     * and one of 'N','n','F','f' to false.
     * For the integer and float values, the value <=0 is false, and > 0 is true.
     */
    bool asBool() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion to double
     *
     * Converts variant value to double.
     */
    double asFloat() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion to string
     *
     * Converts variant value to string.
     */
    virtual std::string asString() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion method
     *
     * Converts variant value to DateTime. The time part of CDdatetime is empty.
     */
    DateTime asDate() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion method
     *
     * Converts variant value to DateTime.
     */
    DateTime asDateTime() const THROWS_EXCEPTIONS;

    /**
     * @brief Conversion method
     *
     * Simply returns the internal data pointer for string/text/blob types.
     * For incompatible types throws an exception.
     */
    void *asImagePtr() const THROWS_EXCEPTIONS;

    /**
     * @brief Sets the NULL state
     *
     * Useful for the database operations.
     * Releases the memory allocated for string/text/blob types.
     * Retains the data type. Sets the data to zero(s).
     * @param vtype VariantType, optional variant type to enforce
     */
    virtual void setNull(VariantType vtype=VAR_NONE);

    /**
     * @brief Null flag
     *
     * Returns true if the NULL state is set
     */
    bool isNull() const;

    /**
     * @brief Returns a name for a particular variant type
     * @param type VariantType, a variant type
     */
    static std::string typeName(VariantType type);

    /**
     * @brief Returns a type for a particular variant type name
     * @param name const char*, a variant type name
     */
    static VariantType nameType(const char* name);

    /**
     * @brief Loads the data from XML node
     * @param node const XMLNode&, XML node to load data from
     */
    void load(const XMLNode& node);

    /**
     * @brief Loads the data from XML node
     * @param node const XMLNode*, XML node to load data from
     */
    void load(const XMLNode* node);

    /**
     * @brief Saves the data into XML node
     * @param node XMLNode&, XML node to save data into
     */
    void save(XMLNode& node) const;

    /**
     * @brief Saves the data into XML node
     * @param node XMLNode*, XML node to save data into
     */
    void save(XMLNode* node) const;
};
/**
 * @}
 */
}
#endif
