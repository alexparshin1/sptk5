/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          cintlist.h  -  description
                             -------------------
    begin                : Mon Apr 17 2000
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

#ifndef __CINTLIST_H__
#define __CINTLIST_H__

#include <sptk5/sptk.h>
#include <vector>
#include <algorithm>

namespace sptk {

class CControl;

/// @addtogroup utility Utility Classes
/// @{

/// @brief The list of integers.
///
/// Extends the std::vector<int>
/// with CSV string encoders and decoders.
class SP_EXPORT CIntList : public std::vector<uint32_t>
{
public:
    /// Constructor
    CIntList() {}

    /// Assigns another CIntList
    CIntList& operator = (const CIntList& list)
    {
        resize(list.size());
        std::copy(list.begin(),list.end(),begin());
        return *this;
    }

    /// Converts to string as a list of values, separated with user-defined character
    /// (comma, by default)
    std::string toString(const char * separator=",") const;

    /// Converts from string of a list of values, separated with user-defined character
    /// (comma, by default)
    void fromString(const char * s,const char * separator=",");
};
/// @}
}
#endif
