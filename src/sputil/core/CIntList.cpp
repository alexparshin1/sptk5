/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          cintlist.cpp  -  description
                             -------------------
    begin                : Fri Jul 20 2001
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

#include <sptk5/CStrings.h>
#include <sptk5/CIntList.h>
#include <stdint.h>

using namespace std;
using namespace sptk;

//---------------------------------------------------------------------------
string CIntList::toString (const char * separator) const
{
    string s;
    uint32_t cnt = (uint32_t) size();

    if (!cnt) return s;

    s = int2string ( (uint32_t) (*this) [0]);

    for (uint32_t i = 1; i < cnt; i++)
        s += separator + int2string ( (int32_t) (*this) [i]);

    return s;
}

void CIntList::fromString (const char *s,const char * separator)
{
    string   str = s;
    CStrings sl (s,separator);

    clear();
    uint32_t cnt = (uint32_t) sl.size();

    for (uint32_t i = 0; i < cnt; i++)
        push_back((uint32_t) atol(sl[i].c_str()));
}
