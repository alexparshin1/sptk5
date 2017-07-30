/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Strings.h - description                                ║
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

#ifndef __SPTK_STRINGS_H__
#define __SPTK_STRINGS_H__

#include <sptk5/sptk.h>
#include <sptk5/string_ext.h>
#include <sptk5/Exception.h>
#include <string>
#include <vector>
#include <algorithm>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * A workaround for VC++ bug
 */
typedef std::string std_string;

class Strings;

/**
 * @brief String with ID
 * Extended version of std::string that supports an integer string ID
 */
class SP_EXPORT String : public std_string
{
protected:
    /**
     * String ID
     */
    int32_t m_id;


public:
    /**
     * @brief Default constructor
     */
    String(): m_id(0)
    {
    }

    /**
     * @brief Copy constructor
     * @param str const idstring&, source string
     */
    String(const String &str) : std_string(str), m_id(str.m_id)
    {
    }

    /**
     * @brief Move constructor
     * @param src const Strings&, other object
     */
    String(String&& src)
    : std_string(std::move(src)), m_id(src.m_id)
    {
    }

    /**
     * @brief Move constructor
     * @param src const Strings&, other object
     */
    String(std::string&& src)
    : std_string(std::move(src)), m_id(0)
    {
    }

    /**
     * @brief Constructor
     * @param str const string&, source string
     * @param id in, optional string id
     */
    String(const std::string &str, int32_t id = 0) : std_string(str), m_id(id)
    {
    }

    /**
     * Constructor
     * @param str const char *, source string
     * @param len int, optional string length
     * @param id int, optional string id
     */
    String(const char *str, int32_t len, int32_t id=0) : std_string(str, len), m_id(id)
    {
    }

    /**
     * Constructor
     * @param str const char *, source string
     */
    String(const char *str) : std_string(str), m_id(0)
    {
    }

    /**
     * Assignment operator
     */
    String &operator=(const std::string& si)
    {
        assign(si);
        m_id = 0;
        return *this;
    }

    /**
     * Assignment operator
     */
    String &operator=(const String& si)
    {
        assign(si);
        m_id = si.m_id;
        return *this;
    }

    /**
     * Assignment operator
     * @param str const char *, source string
     */
    String &operator=(const char *str)
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
     * @param pattern std::string, Regular expression pattern
     * @param options std::string, Regular expression options (@see class CRegExp)
     */
    bool matches(std::string pattern, std::string options="") const;

    /**
     * @brief Returns strings produced from current string by splitting it using regular expression pattern
     * @param pattern std::string, Regular expression pattern
     */
    Strings split(std::string pattern) const;

    /**
     * @brief Returns string with regular expression pattern replaced to replacement string
     *
     * Replacement string may optionally use references to pattern's group
     * @return Processed string
     * @param pattern std::string, Regular expression pattern
     * @param replacement std::string, Replacement string
     */
    std::string replace(std::string pattern, std::string replacement) const;

    /**
     * @brief Returns upper case version of the string
     */
    std::string toUpperCase() const;

    /**
     * @brief Returns upper case version of the string
     */
    std::string toLowerCase() const;

    /**
     * @brief Returns true if the string starts from subject
     * @param subject std::string, Subject to look for
     */
    bool startsWith(std::string subject) const;

    /**
     * @brief Returns true if the string ends with subject
     * @param subject std::string, Subject to look for
     */
    bool endsWith(std::string subject) const;

    /**
     * @brief Returns trimmed string
     */
    String trim() const;
};

/**
 * @brief List of strings with ids
 *
 * General string list. Based on vector<idstring>. Stores strings with (optional) integer Ids.
 * Includes several extra methods to construct it from string or load/save from/to file.
 */
class SP_EXPORT Strings
: public std::vector<String>
{
    /**
     * User-specified data
     */
    int32_t m_userData;

    /**
     * Ascending sort compare function, used in sort()
     * @param first const String&, First compared string
     * @param second const String&, Second compared string
     */
    static bool sortAscending(const String& first, const String& second);

    /**
     * Descending sort compare function, used in sort()
     * @param first const String&, First compared string
     * @param second const String&, Second compared string
     */
    static bool sortDescending(const String& first, const String& second);

    /**
     * @brief Splits source string on substrings using exact delimiter
     *
     * Consequent delimiters create empty strings.
     * @param src const std::string&, a source string
     * @param delimiter const char *, a delimiter string
     */
    void splitByDelimiter(const std::string &src, const char *delimiter);

    /**
     * @brief Splits source string on substrings using any char in delimiter
     *
     * Consequent delimiters are treated as a single one.
     * @param src const std::string&, a source string
     * @param delimiter const char *, a delimiter string
     */
    void splitByAnyChar(const std::string &src, const char *delimiter);

    /**
     * @brief Splits source string on substrings using regular expression
     *
     * Consequent delimiters are treated as a single one.
     * @param src const std::string&, a source string
     * @param pattern const char *, a regex pattern string
     */
    void splitByRegExp(const std::string &src, const char *pattern);

public:

    /**
     * @brief String split mode
     */
    enum SplitMode
    {
        /**
         * Split by the whole delimiter
         */
        SM_DELIMITER,

        /**
         * Split by any char in delimiter
         */
        SM_ANYCHAR,

        /**
         * Regular expression
         */
        SM_REGEXP

    };

    /**
     * @brief Default constructor
     */
    Strings() noexcept
    {
        m_userData = 0;
    }

    /**
     * @brief Copy constructor
     * @param src const Strings&, other object
     */
    Strings(const Strings &src) noexcept
    : std::vector<String>(src), m_userData(src.m_userData)
    {
    }

    /**
     * @brief Move constructor
     * @param src const Strings&, other object
     */
    Strings(Strings&& src) noexcept
    : std::vector<String>(std::move(src)), m_userData(src.m_userData)
    {
    }

    /**
     * @brief Constructor from a string with elements separated by a delimiter string
     * @param src const std::string&, a source string
     * @param delimiter const char *, a delimiter string
     * @param mode SplitMode, delimiter string usage
     */
    Strings(const std::string &src, const char *delimiter, SplitMode mode = SM_DELIMITER) noexcept
    : m_userData(0)
    {
        fromString(src.c_str(), delimiter, mode);
    }

    /**
     * @brief Constructor from a string with elements separated by a delimiter string
     * @param src const char *, a source string
     * @param delimiter const char *, a delimiter string
     * @param mode SplitMode, delimiter string usage
     */
    Strings(const char *src, const char *delimiter, SplitMode mode = SM_DELIMITER) noexcept
    : m_userData(0)
    {
        clear();
        fromString(src, delimiter, mode);
    }

    /**
     * @brief Assignment operator
     * @param other const Strings&, other object
     */
    Strings &operator=(const Strings &other)
    {
        m_userData = other.m_userData;
        assign(other.begin(), other.end());
        return *this;
    }

    /**
     * @brief Assigns strings from a string with elements separated by a delimiter string
     * @param src const std::string&, a source string
     * @param delimiter const char *, a delimiter string
     * @param mode SplitMode, delimiter string usage
     */
    void fromString(const std::string &src, const char *delimiter, SplitMode mode);

    /**
     * @brief Makes string from own strings separated by a delimiter string
     * @param delimiter const char *, a delimiter string
     */
    std::string asString(const char *delimiter) const;

    /**
     * @brief Returns an index of the string in strings, or -1 if not found
     * @param s std::string, a string to find
     * @returns a string index, or -1
     */
    int indexOf(std::string s) const;

    /**
     * @brief Saves strings to file. String ids are discarded.
     * @param fileName std::string, the name of the file
     */
    void saveToFile(std::string fileName) const THROWS_EXCEPTIONS;

    /**
     * @brief Loads strings from file. String ids are not loaded.
     * @param fileName std::string, the name of the file
     */
    void loadFromFile(std::string fileName) THROWS_EXCEPTIONS;

    /**
     * @brief Returns user data as integer
     */
    int32_t argument() const
    {
        return(int) m_userData;
    }

    /**
     * @brief Sets user data as integer
     * @param d int, new value for user data
     */
    void argument(int32_t d)
    {
        m_userData = d;
    }

    /**
     * @brief Removes a string from this object
     * @param i uint32_t, string index in the string vector
     */
    void remove(uint32_t i)
    {
        erase(begin() + i);
    }

    /**
     * @brief Returns concatenated string
     * @param delimiter std::string, delimiter
     */
    std::string join(std::string delimiter) const;

    /**
     * @brief Returns strings matching regex pattern
     * @param pattern std::string, regex pattern
     */
    Strings grep(std::string pattern) const;

    /**
     * @brief Sort strings inside this object
     */
    void sort(bool ascending=true);
};

/**
 * @}
 */
}
#endif
