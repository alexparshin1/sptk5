/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSRequest.h>


using namespace std;
using namespace sptk;

namespace {
void extractNameSpaces(const xdoc::SNode& node, map<String, WSNameSpace>& nameSpaces)
{
    for (const auto& [attr, value]: node->attributes())
    {
        if (!attr.startsWith("xmlns:"))
        {
            continue;
        }
        const auto tagname = WSRequest::tagName(attr);
        nameSpaces[tagname] = WSNameSpace(tagname, value);
    }
}
} // namespace

void WSRequest::requestBroker(const String& requestName, const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                              HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    try
    {
        const auto itor = m_requestMethods.find(requestName);
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
        logError(requestName, e.what(), static_cast<int>(e.statusCode()));
        handleError(xmlContent, jsonContent, e.what(), static_cast<int>(e.statusCode()));
    }
    catch (const Exception& e)
    {
        logError(requestName, e.what(), 0);
        handleError(xmlContent, jsonContent, e.what(), 0);
    }
}

void WSRequest::handleError(const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent, const String& error,
                            int errorCode) const
{
    // Error handling
    if (xmlContent)
    {
        using enum sptk::xdoc::Node::Type;
        const auto& soapBody = xmlContent->parent();
        soapBody->clearChildren();
        String soapNamespace = soapBody->getNameSpace();
        if (!soapNamespace.empty())
        {
            soapNamespace += ":";
        }
        const auto& faultNode = soapBody->pushNode(String(soapNamespace + "Fault"), Object);
        const auto& faultCodeNode = faultNode->pushNode("faultcode", Text);
        faultCodeNode->set(soapNamespace + "Client");
        const auto& faultStringNode = faultNode->pushNode("faultstring", Text);
        faultStringNode->set(error);
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
            logger.error(requestName + ": " + requestName.c_str() + to_string(errorCode) + " " + error);
        }
        else
        {
            logger.error(requestName + ": " + error);
        }
    }
}

xdoc::SNode WSRequest::findSoapBody(const xdoc::SNode& soapEnvelope, const WSNameSpace& soapNamespace)
{
    const scoped_lock lock(*this);

    auto soapBody = soapEnvelope->findFirst(String(soapNamespace.getAlias() + ":Body"));
    if (!soapBody)
    {
        throw Exception("Can't find SOAP Body node in incoming request");
    }

    return soapBody;
}

void WSRequest::processRequest(const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                               HttpAuthentication* authentication, String& requestName)
{
    WSNameSpace requestNameSpace;

    xdoc::SNode xmlRequestNode;
    if (xmlContent)
    {
        WSNameSpace              soapNamespace;
        xdoc::SNode              soapEnvelope;
        map<String, WSNameSpace> allNamespaces;
        for (const auto& node: xmlContent->nodes())
        {
            if (node->getName().toLowerCase() == "envelope")
            {
                soapEnvelope = node;
                const String nameSpaceAlias = node->getNameSpace();
                extractNameSpaces(soapEnvelope, allNamespaces);
                soapNamespace = allNamespaces[nameSpaceAlias];
                break;
            }
        }

        if (soapEnvelope == nullptr)
        {
            throw Exception("Can't find SOAP Envelope node");
        }

        const auto soapBody = findSoapBody(soapEnvelope, soapNamespace);
        if (!soapBody || soapBody->nodes().empty())
        {
            throw Exception("Can't find request node");
        }

        xmlRequestNode = soapBody->nodes().front();

        const scoped_lock lock(*this);
        const String      nameSpaceAlias = xmlRequestNode->getNameSpace();
        extractNameSpaces(xmlRequestNode, allNamespaces);

        if (const auto itor = allNamespaces.find(nameSpaceAlias);
            itor == allNamespaces.end())
        {
            requestNameSpace = WSNameSpace(xmlRequestNode->getNameSpace());
        }
        else
        {
            requestNameSpace = itor->second;
        }

        requestName = xmlRequestNode->getName();
    }
    else
    {
        requestName = jsonContent->getString("rest_method_name");
    }

    requestBroker(requestName, xmlRequestNode, jsonContent, authentication, requestNameSpace);
}

void WSRequest::setRequestMethods(map<sptk::String, RequestMethod>&& requestMethods)
{
    m_requestMethods = std::move(requestMethods);
}

String WSRequest::tagName(const String& nodeName)
{
    const auto pos = nodeName.find(':');
    return pos == string::npos ? "" : nodeName.substr(pos + 1);
}
