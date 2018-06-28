/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Strings.h - description                                ║
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

#ifndef __SPTK_STRINGS_H__
#define __SPTK_STRINGS_H__

#include <sptk5/String.h>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief List of strings with ids
 *
 * General string list. Based on vector<idstring>. Stores strings with (optional) integer Ids.
 * Includes several extra methods to construct it from string or load/save from/to file.
 */
class SP_EXPORT Strings : public std::vector<String>
{
    /**
     * User-specified data
     */
    int32_t m_userData;

    /**
     * Ascending sort compare function, used in sort()
     * @param first             First compared string
     * @param second            Second compared string
     */
    static bool sortAscending(const String& first, const String& second);

    /**
     * Descending sort compare function, used in sort()
     * @param first             First compared string
     * @param second            Second compared string
     */
    static bool sortDescending(const String& first, const String& second);

    /**
     * @brief Splits source string on substrings using exact delimiter
     *
     * Consequent delimiters create empty strings.
     * @param src               Source string
     * @param delimiter         Delimiter string
     */
    void splitByDelimiter(const String& src, const char *delimiter);

    /**
     * @brief Splits source string on substrings using any char in delimiter
     *
     * Consequent delimiters are treated as a single one.
     * @param src               Source string
     * @param delimiter         Delimiter string
     */
    void splitByAnyChar(const String& src, const char *delimiter);

    /**
     * @brief Splits source string on substrings using regular expression
     *
     * Consequent delimiters are treated as a single one.
     * @param src               Source string
     * @param pattern           Regex pattern string
     */
    void splitByRegExp(const String& src, const char *pattern);

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
     * @param src               Other object
     */
    Strings(const Strings &src) noexcept
    : std::vector<String>(src), m_userData(src.m_userData)
    {
    }

    /**
     * @brief Move constructor
     * @param src               Other object
     */
    Strings(Strings&& src) noexcept
    : std::vector<String>(std::move(src)), m_userData(src.m_userData)
    {
    }

    /**
     * @brief Constructor from a string with elements separated by a delimiter string
     * @param src               Source string
     * @param delimiter         Delimiter string
     * @param mode              Delimiter string usage
     */
    Strings(const String& src, const char *delimiter, SplitMode mode = SM_DELIMITER) noexcept
    : m_userData(0)
    {
        try {
            fromString(src.c_str(), delimiter, mode);
        }
        catch (const std::exception& e) {
            push_back("# ERROR: " + String(e.what()));
        }
    }

    /**
     * @brief Constructor from a string with elements separated by a delimiter string
     * @param src               Source string
     * @param delimiter         Delimiter string
     * @param mode              Delimiter string usage
     */
    Strings(const char *src, const char *delimiter, SplitMode mode = SM_DELIMITER) noexcept
    : m_userData(0)
    {
        clear();
        try {
            fromString(src, delimiter, mode);
        }
        catch (const std::exception& e) {
            push_back("# ERROR: " + String(e.what()));
        }
    }

    /**
     * @brief Assignment operator
     * @param other             Other object
     */
    Strings &operator=(const Strings &other)
    {
        m_userData = other.m_userData;
        assign(other.begin(), other.end());
        return *this;
    }

    /**
     * @brief Assigns strings from a string with elements separated by a delimiter string
     * @param src               Source string
     * @param delimiter         Delimiter string
     * @param mode              Delimiter string usage
     */
    void fromString(const String& src, const char *delimiter, SplitMode mode);

    /**
     * @brief Makes string from own strings separated by a delimiter string
     * @param delimiter         Delimiter string
     */
    String asString(const char* delimiter) const;

    /**
     * @brief Returns an index of the string in strings, or -1 if not found
     * @param s                 String to find
     * @returns                 String index, or -1
     */
    int indexOf(const String& s) const;

    /**
     * @brief Saves strings to file. String ids are discarded.
     * @param fileName          The name of the file
     */
    void saveToFile(const String& fileName) const;

    /**
     * @brief Loads strings from file. String ids are not loaded.
     * @param fileName          The name of the file
     */
    void loadFromFile(const String& fileName);

    /**
     * @brief Returns user data as integer
     */
    int32_t argument() const
    {
        return(int) m_userData;
    }

    /**
     * @brief Sets user data as integer
     * @param d                 New value for user data
     */
    void argument(int32_t d)
    {
        m_userData = d;
    }

    /**
     * @brief Removes a string from this object
     * @param i                 String index in the string vector
     */
    void remove(uint32_t i)
    {
        erase(begin() + i);
    }

    /**
     * @brief Returns concatenated string
     * @param delimiter         Delimiter
     */
    String join(const String& delimiter) const;

    /**
     * @brief Returns strings matching regex pattern
     * @param pattern           Regex pattern
     */
    Strings grep(const String& pattern) const;

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
