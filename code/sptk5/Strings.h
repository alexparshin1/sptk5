/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

class Exception;

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
class SP_EXPORT Strings : public std::vector<String>
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
    Strings() = default;

    /**
     * Copy constructor
     * @param src               Other object
     */
    Strings(const Strings& src) noexcept
    : StringVector(src), m_userData(src.m_userData)
    {
    }

    /**
     * Initializer list constructor
     * @param list              Initializer list
     */
    Strings(std::initializer_list<String> list)
    {
        std::copy(list.begin(), list.end(), back_inserter(*this));
    }

    /**
     * Move constructor
     * @param src               Other object
     */
    Strings(Strings&& src) noexcept
    : StringVector(std::move(src)), m_userData(src.m_userData)
    {
    }

    /**
     * Constructor from a string with elements separated by a delimiter string
     * @param src               Source string
     * @param delimiter         Delimiter string
     * @param mode              Delimiter string usage
     */
    Strings(const String& src, const char *delimiter, SplitMode mode = SM_DELIMITER) noexcept;

    /**
     * Constructor from a string with elements separated by a delimiter string
     * @param argc              Number of arguments
     * @param argv              Arguments
     */
    Strings(int argc, const char *argv[]) noexcept;

    /**
     * Destructor
     */
    ~Strings() noexcept = default;

    /**
     * Assignment operator
     * @param other             Other object
     */
    Strings &operator=(const Strings &other)
    {
        if (&other != this) {
            m_userData = other.m_userData;
            assign(other.begin(), other.end());
        }
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
     * Returns an index of the string in strings, or -1 if not found.
     * If strings were sorted prior to calling this method, and not modified
     * since that, then binary search is used.
     * @param s                 String to find
     * @returns                 String index, or -1
     */
    virtual int indexOf(const String& s) const;

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
    int64_t argument() const
    {
        return(int) m_userData;
    }

    /**
     * Sets user data as integer
     * @param d                 New value for user data
     */
    void argument(int64_t d)
    {
        m_userData = d;
    }

    /**
     * Removes a string from this object
     * @param i                 String index in the string vector
     */
    iterator remove(size_t i)
    {
        return StringVector::erase(begin() + i);
    }

    /**
     * Removes a string from this object
     * @param str               String to remove from the string vector
     */
    iterator remove(const String& str)
    {
        auto itor = std::find(begin(), end(), str);
        if (itor != end())
            return StringVector::erase(itor);
        return end();
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
        m_sorted = UNSORTED;
        StringVector::clear();
        m_userData = 0;
    }

    /**
     * Change size of the collection
     * @param size              New number of strings in the collection
     */
    void resize(size_t size)
    {
        if (size > this->size())
            m_sorted = UNSORTED;
        StringVector::resize(size);
    }

    /**
     * Push back a string
     */
    void push_back(const String& str)
    {
        m_sorted = UNSORTED;
        StringVector::push_back(str);
    }

    /**
     * Push back a string
     */
    void push_back(String&& str)
    {
        m_sorted = UNSORTED;
        StringVector::push_back(std::move(str));
    }

    /**
     * Emplace back a string
     */
    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        m_sorted = UNSORTED;
        StringVector::emplace_back(args...);
    }

    /**
     * Index operator
     * @param index             String index
     * @return string by the index
     */
    String& operator[] (size_t index)
    {
        m_sorted = UNSORTED;
        return StringVector::operator[](index);
    }

    /**
     * Index operator
     * @param index             String index
     * @return string by the index
     */
    const String& operator[] (size_t index) const
    {
        return StringVector::operator[](index);
    }

private:

    typedef std::vector<String> StringVector;

    /**
     * User-specified data
     */
    int64_t             m_userData {0};

    /**
     * Is sorted flag
     */
    SortOrder           m_sorted {UNSORTED};
};

/**
 * @}
 */
}
#endif
