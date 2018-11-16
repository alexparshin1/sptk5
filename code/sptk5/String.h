/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       String.h - description                                 ║
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

#ifndef __SPTK_STRING_H__
#define __SPTK_STRING_H__

#include "sptk.h"
#include "string_ext.h"
#include <string>
#include <algorithm>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

class Strings;

/**
 * @brief String with ID
 * Extended version of std::string that supports an integer string ID
 */
class SP_EXPORT String : public std::string
{
protected:
    /**
     * String ID
     */
    int32_t m_id {0};


public:
    /**
     * @brief Default constructor
     */
    String() noexcept
    {}

    /**
     * @brief Copy constructor
     * @param str				Source string
     */
    String(const String& str) noexcept
    : std::string(str), m_id(str.m_id)
    {
    }

    /**
     * @brief Move constructor
     * @param src				Other object
     */
    String(String&& src) noexcept
    : std::string(std::move(src)), m_id(src.m_id)
    {
        src.m_id = 0;
    }

    /**
     * @brief Move constructor
     * @param src				Other object
     */
    String(std::string&& src) noexcept
    : std::string(std::move(src))
    {}

    /**
     * @brief Constructor
     * @param str				Source string
     * @param id				Optional string id
     */
    String(const std::string& str, int32_t id = 0) noexcept
    : std::string(str), m_id(id)
    {
    }

    /**
     * @brief Constructor
     * @param str				Source string
     * @param id				Optional string id
     */
    String(const char* str, int32_t id = 0) noexcept
    : std::string(str), m_id(id)
    {
    }

    /**
     * Constructor
     * @param str				Source string
     * @param len				Optional string length
     * @param id				Optional string id
     */
    String(const char* str, size_t len, int32_t id = 0) noexcept
    : std::string(str, len), m_id(id)
    {
    }

    /**
     * Constructor
     * @param len				String length
     * @param ch				Fill character
     * @param id				Optional string id
     */
    String(size_t len, char ch, int32_t id = 0) noexcept
    : std::string(len, ch), m_id(id)
    {
    }

    /**
     * Assignment operator
     * @param si 				Source string
     */
    String& operator=(const std::string& si)
    {
        assign(si);
        m_id = 0;
        return *this;
    }

    /**
     * Assignment operator
     * @param si 				Source string
     */
    String& operator=(const String& si)
    {
        assign(si);
        m_id = si.m_id;
        return *this;
    }

    /**
     * Assignment operator
     * @param str				Source string
     */
    String& operator=(const char* str)
    {
        assign(str);
        m_id = 0;
        return *this;
    }

    /**
     * @brief Returns string ID
     */
    int32_t ident() const
    {
        return m_id;
    }

    /**
     * @brief Sets string ID
     */
    void ident(int32_t id)
    {
        m_id = id;
    }

    /**
     * @brief Checks if string is matching with regular expression pattern
     * @param pattern           Regular expression pattern
     * @param options           Regular expression options (@see class CRegExp)
     */
    bool matches(const String& pattern, const String& options = "") const;

    /**
     * @brief Returns strings produced from current string by splitting it using regular expression pattern
     * @param pattern           Regular expression pattern
     */
    Strings split(const String& pattern) const;

    /**
     * @brief Returns string with regular expression pattern replaced to replacement string
     *
     * Replacement string may optionally use references to pattern's group
     * @return Processed string
     * @param pattern           Regular expression pattern
     * @param replacement       Replacement string
     */
    String replace(const String& pattern, const String& replacement) const;

    /**
     * @brief Returns upper case version of the string
     */
    String toUpperCase() const;

    /**
     * @brief Returns upper case version of the string
     */
    String toLowerCase() const;

    /**
     * @brief Returns true if the string starts from subject
     * @param subject           Subject to look for
     */
    bool startsWith(const String& subject) const;

    /**
     * @brief Returns true if the string ends with subject
     * @param subject           Subject to look for
     */
    bool endsWith(const String& subject) const;

    /**
     * @brief Returns trimmed string
     */
    String trim() const;
};

} // namespace sptk

#endif
