/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSRequest.h  -  description
                             -------------------
    begin                : 03 Aug 2012
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

#ifndef __CWSREQUEST_H__
#define __CWSREQUEST_H__

#include <sptk5/cxml>

namespace sptk
{

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Parser of WSDL requests
class CWSRequest
{
protected:
    std::string m_namespace;
    virtual void requestBroker(CXmlElement* requestNode) THROWS_EXCEPTIONS = 0;
public:
    /// @brief Constructor
    CWSRequest() {}

    /// @brief Destructor
    virtual ~CWSRequest() {}

    /// @brief Processes incoming requests
    ///
    /// The processing results are stored in the same request XML
    /// @param request CXmlDoc*, Incoming request and outgoing response
    void processRequest(CXmlDoc* request) THROWS_EXCEPTIONS;
};

}
#endif
