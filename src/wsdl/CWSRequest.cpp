/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSRequest.cpp  -  description
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

#include <sptk5/wsdl/CWSRequest.h>

using namespace std;
using namespace sptk;

void CWSRequest::processRequest(CXmlDoc* request) THROWS_EXCEPTIONS
{
    CXmlElement* soapEnvelope = NULL;
    for (CXmlElement::iterator itor = request->begin(); itor != request->end(); itor++) {
        CXmlElement* node = dynamic_cast<CXmlElement*>(*itor);
        if (!node)
            continue;
        CStrings nameParts(node->name(),":");
        if (nameParts[1] == "Envelope") {
            soapEnvelope = node;
            m_namespace = nameParts[0];
            break;
        }
    }
    if (!soapEnvelope)
        throwException("Can't find SOAP Envelope node");

    CXmlElement* soapBody = dynamic_cast<CXmlElement*>(soapEnvelope->findFirst(m_namespace + ":Body"));
    if (!soapBody)
        throwException("Can't find SOAP Body node in incoming request");
    
    CXmlElement* requestNode = NULL;
    for (CXmlElement::iterator itor = soapBody->begin(); itor != soapBody->end(); itor++) {
        CXmlElement* node = dynamic_cast<CXmlElement*>(*itor);
        if (node && node->name().find("ns1:") != string::npos) {
            requestNode = node;
            break;
        }
    }
    if (!requestNode)
        throwException("Can't find request node in SOAP Body");

    requestBroker(requestNode);
}
