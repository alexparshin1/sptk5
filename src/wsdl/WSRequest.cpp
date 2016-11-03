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
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;

static void extractNameSpaces(XMLNode* node, map<String,WSNameSpace>& nameSpaces)
{
    for (XMLNode* attribute: node->attributes()) {
        if (attribute->nameSpace() != "xmlns")
            continue;
        nameSpaces[attribute->tagname()] = WSNameSpace(attribute->tagname(), attribute->text());
    }
}

void WSRequest::processRequest(XMLDocument* request) THROWS_EXCEPTIONS
{
    XMLElement*             soapEnvelope = NULL;
    map<String,WSNameSpace> allNameSpaces;
    for (XMLElement::iterator itor = request->begin(); itor != request->end(); ++itor) {
        XMLElement* node = dynamic_cast<XMLElement*>(*itor);
        if (!node)
            continue;
        if (node->tagname() == "Envelope") {
            soapEnvelope = node;
            {
                SYNCHRONIZED_CODE;
                String nameSpaceAlias = node->nameSpace();
                extractNameSpaces(soapEnvelope, allNameSpaces);
                m_soapNamespace = allNameSpaces[nameSpaceAlias];
            }
            break;
        }
    }

    if (!soapEnvelope)
        throwException("Can't find SOAP Envelope node");

    XMLElement* soapBody;
    {
        SYNCHRONIZED_CODE;
        soapBody = dynamic_cast<XMLElement*>(soapEnvelope->findFirst(m_soapNamespace.getAlias() + ":Body"));
        if (!soapBody)
            throwException("Can't find SOAP Body node in incoming request");
    }

    XMLElement* requestNode = NULL;
    for (XMLElement::iterator itor = soapBody->begin(); itor != soapBody->end(); ++itor) {
        XMLElement* node = dynamic_cast<XMLElement*>(*itor);
        if (node) {
            requestNode = node;
            String nameSpaceAlias = requestNode->nameSpace();
            extractNameSpaces(requestNode, allNameSpaces);
            m_requestNamespace = allNameSpaces[nameSpaceAlias];
            break;
        }
    }
    if (!requestNode)
        throwException("Can't find request node in SOAP Body");

    requestBroker(requestNode);
}
