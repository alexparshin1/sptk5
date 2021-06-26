/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/wsdl/WSParser.h>

using namespace std;
using namespace sptk;

static void extractNameSpaces(xml::Node* node, map<String, WSNameSpace>& nameSpaces)
{
    for (const auto* attributeNode: node->attributes())
    {
        const auto* attribute = dynamic_cast<const xml::Attribute*>(attributeNode);
        if (attribute == nullptr)
        {
            continue;
        }
        if (attribute->nameSpace() != "xmlns")
        {
            continue;
        }
        nameSpaces[attribute->tagname()] = WSNameSpace(attribute->tagname(), attribute->value());
    }
}

void WSRequest::requestBroker(const String& requestName, xml::Element* xmlContent, json::Element* jsonContent,
                              HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    try
    {
        auto itor = m_requestMethods.find(requestName);
        if (itor == m_requestMethods.end())
        {
            throw SOAPException("Request '" + requestName + "' is not defined in this service");
        }
        itor->second(xmlContent, jsonContent, authentication, requestNameSpace);
    }
    catch (const SOAPException& e)
    {
        logError(requestName, e.what(), 0);
        handleError(xmlContent, jsonContent, e.what(), 0);
    }
    catch (const HTTPException& e)
    {
        logError(requestName, e.what(), (int) e.statusCode());
        handleError(xmlContent, jsonContent, e.what(), (int) e.statusCode());
    }
    catch (const Exception& e)
    {
        logError(requestName, e.what(), 0);
        handleError(xmlContent, jsonContent, e.what(), 0);
    }
}

void WSRequest::handleError(xml::Element* xmlContent, json::Element* jsonContent, const String& error,
                            int errorCode) const
{
    // Error handling
    if (xmlContent)
    {
        auto* soapBody = (xml::Element*) xmlContent->parent();
        soapBody->clearChildren();
        String soap_namespace = WSParser::get_namespace(soapBody->name());
        if (!soap_namespace.empty())
        {
            soap_namespace += ":";
        }
        auto* faultNode = new xml::Element(soapBody, (soap_namespace + "Fault").c_str());
        auto* faultCodeNode = new xml::Element(faultNode, "faultcode");
        faultCodeNode->text(soap_namespace + "Client");
        auto* faultStringNode = new xml::Element(faultNode, "faultstring");
        faultStringNode->text(error);
        new xml::Element(faultNode, "detail");
    }
    else
    {
        jsonContent->clear();
        if (errorCode != 0)
        {
            jsonContent->set("error_code", errorCode);
        }
        jsonContent->set("error_description", error);
    }
}

void WSRequest::logError(const String& requestName, const String& error, int errorCode) const
{
    if (m_logEngine)
    {
        Logger logger(*m_logEngine);
        if (errorCode != 0)
        {
            logger.error(requestName + ": " + to_string(errorCode) + " " + error);
        }
        else
        {
            logger.error(requestName + ": " + error);
        }
    }
}

xml::Element* WSRequest::findSoapBody(const xml::Element* soapEnvelope, const WSNameSpace& soapNamespace)
{
    scoped_lock lock(*this);

    auto* soapBody = dynamic_cast<xml::Element*>(soapEnvelope->findFirst(soapNamespace.getAlias() + ":Body"));
    if (soapBody == nullptr)
    throwException("Can't find SOAP Body node in incoming request")

    return soapBody;
}

void WSRequest::processRequest(xml::Document* xmlContent, json::Document* jsonContent,
                               HttpAuthentication* authentication, String& requestName)
{
    WSNameSpace requestNameSpace;

    if (xmlContent)
    {
        WSNameSpace soapNamespace;
        xml::Element* soapEnvelope = nullptr;
        map<String, WSNameSpace> allNamespaces;
        for (auto* anode: *xmlContent)
        {
            auto* node = dynamic_cast<xml::Element*>(anode);
            if (node == nullptr)
            {
                continue;
            }
            if (node->tagname() == "Envelope")
            {
                soapEnvelope = node;
                String nameSpaceAlias = node->nameSpace();
                extractNameSpaces(soapEnvelope, allNamespaces);
                soapNamespace = allNamespaces[nameSpaceAlias];
                break;
            }
        }

        if (soapEnvelope == nullptr)
        throwException("Can't find SOAP Envelope node")

        const xml::Element* soapBody = findSoapBody(soapEnvelope, soapNamespace);

        xml::Element* requestNode = nullptr;
        for (auto* anode: *soapBody)
        {
            auto* node = dynamic_cast<xml::Element*>(anode);
            if (node != nullptr)
            {
                scoped_lock lock(*this);
                requestNode = node;
                String nameSpaceAlias = requestNode->nameSpace();
                extractNameSpaces(requestNode, allNamespaces);
                requestNameSpace = allNamespaces[nameSpaceAlias];
                break;
            }
        }
        if (requestNode == nullptr)
        throwException("Can't find request node in SOAP Body")

        requestName = WSParser::strip_namespace(requestNode->name());
    }
    else
    {
        requestName = (String) jsonContent->root()["rest_method_name"];
    }

    json::Element* jsonNode = jsonContent ? &jsonContent->root() : nullptr;
    requestBroker(requestName, xmlContent, jsonNode, authentication, requestNameSpace);
}

void WSRequest::setRequestMethods(map<sptk::String, RequestMethod>&& requestMethods)
{
    m_requestMethods = move(requestMethods);
}
