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
 * List of strings with ids
 *
 * General string list. Based on vector<idstring>. Stores strings with (optional) integer Ids.
 * Includes several extra methods to construct it from string or load/save from/to file.
 */
class SP_EXPORT Strings
{
public:
    /**
     * Sort order enumeration
     */
    enum SortOrder {
        UNSORTED,
        ASCENDING,
        DESCENDING
    };

    /**
     * Regular forward iterator type
     */
    typedef std::vector<String>::iterator               iterator;

    /**
     * Const forward iterator type
     */
    typedef std::vector<String>::const_iterator         const_iterator;

    /**
     * Regular reverse iterator type
     */
    typedef std::vector<String>::reverse_iterator       reverse_iterator;

    /**
     * Const reverse iterator type
     */
    typedef std::vector<String>::const_reverse_iterator const_reverse_iterator;

private:
    /**
     * Actual strings
     */
    std::vector<String> m_strings;

    /**
     * User-specified data
     */
    int32_t             m_userData {0};

    /**
     * Is sorted flag
     */
    SortOrder           m_sorted {UNSORTED};

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
     * Splits source string on substrings using exact delimiter
     *
     * Consequent delimiters create empty strings.
     * @param src               Source string
     * @param delimiter         Delimiter string
     */
    void splitByDelimiter(const String& src, const char *delimiter);

    /**
     * Splits source string on substrings using any char in delimiter
     *
     * Consequent delimiters are treated as a single one.
     * @param src               Source string
     * @param delimiter         Delimiter string
     */
    void splitByAnyChar(const String& src, const char *delimiter);

    /**
     * Splits source string on substrings using regular expression
     *
     * Consequent delimiters are treated as a single one.
     * @param src               Source string
     * @param pattern           Regex pattern string
     */
    void splitByRegExp(const String& src, const char *pattern);

public:

    /**
     * String split mode
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
     * Default constructor
     */
    Strings() noexcept
    {
        m_userData = 0;
    }

    /**
     * Copy constructor
     * @param src               Other object
     */
    Strings(const Strings &src) noexcept
    : m_strings(src.m_strings), m_userData(src.m_userData)
    {
    }

    /**
     * Move constructor
     * @param src               Other object
     */
    Strings(Strings&& src) noexcept
    : m_strings(std::move(src.m_strings)), m_userData(src.m_userData)
    {
    }

    /**
     * Constructor from a string with elements separated by a delimiter string
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
     * Constructor from a string with elements separated by a delimiter string
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
     * Assignment operator
     * @param other             Other object
     */
    Strings &operator=(const Strings &other)
    {
        m_userData = other.m_userData;
        m_strings.assign(other.m_strings.begin(), other.m_strings.end());
        return *this;
    }

    /**
     * Assigns strings from a string with elements separated by a delimiter string
     * @param src               Source string
     * @param delimiter         Delimiter string
     * @param mode              Delimiter string usage
     */
    void fromString(const String& src, const char *delimiter, SplitMode mode);

    /**
     * Makes string from own strings separated by a delimiter string
     * @param delimiter         Delimiter string
     */
    String asString(const char* delimiter) const;

    /**
     * Returns an index of the string in strings, or -1 if not found.
     * If strings were sorted prior to calling this method, and not modified
     * since that, then binary search is used.
     * @param s                 String to find
     * @returns                 String index, or -1
     */
    int indexOf(const String& s) const;

    /**
     * Saves strings to file. String ids are discarded.
     * @param fileName          The name of the file
     */
    void saveToFile(const String& fileName) const;

    /**
     * Loads strings from file. String ids are not loaded.
     * @param fileName          The name of the file
     */
    void loadFromFile(const String& fileName);

    /**
     * Returns user data as integer
     */
    int32_t argument() const
    {
        return(int) m_userData;
    }

    /**
     * Sets user data as integer
     * @param d                 New value for user data
     */
    void argument(int32_t d)
    {
        m_userData = d;
    }

    /**
     * Removes a string from this object
     * @param i                 String index in the string vector
     */
    void remove(uint32_t i)
    {
        m_strings.erase(m_strings.begin() + i);
    }

    /**
     * Returns concatenated string
     * @param delimiter         Delimiter
     */
    String join(const String& delimiter) const;

    /**
     * Returns strings matching regex pattern
     * @param pattern           Regex pattern
     */
    Strings grep(const String& pattern) const;

    /**
     * Sort strings inside this object
     */
    void sort(bool ascending=true);

    /**
     * Clear strings
     */
    void clear()
    {
        m_sorted = ASCENDING;
        m_strings.clear();
        m_userData = 0;
    }

    /**
     * Get size of the collection
     * @return number of strings in the collection
     */
    size_t size() const noexcept
    {
        return m_strings.size();
    }

    /**
     * Is the collection empty?
     * @return true if the collection is empty
     */
    bool empty() const noexcept
    {
        return m_strings.empty();
    }

    /**
     * Change size of the collection
     * @param size              New number of strings in the collection
     */
    void resize(size_t size)
    {
        if (size > m_strings.size())
            m_sorted = UNSORTED;
        m_strings.resize(size);
    }

    /**
     * Pre-allocate size of the collection
     * @param size              New number of strings in the collection
     */
    void reserve(size_t size)
    {
        m_strings.reserve(size);
    }

    /**
     * Push back a string
     */
    void push_back(const String& str)
    {
        m_sorted = UNSORTED;
        m_strings.push_back(str);
    }

    /**
     * Push back a string
     */
    void push_back(String&& str)
    {
        m_sorted = UNSORTED;
        m_strings.push_back(str);
    }

    /**
     * Emplace back a string
     */
    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        m_strings.emplace_back(args...);
    }

    /**
     * Get the collection start
     * @return begin iterator
     */
    iterator begin() noexcept
    {
        return m_strings.begin();
    }

    /**
     * Get the collection start
     * @return begin iterator
     */
    const_iterator begin() const noexcept
    {
        return m_strings.begin();
    }

    /**
     * Get the collection end
     * @return end iterator
     */
    iterator end() noexcept
    {
        return m_strings.end();
    }

    /**
     * Get the collection end
     * @return end iterator
     */
    const_iterator end() const noexcept
    {
        return m_strings.end();
    }

    /**
     * Get the collection reverse start
     * @return rbegin iterator
     */
    reverse_iterator rbegin() noexcept
    {
        return m_strings.rbegin();
    }

    /**
     * Get the collection reverse start
     * @return rbegin iterator
     */
    const_reverse_iterator rbegin() const noexcept
    {
        return m_strings.rbegin();
    }

    /**
     * Get the collection reverse end
     * @return rend iterator
     */
    reverse_iterator rend() noexcept
    {
        return m_strings.rend();
    }

    /**
     * Get the collection reverse end
     * @return rend iterator
     */
    const_reverse_iterator rend() const noexcept
    {
        return m_strings.rend();
    }

    /**
     * Index operator
     * @param index             String index
     * @return string by the index
     */
    String& operator[] (size_t index)
    {
        return m_strings[index];
    }

    /**
     * Index operator
     * @param index             String index
     * @return string by the index
     */
    const String& operator[] (size_t index) const
    {
        return m_strings[index];
    }

    /**
     * Erase string at the iterator position
     * @param itor              Iterator of the position
     */
    void erase(iterator itor)
    {
        m_strings.erase(itor);
    }

    /**
     * Erase strings between the iterator positions
     * @param from              Iterator of the from position
     * @param to                Iterator of the to position
     */
    void erase(iterator from, iterator to)
    {
        m_strings.erase(from, to);
    }
};

/**
 * @}
 */
}
#endif
