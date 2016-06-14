/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       XMLValue.h - description                               ║
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

#ifndef __SPTK_XMLVALUE_H__
#define __SPTK_XMLVALUE_H__

#include <sptk5/cxml>
#include <sptk5/DateTime.h>

namespace sptk {

/// @addtogroup XML
/// @{

class XMLNode;
class XMLDocument;

/// @brief XML value
///
/// A string that has converters to and from most popular types
class SP_EXPORT XMLValue
{
    std::string     m_value;    ///< XML value
public:
    /// @brief Default constructor
    ///
    /// Creates an empty XML attribute
    XMLValue()
    {
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from character string
    /// @param v const char *, value
    XMLValue(const char *v) :
        m_value(v)
    {
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from character string
    /// @param v const char *, value
    /// @param sz size_t, value length
    XMLValue(const char *v, size_t sz) :
        m_value(v, sz)
    {
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from std::string
    /// @param v const std::string&, value
    XMLValue(const std::string& v) :
        m_value(v)
    {
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from integer
    /// @param v int32_t, value
    XMLValue(int32_t v)
    {
        operator =(v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from integer
    /// @param v uint32_t, value
    XMLValue(uint32_t v)
    {
        operator =(v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from int32_t
    /// @param v int64_t, value
    XMLValue(int64_t v)
    {
        operator =(v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from uint32_t
    /// @param v uint64_t, value
    XMLValue(uint64_t v)
    {
        operator =(v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from double
    /// @param v double, value
    XMLValue(double v)
    {
        operator =(v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from bool
    /// @param v bool, value
    XMLValue(bool v)
    {
        operator =(v);
    }

    /// @brief Constructor
    ///
    /// Creates an XML attribute from CDateTime
    /// @param v int32_t, value
    XMLValue(DateTime v) :
        m_value(v)
    {
    }

    /// @brief Assignment of the value
    ///
    /// @param v bool, a new value
    XMLValue& operator =(bool v);

    /// @brief Assignment of the value
    ///
    /// @param v int32_t, a new value
    XMLValue& operator =(int32_t v);

    /// @brief Assignment of the value
    ///
    /// @param v uint32_t, a new value
    XMLValue& operator =(uint32_t v);

    /// @brief Assignment of the value
    ///
    /// @param v int64_t, a new value
    XMLValue& operator =(int64_t v);

    /// @brief Assignment of the value
    ///
    /// @param v int64_t, a new value
    XMLValue& operator =(uint64_t v);

    /// @brief Assignment of the value
    ///
    /// @param v double, a new value
    XMLValue& operator =(double v);

    /// @brief Assignment of the value
    ///
    /// @param s const std::string, a new value
    XMLValue& operator =(const std::string& s)
    {
        m_value = s;
        return *this;
    }

    /// @brief Assignment of the value
    ///
    /// @param s const char *, a new value
    XMLValue& operator =(const char *s)
    {
        m_value = s;
        return *this;
    }

    /// @brief Assignment of the value
    ///
    /// @param s CDateTime, a new value
    XMLValue& operator =(DateTime s)
    {
        m_value = s.dateString() + " " + s.timeString(true);
        return *this;
    }

    /// @brief Returns const reference to string value
    const std::string& str() const
    {
        return m_value;
    }

    /// @brief Returns const pointer to string data
    const char* c_str() const
    {
        return m_value.c_str();
    }
    
    /// @brief Returns the value with the conversion
    operator std::string() const
    {
        return m_value;
    }

    /// @brief Returns the value with the conversion
    operator uint32_t() const
    {
        return (uint32_t) atol(m_value.c_str());
    }

    /// @brief Returns the value with the conversion
    operator int32_t() const
    {
        return (int32_t) atol(m_value.c_str());
    }

    /// @brief Returns the value with the conversion
    operator uint64_t() const
    {
#ifdef __UNIX_COMPILER__
        return (uint64_t) atoll(m_value.c_str());
#else
        return (uint64_t) _atoi64(m_value.c_str());
#endif
    }

    /// @brief Returns the value with the conversion
    operator int64_t() const
    {
#ifdef __UNIX_COMPILER__
        return (int64_t) atoll(m_value.c_str());
#else
        return (int64_t) _atoi64(m_value.c_str());
#endif
    }

    /// @brief Returns the value with the conversion
    operator bool() const;

    /// @brief Returns the value with the conversion
    operator double() const
    {
        return atof(m_value.c_str());
    }

    /// @brief Returns the value length
    size_t size() const
    {
        return m_value.size();
    }

    /// @brief Returns true if the value is empty string
    bool empty() const
    {
        return m_value.empty();
    }
};

/// @}
}
#endif

