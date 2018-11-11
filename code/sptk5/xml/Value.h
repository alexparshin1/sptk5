/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Value.h - description                                  ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_XML_VALUE_H__
#define __SPTK_XML_VALUE_H__

#include <sptk5/cxml>
#include <sptk5/DateTime.h>

namespace sptk {
namespace xml {

/**
 * @addtogroup XML
 * @{
 */

class Node;

class Document;

/**
 * @brief XML value
 *
 * A string that has converters to and from most popular types
 */
class SP_EXPORT Value
{
    /**
     * XML value
     */
    String m_value;

public:
    /**
     * @brief Default constructor
     *
     * Creates an empty XML attribute
     */
    Value()
    {}

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from character string
     * @param v                 Value
     */
    Value(const char* v)
    : m_value(v)
    {}

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from character string
     * @param v                 Value
     * @param sz                Value length
     */
    Value(const char* v, size_t sz)
    : m_value(v, sz)
    {
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from string
     * @param v                 value
     */
    Value(const String& v)
    : m_value(v)
    {
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from integer
     * @param v                 Value
     */
    Value(int32_t v)
    {
        operator=(v);
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from integer
     * @param v                 Value
     */
    Value(uint32_t v)
    {
        operator=(v);
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from int64_t
     * @param v                 Value
     */
    Value(int64_t v)
    {
        operator=(v);
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from uint64_t
     * @param v                 Value
     */
    Value(uint64_t v)
    {
        operator=(v);
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from double
     * @param v                 Value
     */
    Value(double v)
    {
        operator=(v);
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from bool
     * @param v                 Value
     */
    Value(bool v)
    {
        operator=(v);
    }

    /**
     * @brief Constructor
     *
     * Creates an XML attribute from DateTime
     * @param v                 Value
     */
    Value(DateTime v)
            :
            m_value(v)
    {
    }

    /**
     * @brief Assignment of the value
     *
     * @param v                 New value
     */
    Value& operator=(bool v);

    /**
     * @brief Assignment of the value
     *
     * @param v                 New value
     */
    Value& operator=(int32_t v);

    /**
     * @brief Assignment of the value
     *
     * @param v                 New value
     */
    Value& operator=(uint32_t v);

    /**
     * @brief Assignment of the value
     *
     * @param v                 New value
     */
    Value& operator=(int64_t v);

    /**
     * @brief Assignment of the value
     *
     * @param v                 New value
     */
    Value& operator=(uint64_t v);

    /**
     * @brief Assignment of the value
     *
     * @param v                 New value
     */
    Value& operator=(double v);

    /**
     * @brief Assignment of the value
     *
     * @param s                 New value
     */
    Value& operator=(const char* s)
    {
        m_value = s;
        return *this;
    }

    /**
     * @brief Assignment of the value
     *
     * @param s                 New value
     */
    Value& operator=(const String& s)
    {
        m_value = s;
        return *this;
    }

    /**
     * @brief Assignment of the value
     *
     * @param s                 New value
     */
    Value& operator=(DateTime s)
    {
        m_value = s.dateString() + " " + s.timeString(true);
        return *this;
    }

    /**
     * @brief Returns const reference to string value
     */
    const String& str() const
    {
        return m_value;
    }

    /**
     * @brief Returns const pointer to string data
     */
    const char* c_str() const
    {
        return m_value.c_str();
    }

    /**
     * @brief Returns the value with the conversion
     */
    operator String() const
    {
        return m_value;
    }

    /**
     * @brief Returns the value with the conversion
     */
    operator std::string() const
    {
        return m_value;
    }

    /**
     * @brief Returns the value with the conversion
     */
    operator uint32_t() const
    {
        return (uint32_t) atol(m_value.c_str());
    }

    /**
     * @brief Returns the value with the conversion
     */
    operator int32_t() const
    {
        return (int32_t) atol(m_value.c_str());
    }

    /**
     * @brief Returns the value with the conversion
     */
    operator uint64_t() const
    {
#ifdef __UNIX_COMPILER__
        return (uint64_t) atoll(m_value.c_str());
#else
        return (uint64_t) _atoi64(m_value.c_str());
#endif
    }

    /**
     * @brief Returns the value with the conversion
     */
    operator int64_t() const
    {
#ifdef __UNIX_COMPILER__
        return (int64_t) atoll(m_value.c_str());
#else
        return (int64_t) _atoi64(m_value.c_str());
#endif
    }

    /**
     * @brief Returns the value with the conversion
     */
    operator bool() const;

    /**
     * @brief Returns the value with the conversion
     */
    operator double() const
    {
        return atof(m_value.c_str());
    }

    /**
     * @brief Returns the value length
     */
    size_t size() const
    {
        return m_value.size();
    }

    /**
     * @brief Returns true if the value is empty string
     */
    bool empty() const
    {
        return m_value.empty();
    }
};

/**
 * @}
 */
}
}
#endif
