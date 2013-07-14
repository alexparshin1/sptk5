/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSharedStrings.h  -  description
                             -------------------
    begin                : June 1 2006
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

#ifndef __CSHAREDSTRINGS_H__
#define __CSHAREDSTRINGS_H__

#include <string>
#include <map>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Shared strings table
///
/// Contains a table of shared strings for the use with different objects
/// such as XML node, ListView, etc
class SP_EXPORT CSharedStrings
{
    typedef std::map<std::string, int> CSIMap; ///< String to int map type
    CSIMap m_stringIdMap; ///< Map of shared strings and reference counters
public:
    /// @brief Default constructor
    CSharedStrings();

    /// @brief Destructor
    ~CSharedStrings()
    {
        clear();
    }

    /// @brief Obtain a shared string
    ///
    /// Looks for an existing shared string, and returns a const std::string&
    /// to it. If a shared string not found, it's created with reference count one.
    /// For an existing shared string, every call of this method encreases string
    /// reference counter by one
    /// @param str const char *, a string to share
    const std::string& shareString(const char *str);

    /// @brief Obtain a shared string
    ///
    /// Looks for an existing shared string, and returns a const char *pointer
    /// to it. If a shared string not found, it's created with reference count one.
    /// For an existing shared string, every call of this method encreases string
    /// reference counter by one
    /// @param str const std::string&, a string to share
    const std::string& shareString(const std::string& str) 
    {
        return shareString(str.c_str());
    }

    /// @brief Releases a shared string
    ///
    /// If the shared string reference counter is greater than one,
    /// it's decreased by one. If the reference counter is one,
    /// the string is removed from SST. If the string doesn't exist,
    /// the exception is thrown.
    /// @param str const char *, a string to release
    void releaseString(const char *str) throw(std::exception);

    /// @brief Releases a shared string
    ///
    /// If the shared string reference counter is greater than one,
    /// it's decreased by one. If the reference counter is one,
    /// the string is removed from SST. If the string doesn't exist,
    /// the exception is thrown.
    /// @param str const std::string&, a string to release
    void releaseString(const std::string& str) throw(std::exception)
    {
        releaseString(str.c_str());
    }

    /// @brief Clears the shared string table
    ///
    /// Only the string containing the empty string ("") is left
    /// after this operation
    void clear();
}
;
/// @}
}

#endif
