/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CStrings.h  -  description
                             -------------------
    begin                : Thu August 11 2005
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#ifndef __CSTRINGS_H__
#define __CSTRINGS_H__

#include <sptk5/sptk.h>
#include <sptk5/string_ext.h>
#include <sptk5/CException.h>
#include <string>
#include <vector>
#include <algorithm>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// A workaround for VC++ bug
typedef std::string std_string;

/// @brief String with ID
/// Extended version of std::string that supports an integer string ID
class SP_EXPORT idstring : public std_string {
public:
    int32_t       m_id;    ///< String ID
public:
    /// Default constructor
    idstring() {
        m_id = 0;
    }

    /// Copy constructor
    /// @param str const idstring&, source string
    idstring(const idstring& str) : std_string(str), m_id(str.m_id) {}

    /// Constructor
    /// @param str const string&, source string
    /// @param id in, optional string id
    idstring(const std::string& str,int32_t id=0) : std_string(str), m_id(id) {}

    /// Constructor
    /// @param str const char *, source string
    /// @param id in, optional string id
    idstring(const char *str,int32_t id=0) : std_string(str), m_id(id) {}

    /// Assignment operator
    idstring& operator = (const idstring& si)  {
        assign(si);
        m_id = si.m_id;
        return *this;
    }

    /// Assignment operator
    /// @param str const char *, source string
    idstring& operator = (const char * str) {
        assign(str);
        m_id = 0;
        return *this;
    }

    /// Returns string ID
    int32_t ident() const {
        return m_id;
    }

    /// Sets string ID
    void ident(int32_t id) {
        m_id = id;
    }
};

/// @brief List of strings with ids
///
/// General string list. Based on vector<idstring>. Stores strings with (optional) integer Ids.
/// Includes several extra methods to construct it from string or load/save from/to file.
class CStrings : public std::vector<idstring> {

    int32_t m_userData;    ///< User-specified data

    /// Parses a string to elements separated by a delimiter string
    /// @param src const char *, a source string
    /// @param separator const char *, a separator string
    void parseString(const char *src,const char *delimitter);
public:
    /// @brief Default constructor
    CStrings() {
        m_userData = 0;
    }

    /// @brief Copy constructor
    CStrings(const CStrings& src) : std::vector<idstring>() {
        resize(src.size());
        std::copy(src.begin(),src.end(),begin());
    }

    /// @brief Constructor from a string with elements separated by a delimiter string
    /// @param src const std::string&, a source string
    /// @param separator const char *, a separator string
    CStrings(const std::string& src,const char *separator) {
        parseString(src.c_str(),separator);
    }

    /// @brief Constructor from a string with elements separated by a delimiter string
    /// @param src const char *, a source string
    /// @param separator const char *, a separator string
    CStrings(const char *src,const char *separator) {
        clear();
        parseString(src,separator);
    }

    /// @brief Assigns strings from a string with elements separated by a delimiter string
    /// @param src const std::string&, a source string
    /// @param separator const char *, a separator string
    void fromString(const std::string& src,const char *separator) {
        clear();
        parseString(src.c_str(),separator);
    }

    /// @brief Makes string from own strings separated by a delimiter string
    /// @param separator const char *, a separator string
    std::string asString(const char *separator) const;

    /// @brief Returns an index of the string in strings, or -1 if not found
    /// @param s std::string, a string to find
    /// @returns a string index, or -1
    int indexOf(std::string s) const;

    /// @brief Saves strings to file. String ids are discarded.
    /// @param fileName std::string, the name of the file
    void saveToFile(std::string fileName) const throw(CException);

    /// @brief Loads strings from file. String ids are not loaded.
    /// @param fileName std::string, the name of the file
    void loadFromFile(std::string fileName) throw(CException);

    /// @brief Returns user data as integer
    int32_t argument() const   {
        return (int)m_userData;
    }

    /// @brief Sets user data as integer
    /// @param d int, new value for user data
    void argument(int32_t d)   {
        m_userData = d;
    }

    /// @brief Removes a string from vector
    /// @param i uint32_t, string index in the vector
    void remove(uint32_t i) {
        erase(begin()+i);
    }
};
/// @}
}
#endif
