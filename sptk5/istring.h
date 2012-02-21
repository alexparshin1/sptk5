/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          istring.h  -  description
                             -------------------
    begin                : July 19 2005
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __ISTRING_H__
#define __ISTRING_H__

#include <sptk5/sptk.h>
#include <algorithm>

namespace sptk {
/// @addtogroup utility Utility Classes
/// @{

/// @brief Case-insensitive string.
///
/// Lower case string class istring is really useful if we need
/// a case-independent string map
class istring : public std_string {

    /// Converts string to lower case
	void tolower() {
		std::transform(begin(),end(),begin(),(int(*)(int))::tolower);
	}

public:

    /// Default constructor
    istring() : std_string() {}

    /// Copy constructor
    /// @param s const string &, a source string
    istring(const istring& s) : std_string(s.c_str()) {
        tolower();
    }

    /// Constructor
    /// @param s const char *, a source string
    istring(const char *s) : std_string(s) {
        tolower();
    }

    /// Constructor
    /// @param s const string &, a source string
    istring(const std::string& s) : std_string(s) {
        tolower();
    }

    /// Assignment
    /// @param s const char *, a source string
    /// @returns this object
    istring& operator = (const char *s) {
        assign(s);
        return *this;
    }

    /// Assignment
    /// @param s const string &, a source string
    /// @returns this object
    istring& operator = (const std::string& s) {
        assign(s);
        return *this;
    }
};

/// @}

}

#endif
