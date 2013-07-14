/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CStringStack.h  -  description
                             -------------------
    begin                : Mon Aug 4 2003
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

#ifndef __CSTRINGSTACK_H__
#define __CSTRINGSTACK_H__

#include <sptk/CStringList.h>

namespace sptk {

/// Simple String Stack
class CStringStack : protected std::vector<std::string> {
public:
   /// Default constructor
   CStringStack() {}

   /// Pushes a string into the stack
   /// @param str const char *, string to push
   void push(const char *str) { push_back(str); };

   /// Pushes a string into the stack
   /// @param str const CString&, string to push
   void push(const std::string& str) { push_back(str); };

   /// Pops a string from the stack
   /// @returns string from the stack
   std::string pop();

   /// Returns a stack's top string reference
   /// @returns string from the stack
   std::string& top();

   /// Returns true if the stack is empty
   bool empty() const { return size() == 0; }
};

}

#endif
