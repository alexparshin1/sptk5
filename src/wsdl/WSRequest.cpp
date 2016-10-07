/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSRequest.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/wsdl/WSRequest.h>

using namespace std;
using namespace sptk;

void WSRequest::processRequest(sptk::XMLDocument* request) THROWS_EXCEPTIONS
{
    XMLElement* soapEnvelope = NULL;
    for (XMLElement::iterator itor = request->begin(); itor != request->end(); ++itor) {
        XMLElement* node = dynamic_cast<XMLElement*>(*itor);
        if (!node)
            continue;
        Strings nameParts(node->name(),":");
        if (nameParts.size() > 1 && nameParts[1] == "Envelope") {
            soapEnvelope = node;
            {
                SYNCHRONIZED_CODE;
                m_namespace = nameParts[0] + ":";
            }
            break;
        }
        else if (node->name() == "Envelope") {
            soapEnvelope = node;
            break;
        }
    }

    if (!soapEnvelope)
        throwException("Can't find SOAP Envelope node");

    XMLElement* soapBody = dynamic_cast<XMLElement*>(soapEnvelope->findFirst(nameSpace() + "Body"));
    if (!soapBody)
        throwException("Can't find SOAP Body node in incoming request");

    XMLElement* requestNode = NULL;
    for (XMLElement::iterator itor = soapBody->begin(); itor != soapBody->end(); ++itor) {
        XMLElement* node = dynamic_cast<XMLElement*>(*itor);
        if (node) {
            requestNode = node;
            break;
        }
    }
    if (!requestNode)
        throwException("Can't find request node in SOAP Body");

    requestBroker(requestNode);
}
