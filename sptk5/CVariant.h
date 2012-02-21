/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CVariant.h  -  description
                             -------------------
    begin                : Tue Dec 14 1999
    copyright            : (C) 1999-2008 by Alexey Parshin. All rights reserved.
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
    VAR_MONEY     = 4,    ///< Floating-point (double) money
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
struct CVariantDataBuffer {
    char *       data;         ///< String or buffer pointer
    uint32_t     size;         ///< Allocated buffer size
};

/// @brief Universal data storage.
///
/// It's not too compact or too fast.
/// It just allows to pass the data of different structure through the same
/// parameter.
class SP_EXPORT CVariant {
protected:
    /// Internal variant data storage
    union variantData {
        bool      boolData;        ///< Boolean data
        int32_t   intData;         ///< Integer data
        int64_t   int64Data;       ///< 64 bit integer data
        double    floatData;       ///< Floating point data
        double    timeData;        ///< CDateTime data
        CVariantDataBuffer buffer; ///< A buffer for data with the variable length like strings, or just generic buffers
        void *    imagePtr;        ///< Image pointer
        int32_t   imageNdx;        ///< Image index in object-specific table of image pointers
    } m_data;                      ///< Data storage union
    uint32_t     m_dataSize;       ///< Data size
    uint16_t     m_dataType;       ///< Data type

    /// Copies data from another CVariant
    void setData(const CVariant& C);

    /// Releases allocated buffer (if any)
    void releaseBuffers();

protected:
    /// @brief Sets the data size
    ///
    /// If data size is 0, the NULL flag is set. Otherwise, the NULL flag is set to false.
    /// @param ds uint32_t, data size (in bytes).
    void dataSize(uint32_t ds) {
        m_dataSize = ds;
        if (m_dataSize == 0)
           m_dataType |= VAR_NULL;
        else
           m_dataType &= VAR_TYPES | VAR_EXTERNAL_BUFFER;
    }

    /// Sets the data type
    void dataType(uint32_t dt) {
        m_dataType = dt;
    }

public:

    /// Constructor
    CVariant() {
        m_dataType = VAR_NONE|VAR_NULL;
        m_data.intData = 0;
    }

    /// Constructor
    CVariant(int32_t value) {
        m_dataType = VAR_INT;
        m_data.intData = value;
    }

    /// Constructor
    CVariant(uint32_t value) {
        m_dataType = VAR_INT;
        m_data.intData = value;
    }

    /// Constructor
    CVariant(int64_t value) {
        m_dataType = VAR_INT64;
        m_data.int64Data = value;
    }

    /// Constructor
    CVariant(uint64_t value) {
        m_dataType = VAR_INT64;
        m_data.int64Data = value;
    }

    /// Constructor
    CVariant(float value) {
        m_dataType = VAR_FLOAT;
        m_data.floatData = value;
    }

    /// Constructor
    CVariant(double value) {
        m_dataType = VAR_FLOAT;
        m_data.floatData = value;
    }

    /// Constructor
    CVariant(const char * value)   {
        m_dataType = VAR_NONE;
        setString(value);
    }

    /// Constructor
    CVariant(const std::string& v) {
        m_dataType = VAR_NONE;
        setString(v.c_str(),(uint32_t)v.length());
    }

    /// Constructor
    CVariant(CDateTime v) {
        m_dataType = VAR_DATE_TIME;
        m_data.timeData = v;
    }

    /// Constructor
    CVariant(const void * value,uint32_t sz){
        m_dataType = VAR_NONE;
        setBuffer(value,sz);
    }

    /// Constructor
    CVariant(const CBuffer& value) {
        m_dataType = VAR_NONE;
        setBuffer(value.data(),value.bytes());
    }

    /// Constructor
    CVariant(const CVariant& value){
        m_dataType = VAR_NONE;
        setData(value);
    }

    /// Destructor
    virtual ~CVariant() {
        releaseBuffers();
    }

    /// Assignment method
    virtual void setBool(bool value);

    /// Assignment method
    virtual void setInteger(int32_t value);

    /// Assignment method
    virtual void setInt64(int64_t value);

    /// Assignment method
    virtual void setFloat(double value);

    /// Assignment method
    virtual void setMoney(double value);

    /// Assignment method
    virtual void setString(const char * value,uint32_t maxlen=0);

    /// Assignment method
    virtual void setString(const std::string& value) {
        setString(value.c_str(),(uint32_t)value.length());
    }

    /// Assignment method
    void setExternalString(const char * value,int length=-1); 
                
    /// Assignment method
    void setExternalString(const std::string& value) {
	setExternalString(value.c_str(),(uint32_t)value.length());
    } 
                
    /// Assignment method
    virtual void setText(const char * value);

    /// Assignment method
    virtual void setText(const std::string& str);

    /// Assignment method
    virtual void setExternalText(const char * value);

    /// Assignment method
    virtual void setBuffer(const void * value,uint32_t sz);

    /// Assignment method
    virtual void setBuffer(const CBuffer& value) {
        setBuffer(value.data(),value.bytes());
    }

    /// Assignment method
    virtual void setExternalBuffer(const void * value,uint32_t sz);

    /// Assignment method
    virtual void setBuffer(const std::string& str);

    /// Assignment method
    virtual void setDate(CDateTime value);

    /// Assignment method
    virtual void setDateTime(CDateTime value);

    /// Assignment method
    virtual void setImagePtr(const void *value);

    /// Assignment method
    virtual void setImageNdx(uint32_t value);

    /// Assignment operator
    virtual CVariant& operator = (const CVariant &C)    {
        if (this == &C)
            return *this;
        setData(C);
        return *this;
    };

    /// Assignment operator
    virtual CVariant& operator = (int32_t value)   {
        setInteger(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (int64_t value)   {
        setInt64(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (uint32_t value) {
        setInteger(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (uint64_t value) {
        setInt64(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (int16_t value) {
        setInteger(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (uint16_t value) {
        setInteger(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (float value) {
        setFloat(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (double value){
        setFloat(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (const char * value)   {
        setString(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (const std::string& value) {
        setString(value.c_str(),(uint32_t)value.length());
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (CDateTime value)      {
        setDateTime(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (const void *value)    {
        setImagePtr(value);
        return *this;
    }

    /// Assignment operator
    virtual CVariant& operator = (const CBuffer& value) {
        setBuffer(value.data(),value.bytes());
        return *this;
    }

    /// Directly reads the internal data
    virtual bool getBool() const {
        return m_data.boolData;
    }

    /// Directly reads the internal data
    virtual const int32_t& getInteger() const {
        return m_data.intData;
    }

    /// Directly reads the internal data
    virtual const int64_t& getInt64() const {
        return m_data.int64Data;
    }

    /// Directly reads the internal data
    virtual const double& getFloat() const {
        return m_data.floatData;
    }

    /// Directly reads the internal data
    virtual const char* getString() const {
        return m_data.buffer.data;
    }

    /// Directly reads the internal data
    virtual const char* getBuffer() const {
        return m_data.buffer.data;
    }

    /// Directly reads the internal data
    virtual const char* getText() const {
        return m_data.buffer.data;
    }

    /// Directly reads the internal data
    virtual CDateTime getDateTime() const {
        return m_data.floatData;
    }

    /// Directly reads the internal data
    virtual CDateTime getDate() const {
        return (int)m_data.floatData;
    }

    /// Directly reads the internal data
    virtual void* getImagePtr() const {
        return m_data.imagePtr;
    }

    /// Directly reads the internal data
    virtual uint32_t getImageNdx() const {
        return m_data.imageNdx;
    }
    
    /// Returns the data type
    CVariantType dataType() const     {
        return (CVariantType) (m_dataType & VAR_TYPES);
    }

    /// Returns the data size
    uint32_t dataSize() const         {
        return m_dataSize;
    }

    /// Returns the allocated buffer size
    uint32_t bufferSize() const       {
        return m_data.buffer.size;
    }

    /// Returns the internal buffer
    void * dataBuffer() const         {
        return (void *)(variantData *)&m_data;
    }

    /// Conversion operator
    operator bool () const throw(CException)  {
        return asBool();
    }

    /// Conversion operator
    operator int32_t () const throw(CException)   {
        return asInteger();
    }

    /// Conversion operator
    operator uint32_t () const throw(CException)       {
        return asInteger();
    }

    /// Conversion operator
    operator int64_t () const throw(CException)   {
        return asInt64();
    }

    /// Conversion operator
    operator uint64_t () const throw(CException)   {
        return asInt64();
    }

    /// Conversion operator
    operator float () const throw(CException) {
        return (float)asFloat();
    }

    /// Conversion operator
    operator double () const throw(CException){
        return asFloat();
    }

    /// Conversion operator
    operator std::string () const throw(CException)    {
        return asString();
    }

    /// Conversion operator
    operator CDateTime () const throw(CException)      {
        return asDateTime();
    }

    /// @brief Conversion method
    ///
    /// Converts variant value to double.
    int32_t   asInteger() const throw(CException);

    /// @brief Conversion method
    ///
    /// Converts variant value to double.
    int64_t   asInt64() const throw(CException);

    /// @brief Conversion to bool
    ///
    /// Converts variant string value with first char one of 'Y','y','T','t' to true,
    /// and one of 'N','n','F','f' to false.
    /// For the integer and float values, the value <=0 is false, and > 0 is true.
    bool      asBool() const throw(CException);

    /// @brief Conversion to double
    ///
    /// Converts variant value to double.
    double    asFloat() const throw(CException);

    /// @brief Conversion to string
    ///
    /// Converts variant value to string.
    virtual std::string asString() const throw(CException);

    /// @brief Conversion method
    ///
    /// Converts variant value to CDateTime. The time part of CDdatetime is empty.
    CDateTime asDate() const throw(CException);

    /// @brief Conversion method
    ///
    /// Converts variant value to CDateTime.
    CDateTime asDateTime() const throw(CException);

    /// @brief Conversion method
    ///
    /// Simply returns the internal data pointer for string/text/blob types.
    /// For incompatible types throws an exception.
    void *asImagePtr() const throw(CException);

    /// @brief Sets the NULL state
    ///
    /// Useful for the database operations.
    /// Releases the memory allocated for string/text/blob types.
    /// Retains the data type. Sets the data to zero(s).
    void setNull();

    /// @brief Null flag
    ///
    /// Returns true if the NULL state is set
    bool isNull() const {
        return (m_dataType & VAR_NULL) != 0;
    }

    /// @brief Returns a name for a particular variant type
    ///
    /// @param type CVariantType, a variant type
    static std::string typeName(CVariantType type);

    /// @brief Returns a type for a particular variant type name
    ///
    /// @param name const char*, a variant type name
    static CVariantType nameType(const char* name);

    /// @brief Loads the data from XML node
    ///
    /// @param node const CXmlNode&, XML node to load data from
    void load(const CXmlNode& node);

    /// @brief Loads the data from XML node
    ///
    /// @param node const CXmlNode*, XML node to load data from
    void load(const CXmlNode* node) {
        load(*node);
    }

    /// @brief Saves the data into XML node
    ///
    /// @param node CXmlNode&, XML node to save data into
    void save(CXmlNode& node) const;

    /// @brief Saves the data into XML node
    ///
    /// @param node CXmlNode*, XML node to save data into
    void save(CXmlNode* node) const {
        save(*node);
    }
};
/// @}
}
#endif
