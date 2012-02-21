/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlValue.h  -  description
                             -------------------
    begin                : Wed June 21 2006
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CXMLVALUE_H__
#define __CXMLVALUE_H__

#include <sptk5/cxml>
#include <sptk5/CDateTime.h>

namespace sptk {

/// @addtogroup XML
/// @{

class CXmlNode;
class CXmlDoc;

/// @brief XML value
///
/// A string that has converters to and from most popular types
class SP_EXPORT CXmlValue : public std::string {

public:
    /// @brief Default constructor
    ///
    /// Creates an empty XML attribute
    CXmlValue() {}

    /// @brief Constructor
    ///
    /// Creates an XML attribute from character string
    CXmlValue(const char *v) : std::string(v) {}

    /// @brief Constructor
    ///
    /// Creates an XML attribute from std::string
    CXmlValue(std::string v) : std::string(v) {}

    /// @brief Constructor
    ///
    /// Creates an XML attribute from integer
    CXmlValue(int32_t v) {
        operator = (v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from integer
    CXmlValue(uint32_t v) {
        operator = (v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from int32_t
    CXmlValue(int64_t v)        {
        operator = (v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from uint32_t
    CXmlValue(uint64_t v) {
        operator = (v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from double
    CXmlValue(double v) {
        operator = (v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from bool
    CXmlValue(bool v) {
        operator = (v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from CDateTime
    CXmlValue(CDateTime v) : std::string(v) {}

    /// @brief Assignment of the value
    ///
    /// @param v bool, a new value
    CXmlValue& operator = (bool v);

    /// @brief Assignment of the value
    ///
    /// @param v int32_t, a new value
    CXmlValue& operator = (int32_t v);

    /// @brief Assignment of the value
    ///
    /// @param v uint32_t, a new value
    CXmlValue& operator = (uint32_t v);

    /// @brief Assignment of the value
    ///
    /// @param v int64_t, a new value
    CXmlValue& operator = (int64_t v);

    /// @brief Assignment of the value
    ///
    /// @param v int64_t, a new value
    CXmlValue& operator = (uint64_t v);

    /// @brief Assignment of the value
    ///
    /// @param v double, a new value
    CXmlValue& operator = (double v);

    /// @brief Assignment of the value
    ///
    /// @param s const std::string, a new value
    CXmlValue& operator = (const std::string& s) {
        assign(s);
        return *this;
    }

    /// @brief Assignment of the value
    ///
    /// @param s const char *, a new value
    CXmlValue& operator = (const char *s)        {
        assign(s);
        return *this;
    }

    /// @brief Assignment of the value
    ///
    /// @param s CDateTime, a new value
    CXmlValue& operator = (CDateTime s)          {
        assign(s);
        return *this;
    }

    /// @brief Returns the value with the conversion
    operator uint32_t () const                  {
        return atol(c_str());
    }

    /// @brief Returns the value with the conversion
    operator int32_t () const                           {
        return atol(c_str());
    }

    /// @brief Returns the value with the conversion
    operator uint64_t () const                  {
#ifdef __UNIX_COMPILER__
        return atoll(c_str());
#else
        return _atoi64(c_str());
#endif
    }

    /// @brief Returns the value with the conversion
    operator int64_t () const                           {
#ifdef __UNIX_COMPILER__
        return atoll(c_str());
#else
        return _atoi64(c_str());
#endif
    }

    /// @brief Returns the value with the conversion
    operator bool () const;

    /// @brief Returns the value with the conversion
    operator double () const                         {
        return atof(c_str());
    }
};

/// @}
}
#endif
