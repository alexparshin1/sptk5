/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXml.h  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
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

#ifndef __CXML_H__
#define __CXML_H__

#include <sptk5/CException.h>
#include <sptk5/xml/CXmlDoc.h>
#include <sptk5/xml/CXmlNode.h>

namespace sptk {

/// @addtogroup XML
/// @{

/// @brief XML exception
///
/// Xml extension throws CXmlException type exceptions.
/// You should catch always atleast these type of exceptions, when processing XML.
class SP_EXPORT CXmlException: public std::exception
{
    std::string m_message; ///< Exception text
public:

    /// @brief Constructor
    /// @param error const char*, error text
    /// @param xmlbase const char*, parsed xml text
    /// @param position const char*, parsed xml error position
    CXmlException(const char* error, const char* xmlbase, const char* position);

    /// @brief Destructor
    ~CXmlException() DOESNT_THROW
    {
    }

    /// @brief Returns human readable error string.
    const char *what() const DOESNT_THROW
    {
        return m_message.c_str();
    }
};
/// @}
}
#endif
