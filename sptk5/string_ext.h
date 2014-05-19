/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          string_ext.h  -  description
                             -------------------
    begin                : Thu July 14 2005
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#ifndef __STRING_EXT_H__
#define __STRING_EXT_H__

#include <sptk5/sptk.h>
#include <string>
#include <vector>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// Converts string to upper case
std::string SP_EXPORT upperCase(const std::string& str);

/// Converts string to lower case
std::string SP_EXPORT lowerCase(const std::string& str);

/// Trims string to remove leading and trailing spaces
std::string SP_EXPORT trim(const std::string& str);

/// Converts int32_t to string
std::string SP_EXPORT int2string(int32_t);

/// Converts uint32_t to string
std::string SP_EXPORT int2string(uint32_t);

/// Converts uint64_t to string
std::string SP_EXPORT int2string(int64_t);

/// Converts uint64_t to string
std::string SP_EXPORT int2string(uint64_t);

/// Converts string to integer. The optional default value is used
/// for unsuccessful conversion
int SP_EXPORT string2int(const std::string& str,int defaultValue=0);

/// Replaces all the pattern in str with replacement
std::string SP_EXPORT replaceAll(const std::string& str,const std::string& pattern,const std::string& replacement);

/// Capitalizes all the words in string
std::string SP_EXPORT capitalizeWords(const std::string& str);

/// Converts a string vector into CSV string
void SP_EXPORT join(std::string& dest, const std::vector<std::string> src, std::string separator);

/// Converts a CSV string into string vector
void SP_EXPORT split(std::vector<std::string> dest, const std::string& src, std::string separator);

/// Converts a CSV string into string vector
void SP_EXPORT stringToStringVector(const std::string& src,std::vector<std::string> dest,std::string delimitter);

/// @}

}

#endif
