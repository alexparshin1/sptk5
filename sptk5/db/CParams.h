/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CParams.h  -  description
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

#ifndef __CPARAMS_H__
#define __CPARAMS_H__

#include <sptk5/sptk.h>
#include <sptk5/istring.h>
#include <sptk5/CStrings.h>
#include <sptk5/CVariant.h>
#include <sptk5/CBuffer.h>
#include <sptk5/CIntList.h>

#include <vector>
#include <map>

namespace sptk {

/// @addtogroup Database Database Support
/// @{

/// @brief Parameter Binding descriptor
///
/// Stores the last information on parameter binding
class SP_EXPORT CParamBinding
{
    void*           m_stmt;    ///< Statement handle or id
    CVariantType    m_type;    ///< Data type
    void*           m_buffer;  ///< Buffer
    uint32_t        m_size;    ///< Buffer size
public:
    /// @brief Constructor
    CParamBinding()
    {
        reset();
    }

    /// @brief Resets the binding information
    void reset()
    {
        m_stmt = 0;
        m_type = VAR_NONE;
        m_size = 0;
        m_buffer = 0;
    }

    /// @brief Checks if the parameter binding is matching the cached
    ///
    /// Returns true, if the passed parameters are matching last binding parameters.
    /// Returns false and stores new parameters into last binding parameters otherwise.
    /// @param stmt void*, statement handle
    /// @param type CVariantType, data type
    /// @param size uint32_t, binding buffer size
    /// @param buffer void*, binding buffer
    bool check(void* stmt, CVariantType type, uint32_t size, void* buffer)
    {
        bool changed = true;
        if (m_stmt != stmt) {
            m_stmt = stmt;
            changed = false;
        }
        if (m_type != type) {
            m_type = type;
            changed = false;
        }
        if (m_size != size) {
            m_size = size;
            changed = false;
        }
        if (m_buffer != buffer) {
            m_buffer = buffer;
            changed = false;
        }
        return changed;
    }
};

/// @brief SQL query parameter.
///
/// Simplifies the ODBC parameter binding.
/// Automatically handles most of the data conversions on assignments.
class SP_EXPORT CParam: public CVariant
{
    friend class CQuery;
    friend class CParamList;

protected:
    std::string     m_name;              ///< Parameter name
    CIntList        m_bindParamIndexes;  ///< The list of SQL query parameter numbers with this name
    char*           m_timeData;          ///< Special memory allocated for time structures
    int32_t         m_callbackLength;    ///< An integer reserved to callback parameter data length

public:
    CParamBinding   m_binding;     ///< The last successfull binding information

    /// Clears internal parameter binding index
    void bindClear()
    {
        m_bindParamIndexes.clear();
    }

    /// Adds internal parameter binding index
    void bindAdd(uint32_t bindIndex);

    /// Returns internal parameter binding count
    uint32_t bindCount()
    {
        return (uint32_t) m_bindParamIndexes.size();
    }

    /// Returns the parameter bing position by index in the binding list
    uint32_t bindIndex(uint32_t ind)
    {
        return m_bindParamIndexes[ind];
    }

    /// Returns the internal small conversion buffer used to convert the
    /// date structure to SPTK. Please, don't use it.
    char *conversionBuffer()
    {
        return m_timeData;
    }

    /// An integer reserved to callback parameter data length
    int32_t& callbackLength()
    {
        return m_callbackLength;
    }

public:

    /// Constructor
    /// @param name char *, parameter name
    CParam(char *name);

    /// Destructor
    ~CParam()
    {
        delete[] m_timeData;
    }

    /// Returns parameter name
    std::string name() const
    {
        return m_name;
    }

    /// Assign operator
    CParam& operator =(const CParam& param);

    /// Assign operator
    CParam& operator =(const CVariant& param)
    {
        if (this == &param)
            return *this;
        setData(param);
        return *this;
    }
    ;

    /// Assign operator
    CParam& operator =(int16_t v)
    {
        setInteger(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(uint16_t v)
    {
        setInteger(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(int32_t v)
    {
        setInteger(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(uint32_t v)
    {
        setInteger(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(int64_t v)
    {
        setInt64(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(uint64_t v)
    {
        setInt64(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(float v)
    {
        setFloat(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(double v)
    {
        setFloat(v);
        return *this;
    }

    /// Assign operator
    CParam& operator =(const char *s)
    {
        setString(s);
        return *this;
    }

    /// Assign operator
    CParam& operator =(const std::string& s)
    {
        setString(s.c_str());
        return *this;
    }

    /// Assign operator
    CParam& operator =(CDateTime dt)
    {
        setDateTime(dt);
        return *this;
    }

    /// Assignment operator
    virtual CParam& operator =(const void *value)
    {
        setImagePtr(value);
        return *this;
    }

    /// Assign operator
    CParam& operator =(const CBuffer& buffer)
    {
        setBuffer(buffer);
        return *this;
    }

    /// Returns parameter info as XML
    std::string asXML() const;

    /// @brief String assignment method
    ///
    /// In contrast to CVariant::setString() method, this method
    /// tries not to decrease the allocated buffer.
    /// @param value const char*, string to assign
    /// @param maxlen uint32_t, maximum length of the assigned string
    virtual void setString(const char * value, uint32_t maxlen = 0);

    /// @brief String assignment method
    ///
    /// In contrast to CVariant::setString() method, this method
    /// tries not to decrease the allocated buffer.
    /// @param value const string&, string to assign
    virtual void setString(const std::string& value)
    {
        setString(value.c_str(), (uint32_t) value.length());
    }
};

/// @brief A vector of CParam*
///
/// Doesn't mantain CParam memory.
/// Used to return a list of pointers on existing parameters.
typedef std::vector<CParam *> CParamVector;

/// @brief Query parameters list.
///
/// Has internal index to speed up the parameter search by name.
/// @see CQuery
/// @see CParam
class SP_EXPORT CParamList
{
    friend class CQuery;

    CParamVector                m_items;             ///< The list of parameters
    CParamVector::iterator      m_paramStreamItor;   ///< The list of parameters iterator
    std::map<istring, CParam*>  m_index;             ///< The parameters index
protected:
    /// @brief Adds a parameter to the list
    void add(CParam *item);

public:

    /// @brief Default constructor
    CParamList()
    {
        m_paramStreamItor = m_items.begin();
    }
    ;

    /// @brief Destructor
    ~CParamList();

    /// @brief Removes all the parameters from the list
    ///
    /// Releases any allocated resources
    void clear();

    /// @brief Returns parameter by name
    ///
    /// If the parameter isn't found, returns 0
    /// @param paramName const char *, parameter name
    /// @returns parameter pointer, or 0 if not found
    CParam* find(const char *paramName);

    /// @brief Removes a parameter from the list and from the memory.
    /// @param ndx uint32_t, parameter index in the list
    void remove(uint32_t ndx);

    /// @brief Parameter access by index
    /// @param index uint32_t, parameter index
    CParam& operator[](uint32_t index) const
    {
        return *m_items[index];
    }

    /// @brief Parameter access by name
    /// @param paramName const char *, parameter name
    CParam& operator[](const char *paramName) const;

    /// @brief Parameter access by name
    /// @param paramName const std::string&, parameter name
    CParam& operator[](const std::string& paramName) const
    {
        return operator[](paramName.c_str());
    }

    /// @brief Returns parameter count
    uint32_t size() const
    {
        return (uint32_t) m_items.size();
    }

    /// Sets the parameter stream iterator to the first parameter
    void rewind()
    {
        m_paramStreamItor = m_items.begin();
    }

    /// @brief Returns the parameter pointers
    ///
    /// A parameter is included for every parameter position in the query.
    /// @param params CParamVector&, parameters vector
    void enumerate(CParamVector& params);

    /// Sets the parameter stream iterator to the next parameter.
    /// After the last parameter is reached, the iterator is switch to the first parameter
    /// @returns current parameter
    CParam& next();
};

/// Streamed parameter assign. The data is assigned to the current parameter,
/// and then next parameter becomes current. The rewind() method is called
/// automatically to reset the parameter iterator to the first parameter upon
/// query exec() or open() method calls.
/// @param paramList CParamList&, a list of parameters to assign
/// @param data const std::string, a data to assign to current parameter
SP_EXPORT CParamList& operator <<(sptk::CParamList& paramList, const std::string& data);

/// Streamed parameter assign. The data is assigned to the current parameter,
/// and then next parameter becomes current. The rewind() method is called
/// automatically to reset the parameter iterator to the first parameter upon
/// query exec() or open() method calls.
/// @param paramList CParamList&, a list of parameters to assign
/// @param data const char *, a data to assign to current parameter
SP_EXPORT CParamList& operator <<(sptk::CParamList& paramList, const char *data);

/// Streamed parameter assign. The data is assigned to the current parameter,
/// and then next parameter becomes current. The rewind() method is called
/// automatically to reset the parameter iterator to the first parameter upon
/// query exec() or open() method calls.
/// @param paramList CParamList&, a list of parameters to assign
/// @param data int, a data to assign to current parameter
SP_EXPORT CParamList& operator <<(sptk::CParamList& paramList, int data);

/// Streamed parameter assign. The data is assigned to the current parameter,
/// and then next parameter becomes current. The rewind() method is called
/// automatically to reset the parameter iterator to the first parameter upon
/// query exec() or open() method calls.
/// @param paramList CParamList&, a list of parameters to assign
/// @param data double, a data to assign to current parameter
SP_EXPORT CParamList& operator <<(sptk::CParamList& paramList, double data);

/// Streamed parameter assign. The data is assigned to the current parameter,
/// and then next parameter becomes current. The rewind() method is called
/// automatically to reset the parameter iterator to the first parameter upon
/// query exec() or open() method calls.
/// @param paramList CParamList&, a list of parameters to assign
/// @param data CDateTime, a data to assign to current parameter
SP_EXPORT CParamList& operator <<(sptk::CParamList& paramList, sptk::CDateTime data);

/// Streamed parameter assign. The data is assigned to the current parameter,
/// and then next parameter becomes current. The rewind() method is called
/// automatically to reset the parameter iterator to the first parameter upon
/// query exec() or open() method calls.
/// @param paramList CParamList&, a list of parameters to assign
/// @param data const CBuffer&, a data to assign to current parameter
SP_EXPORT CParamList& operator <<(sptk::CParamList& paramList, const sptk::CBuffer& data);

/// @}
}

#endif
