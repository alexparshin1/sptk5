/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/VariantStorageClient.h>
#include <sptk5/string_ext.h>

#include <algorithm>
#include <string>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

class Strings;

/**
 * String with ID
 * Extended version of std::string that supports an integer string ID
 */
class SP_EXPORT String
    : public std::string
    , public VariantStorageClient
{
public:
    /**
     * Default constructor
     */
    String()
        : m_id(0)
    {
    }

    /**
     * Copy constructor
     * @param str                Other object
     */
    String(const String& other) = default;

    /**
     * Move constructor
     * @param other                Other object
     */
    String(String&& other) noexcept = default;

    /**
     * Constructor
     * @param str                Source string
     * @param id                 Optional string id
     */
    String(const std::string& str, int64_t id = 0)
        : std::string(str)
        , m_id(id)
    {
    }

    /**
     * Constructor
     * @param str                Source string
     * @param len                Optional string id
     */
    String(const char* str)
        : std::string(str)
    {
    }

    /**
     * Constructor
     * @param str                Source string
     * @param len                String length
     * @param id                 String id
     */
    String(const char* str, size_t len, int64_t id = 0)
        : std::string(str, len)
        , m_id(id)
    {
    }

    /**
     * Constructor
     * @param len                String length
     * @param ch                Fill character
     * @param id                Optional string id
     */
    String(size_t len, char ch, int64_t id = 0)
        : std::string(len, ch)
        , m_id(id)
    {
    }

    /**
     * Destructor
     */
    ~String() noexcept override = default;

    /**
     * Assignment operator
     * @param si                 Source string
     */
    String& operator=(const std::string& si)
    {
        assign(si);
        m_id = 0;
        return *this;
    }

    /**
     * Copy assignment operator
     * @param other             Source string
     */
    String& operator=(const String& other) = default;

    /**
     * Move assignment operator
     * @param other             Source string
     */
    String& operator=(String&& other) noexcept = default;

    /**
     * Assignment operator
     * @param str                Source string
     */
    String& operator=(const char* str)
    {
        assign(str);
        m_id = 0;
        return *this;
    }

    /**
     * Returns string ID
     */
    int64_t ident() const
    {
        return m_id;
    }

    /**
     * Sets string ID
     */
    void ident(int64_t id)
    {
        m_id = id;
    }

    /**
     * Check if string is in the list
     * @param list              List of values
     * @return true if string is in the list
     */
    bool in(std::initializer_list<String> list) const;

    /**
     * Checks if string is matching with regular expression pattern
     * @param pattern           Regular expression pattern
     * @param options           Regular expression options (@see class CRegExp)
     */
    bool matches(const String& pattern, const String& options = String()) const;

    /**
     * Returns strings produced from current string by splitting it using regular expression pattern
     * @param pattern           Regular expression pattern
     */
    Strings split(const String& pattern) const;

    /**
     * Returns string with regular expression pattern replaced to replacement string
     *
     * Replacement string may optionally use references to pattern's group
     * @return Processed string
     * @param pattern           Regular expression pattern
     * @param replacement       Replacement string
     */
    String replace(const String& pattern, const String& replacement) const;

    /**
     * Returns upper case version of the string
     */
    String toUpperCase() const;

    /**
     * Returns upper case version of the string
     */
    String toLowerCase() const;

    /**
     * Converts string to integer
     */
    int toInt() const;

    /**
     * Returns true if the string starts from subject
     * @param subject           Subject to look for
     */
    bool startsWith(const String& subject) const;

    /**
     * Returns true if the string contains subject
     * @param subject           Subject to look for
     */
    bool contains(const String& subject) const;

    /**
     * Returns true if the string ends with subject
     * @param subject           Subject to look for
     */
    bool endsWith(const String& subject) const;

    /**
     * Returns trimmed string
     */
    String trim() const;

    static VariantDataType variantDataType()
    {
        return VariantDataType::VAR_STRING;
    }

    size_t dataSize() const override
    {
        return size();
    }

private:
    /**
     * String ID
     */
    int64_t m_id {0};
};

} // namespace sptk
