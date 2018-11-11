/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSRequest.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/wsdl/WSRequest.h>

using namespace std;
using namespace sptk;

static void extractNameSpaces(xml::Node* node, map<String,WSNameSpace>& nameSpaces)
{
    for (auto attributeNode: node->attributes()) {
        auto attribute = dynamic_cast<xml::Attribute*>(attributeNode);
        if (attribute == nullptr)
            continue;
        if (attribute->nameSpace() != "xmlns")
            continue;
        nameSpaces[attribute->tagname()] = WSNameSpace(attribute->tagname(), attribute->value());
    }
}

void WSRequest::processRequest(sptk::xml::Document* request, HttpAuthentication* authentication)
{
    WSNameSpace             soapNamespace, requestNameSpace;
    xml::Element*           soapEnvelope = nullptr;
    map<String,WSNameSpace> allNamespaces;
    for (auto anode: *request) {
        auto node = dynamic_cast<xml::Element*>(anode);
        if (node == nullptr)
            continue;
        if (node->tagname() == "Envelope") {
            soapEnvelope = node;
            String nameSpaceAlias = node->nameSpace();
            extractNameSpaces(soapEnvelope, allNamespaces);
            soapNamespace = allNamespaces[nameSpaceAlias];
            break;
        }
    }

    if (soapEnvelope == nullptr)
        throwException("Can't find SOAP Envelope node");

    xml::Element* soapBody;
    {
        lock_guard<mutex> lock(*this);
        soapBody = dynamic_cast<xml::Element*>(soapEnvelope->findFirst(soapNamespace.getAlias() + ":Body"));
        if (soapBody == nullptr)
            throwException("Can't find SOAP Body node in incoming request");
    }

    xml::Element* requestNode = nullptr;
    for (auto anode: *soapBody) {
        auto node = dynamic_cast<xml::Element*>(anode);
        if (node != nullptr) {
            std::lock_guard<std::mutex> lock(*this);
            requestNode = node;
            String nameSpaceAlias = requestNode->nameSpace();
            extractNameSpaces(requestNode, allNamespaces);
            requestNameSpace = allNamespaces[nameSpaceAlias];
            break;
        }
    }
    if (requestNode == nullptr)
        throwException("Can't find request node in SOAP Body");

    requestBroker(requestNode, authentication, requestNameSpace);
}
