/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CVariant.h  -  description
                             -------------------
    begin                : Tue Dec 14 1999
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CVARIANT_H__
#define __CVARIANT_H__

#include <sptk5/sptk.h>
#include <sptk5/CDateTime.h>
#include <sptk5/CBuffer.h>
#include <sptk5/CException.h>
#include <sptk5/cxml>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// Variant types
enum CVariantType {
    VAR_NONE      = 0,    ///< Undefined
    VAR_INT       = 1,    ///< Integer
    VAR_FLOAT     = 2,    ///< Floating-point (double)
    VAR_MONEY     = 4,    ///< Special (integer quantity and scale) money
    VAR_STRING    = 8,    ///< String pointer
    VAR_TEXT      = 16,   ///< String pointer, corresponding to BLOBS in database
    VAR_BUFFER    = 32,   ///< Data pointer, corresponding to BLOBS in database
    VAR_DATE      = 64,   ///< CDateTime (double)
    VAR_DATE_TIME = 128,  ///< CDateTime (double)
    VAR_IMAGE_PTR = 256,  ///< Image pointer
    VAR_IMAGE_NDX = 512,  ///< Image index in object-specific table of image pointers
    VAR_INT64     = 1024, ///< 64bit integer
    VAR_BOOL      = 2048  ///< Boolean
};

/// FLAG: External const memory buffer, memory isn't managed
#define VAR_EXTERNAL_BUFFER 16384

/// FLAG: Data is NULL
#define VAR_NULL            32768

/// MASK: All the known field types w/o flags
#define VAR_TYPES           16383

class CField;

/// @brief Variant data buffer (internal).
///
/// A buffer for data with the variable length like strings, or just generic buffers
struct CVariantDataBuffer
{
    char*        data;         ///< String or buffer pointer
    size_t       size;         ///< Allocated buffer size
};

/// @brief Money data (internal).
///
/// A combination of integer quantity and scale - positive integer presenting power of ten for divider.
/// A money value is quantity / 10^(scale)
struct CMoneyData
{
    static int64_t dividers[16];
    
    int64_t      quantity;     ///< Integer value 
    uint8_t      scale:4;      ///< Scale (1..15)
    
    operator double () const;  ///< Convert to double value
    operator int64_t () const; ///< Convert to integer value
    operator int32_t () const; ///< Convert to integer value
    operator bool () const;    ///< Convert to bool value
};

/// @brief Universal data storage.
///
/// Reasonably compact an fast class what allows storing data of different
/// types. It also allows conversions to and from supported types.
class SP_EXPORT CVariant
{
protected:
    /// @brief Internal variant data storage
    union variantData
    {
        bool                boolData;        ///< Boolean data
        int32_t             intData;         ///< Integer data
        int64_t             int64Data;       ///< 64 bit integer data
        double              floatData;       ///< Floating point data
        double              timeData;        ///< CDateTime data
        CVariantDataBuffer  buffer;          ///< A buffer for data with the variable length like strings, or just generic buffers
        void*               imagePtr;        ///< Image pointer
        int32_t             imageNdx;        ///< Image index in object-specific table of image pointers
        CMoneyData          moneyData;       ///< Money data
    } m_data;                    ///< Data storage union
    size_t   m_dataSize;         ///< Data size
    uint16_t m_dataType;         ///< Data type

    /// @brief Copies data from another CVariant
    void setData(const CVariant& C);

    /// @brief Releases allocated buffer (if any)
    void releaseBuffers();

protected:
    /// @brief Sets the data size
    ///
    /// If data size is 0, the NULL flag is set. Otherwise, the NULL flag is set to false.
    /// @param ds size_t, data size (in bytes).
    void dataSize(size_t ds)
    {
        m_dataSize = ds;
        if (m_dataSize == 0)
            m_dataType |= VAR_NULL;
        else
            m_dataType &= VAR_TYPES | VAR_EXTERNAL_BUFFER;
    }

    /// @brief Sets the data type
    void dataType(uint32_t dt)
    {
        m_dataType = (uint16_t) dt;
    }

public:

    /// @brief Constructor
    CVariant()
    {
        m_dataType = VAR_NONE | VAR_NULL;
        m_data.int64Data = 0;
    }

    /// @brief Constructor
    CVariant(int32_t value)
    {
        m_dataType = VAR_INT;
        m_data.intData = value;
    }

    /// @brief Constructor
    CVariant(uint32_t value)
    {
        m_dataType = VAR_INT;
        m_data.intData = (int32_t) value;
    }

    /// @brief Constructor
    CVariant(int64_t value, unsigned scale = 1)
    {
        if (scale > 1) {
            m_dataType = VAR_MONEY;
            m_data.moneyData.quantity = value;
            m_data.moneyData.scale = (uint8_t) scale;
        } else {
            m_dataType = VAR_INT64;
            m_data.int64Data = value;
        }
    }

    /// @brief Constructor
    CVariant(uint64_t value)
    {
        m_dataType = VAR_INT64;
        m_data.int64Data = (int64_t) value;
    }

    /// @brief Constructor
    CVariant(float value)
    {
        m_dataType = VAR_FLOAT;
        m_data.floatData = value;
    }

    /// @brief Constructor
    CVariant(double value)
    {
        m_dataType = VAR_FLOAT;
        m_data.floatData = value;
    }

    /// @brief Constructor
    CVariant(const char * value)
    {
        m_dataType = VAR_NONE;
        setString(value);
    }

    /// @brief Constructor
    CVariant(const std::string& v)
    {
        m_dataType = VAR_NONE;
        setString(v.c_str(), v.length());
    }

    /// @brief Constructor
    CVariant(CDateTime v)
    {
        m_dataType = VAR_DATE_TIME;
        m_data.timeData = v;
    }

    /// @brief Constructor
    CVariant(const void * value, size_t sz)
    {
        m_dataType = VAR_NONE;
        setBuffer(value, sz);
    }

    /// @brief Constructor
    CVariant(const CBuffer& value)
    {
        m_dataType = VAR_NONE;
        setBuffer(value.data(), value.bytes());
    }

    /// @brief Constructor
    CVariant(const CVariant& value)
    {
        m_dataType = VAR_NONE;
        setData(value);
    }

    /// @brief Destructor
    virtual ~CVariant()
    {
        releaseBuffers();
    }

    /// @brief Assignment method
    virtual void setBool(bool value);

    /// @brief Assignment method
    virtual void setInteger(int32_t value);

    /// @brief Assignment method
    virtual void setInt64(int64_t value);

    /// @brief Assignment method
    virtual void setFloat(double value);

    /// @brief Assignment method
    virtual void setMoney(int64_t value, unsigned scale);

    /// @brief Assignment method
    virtual void setString(const char * value, size_t maxlen = 0);

    /// @brief Assignment method
    virtual void setString(const std::string& value)
    {
        setString(value.c_str(), value.length());
    }

    /// @brief Assignment method
    void setExternalString(const char * value, int length = -1);

    /// @brief Assignment method
    void setExternalString(const std::string& value)
    {
        setExternalString(value.c_str(), (int) value.length());
    }

    /// @brief Assignment method
    virtual void setText(const char * value);

    /// @brief Assignment method
    virtual void setText(const std::string& str);

    /// @brief Assignment method
    virtual void setExternalText(const char * value);

    /// @brief Assignment method
    virtual void setBuffer(const void * value, size_t sz);

    /// @brief Assignment method
    virtual void setBuffer(const CBuffer& value)
    {
        setBuffer(value.data(), value.bytes());
    }

    /// @brief Assignment method
    virtual void setExternalBuffer(const void * value, size_t sz);

    /// @brief Assignment method
    virtual void setBuffer(const std::string& str);

    /// @brief Assignment method
    virtual void setDate(CDateTime value);

    /// @brief Assignment method
    virtual void setDateTime(CDateTime value);

    /// @brief Assignment method
    virtual void setImagePtr(const void *value);

    /// @brief Assignment method
    virtual void setImageNdx(uint32_t value);

    /// @brief Assignment method
    virtual void setMoney(const CMoneyData& value);

    /// @brief Assignment operator
    virtual CVariant& operator =(const CVariant &C)
    {
        if (this == &C)
            return *this;
        setData(C);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(int32_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(int64_t value)
    {
        setInt64(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(uint32_t value)
    {
        setInteger((int32_t) value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(uint64_t value)
    {
        setInt64((int64_t)value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(int16_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(uint16_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(float value)
    {
        setFloat(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(double value)
    {
        setFloat(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(const CMoneyData& value)
    {
        setMoney(value.quantity, value.scale);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(const char * value)
    {
        setString(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(const std::string& value)
    {
        setString(value.c_str(), value.length());
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(CDateTime value)
    {
        setDateTime(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(const void *value)
    {
        setImagePtr(value);
        return *this;
    }

    /// @brief Assignment operator
    virtual CVariant& operator =(const CBuffer& value)
    {
        setBuffer(value.data(), value.bytes());
        return *this;
    }

    /// @brief Directly reads the internal data
    virtual bool getBool() const
    {
        return m_data.boolData;
    }

    /// @brief Directly reads the internal data
    virtual const int32_t& getInteger() const
    {
        return m_data.intData;
    }

    /// @brief Directly reads the internal data
    virtual const int64_t& getInt64() const
    {
        return m_data.int64Data;
    }

    /// @brief Directly reads the internal data
    virtual const double& getFloat() const
    {
        return m_data.floatData;
    }

    /// @brief Directly reads the internal data
    virtual const CMoneyData& getMoney() const
    {
        return m_data.moneyData;
    }

    /// @brief Directly reads the internal data
    virtual const char* getString() const
    {
        return m_data.buffer.data;
    }

    /// @brief Directly reads the internal data
    virtual const char* getBuffer() const
    {
        return m_data.buffer.data;
    }

    /// @brief Directly reads the internal data
    virtual const char* getText() const
    {
        return m_data.buffer.data;
    }

    /// @brief Directly reads the internal data
    virtual CDateTime getDateTime() const
    {
        return m_data.floatData;
    }

    /// @brief Directly reads the internal data
    virtual CDateTime getDate() const
    {
        return (int) m_data.floatData;
    }

    /// @brief Directly reads the internal data
    virtual void* getImagePtr() const
    {
        return m_data.imagePtr;
    }

    /// @brief Directly reads the internal data
    virtual uint32_t getImageNdx() const
    {
        return (uint32_t) m_data.imageNdx;
    }

    /// @brief Returns the data type
    CVariantType dataType() const
    {
        return (CVariantType) (m_dataType & VAR_TYPES);
    }

    /// @brief Returns the data size
    size_t dataSize() const
    {
        return m_dataSize;
    }

    /// @brief Returns the allocated buffer size
    size_t bufferSize() const
    {
        return m_data.buffer.size;
    }

    /// @brief Returns the internal buffer
    void * dataBuffer() const
    {
        return (void *) (variantData *) &m_data;
    }

    /// @brief Conversion operator
    operator bool() const THROWS_EXCEPTIONS
    {
        return asBool();
    }

    /// @brief Conversion operator
    operator int32_t() const THROWS_EXCEPTIONS
    {
        return asInteger();
    }

    /// @brief Conversion operator
    operator uint32_t() const THROWS_EXCEPTIONS
    {
        return (uint32_t) asInteger();
    }

    /// @brief Conversion operator
    operator int64_t() const THROWS_EXCEPTIONS
    {
        return asInt64();
    }

    /// @brief Conversion operator
    operator uint64_t() const THROWS_EXCEPTIONS
    {
        return (uint64_t) asInt64();
    }

    /// @brief Conversion operator
    operator float() const THROWS_EXCEPTIONS
    {
        return (float) asFloat();
    }

    /// @brief Conversion operator
    operator double() const THROWS_EXCEPTIONS
    {
        return asFloat();
    }

    /// @brief Conversion operator
    operator std::string() const THROWS_EXCEPTIONS
    {
        return asString();
    }

    /// @brief Conversion operator
    operator CDateTime() const THROWS_EXCEPTIONS
    {
        return asDateTime();
    }

    /// @brief Conversion method
    ///
    /// Converts variant value to double.
    int32_t asInteger() const THROWS_EXCEPTIONS;

    /// @brief Conversion method
    ///
    /// Converts variant value to double.
    int64_t asInt64() const THROWS_EXCEPTIONS;

    /// @brief Conversion to bool
    ///
    /// Converts variant string value with first char one of 'Y','y','T','t' to true,
    /// and one of 'N','n','F','f' to false.
    /// For the integer and float values, the value <=0 is false, and > 0 is true.
    bool asBool() const THROWS_EXCEPTIONS;

    /// @brief Conversion to double
    ///
    /// Converts variant value to double.
    double asFloat() const THROWS_EXCEPTIONS;

    /// @brief Conversion to string
    ///
    /// Converts variant value to string.
    virtual std::string asString() const THROWS_EXCEPTIONS;

    /// @brief Conversion method
    ///
    /// Converts variant value to CDateTime. The time part of CDdatetime is empty.
    CDateTime asDate() const THROWS_EXCEPTIONS;

    /// @brief Conversion method
    ///
    /// Converts variant value to CDateTime.
    CDateTime asDateTime() const THROWS_EXCEPTIONS;

    /// @brief Conversion method
    ///
    /// Simply returns the internal data pointer for string/text/blob types.
    /// For incompatible types throws an exception.
    void *asImagePtr() const THROWS_EXCEPTIONS;

    /// @brief Sets the NULL state
    ///
    /// Useful for the database operations.
    /// Releases the memory allocated for string/text/blob types.
    /// Retains the data type. Sets the data to zero(s).
    /// @param vtype CVariantType, optional variant type to enforce
    virtual void setNull(CVariantType vtype=VAR_NONE);

    /// @brief Null flag
    ///
    /// Returns true if the NULL state is set
    bool isNull() const
    {
        return (m_dataType & VAR_NULL) != 0;
    }

    /// @brief Returns a name for a particular variant type
    /// @param type CVariantType, a variant type
    static std::string typeName(CVariantType type);

    /// @brief Returns a type for a particular variant type name
    /// @param name const char*, a variant type name
    static CVariantType nameType(const char* name);

    /// @brief Loads the data from XML node
    /// @param node const CXmlNode&, XML node to load data from
    void load(const CXmlNode& node);

    /// @brief Loads the data from XML node
    /// @param node const CXmlNode*, XML node to load data from
    void load(const CXmlNode* node)
    {
        load(*node);
    }

    /// @brief Saves the data into XML node
    /// @param node CXmlNode&, XML node to save data into
    void save(CXmlNode& node) const;

    /// @brief Saves the data into XML node
    /// @param node CXmlNode*, XML node to save data into
    void save(CXmlNode* node) const
    {
        save(*node);
    }
};
/// @}
}
#endif
