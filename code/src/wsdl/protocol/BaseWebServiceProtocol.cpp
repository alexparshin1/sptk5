/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/wsdl/protocol/WSWebServiceProtocol.h"
#include <sptk5/ZLib.h>

using namespace std;
using namespace sptk;

constexpr int httpOkCode = 200;
constexpr int minHttpErrorCode = 400;
constexpr int httpNotFoundErrorCode = 404;
constexpr int httpInvalidContentErrorCode = 406;

BaseWebServiceProtocol::BaseWebServiceProtocol(TCPSocket* socket, const HttpHeaders& headers,
                                               sptk::WSServices& services, const URL& url)
    : WSProtocol(socket, headers)
    , m_services(services)
    , m_url(url)
{
}

xdoc::SNode BaseWebServiceProtocol::getFirstChildElement(const xdoc::SNode& element)
{
    for (const auto& node: element->nodes())
    {
        bool isElement =
            node->type() != xdoc::Node::Type::ProcessingInstruction &&
            node->type() != xdoc::Node::Type::Comment;
        if (isElement)
        {
            return node;
        }
    }
    return nullptr;
}

xdoc::SNode BaseWebServiceProtocol::findRequestNode(const xdoc::SNode& message, const String& messageType)
{
    String ns = "soap";
    for (const auto& node: message->nodes())
    {
        if (lowerCase(node->name()).endsWith(":envelope"))
        {
            size_t pos = node->name().find(':');
            ns = node->name().substr(0, pos);
        }
    }

    const auto xmlBody = message->findFirst(ns + ":Body");
    if (xmlBody == nullptr)
    {
        throw HTTPException(minHttpErrorCode, "Can't find " + ns + ":Body in " + messageType);
    }

    auto xmlRequest = getFirstChildElement(xmlBody);
    if (!xmlRequest)
    {
        throw HTTPException(minHttpErrorCode, "Can't find request data in " + messageType);
    }

    return xmlRequest;
}

void BaseWebServiceProtocol::RESTtoSOAP(const URL& url, const char* startOfMessage, const xdoc::SNode& message)
{
    // Converting JSON request to XML request
    xdoc::Document jsonContent;
    Strings pathElements(url.path(), "/");
    String method(*pathElements.rbegin());
    const auto& xmlEnvelope = message->pushNode("soap:Envelope");
    xmlEnvelope->attributes().set("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");

    const auto& xmlBody = xmlEnvelope->pushNode("soap:Body");
    jsonContent.root()->load(xdoc::DataFormat::JSON, startOfMessage);
    auto& jsonRoot = *jsonContent.root();
    for (const auto& [name, value]: url.params())
    {
        jsonRoot.set(name, value);
    }

    Buffer xmlBuffer;
    jsonRoot.exportTo(xdoc::DataFormat::XML, xmlBuffer, false);
    xmlBody->load(xdoc::DataFormat::XML, xmlBuffer);
}

xdoc::SNode BaseWebServiceProtocol::processXmlContent(const char* startOfMessage, const xdoc::SNode& xmlContent) const
{
    try
    {
        xmlContent->load(xdoc::DataFormat::XML, startOfMessage, false);
    }
    catch (const Exception& e)
    {
        throw HTTPException(httpInvalidContentErrorCode, "Invalid XML content: " + String(e.what()));
    }

    auto xmlRequest = findRequestNode(xmlContent, "API request");
    for (const auto& [name, param]: m_url.params())
    {
        xmlRequest->set(name, param);
    }

    String methodName;
    if (auto pos = xmlRequest->name().find(':');
        pos == string::npos)
    {
        methodName = xmlRequest->name();
    }
    else
    {
        methodName = xmlRequest->name().substr(pos + 1);
    }
    xmlRequest->set("rest_method_name", methodName);

    return xmlRequest;
}

String BaseWebServiceProtocol::processMessage(Buffer& output, const xdoc::SNode& xmlContent,
                                              const xdoc::SNode& jsonContent,
                                              const SHttpAuthentication& authentication, bool requestIsJSON,
                                              HttpResponseStatus& httpResponseStatus, String& contentType) const
{
    String requestName("Error");
    httpResponseStatus.code = httpOkCode;
    httpResponseStatus.description = "OK";
    contentType = "text/xml; charset=utf-8";
    try
    {
        auto pXmlContent = requestIsJSON ? nullptr : xmlContent;
        auto pJsonContent = requestIsJSON ? jsonContent : nullptr;
        auto& service = m_services.get(m_url.location());
        service.processRequest(pXmlContent, pJsonContent, authentication.get(), requestName);
        if (requestIsJSON)
        {
            jsonContent->exportTo(xdoc::DataFormat::JSON, output, false);
            contentType = "application/json";
        }
        else
        {
            xmlContent->exportTo(xdoc::DataFormat::XML, output, true);
            contentType = "application/xml";
        }
    }
    catch (const HTTPException& e)
    {
        generateFault(output, httpResponseStatus, contentType, e, requestIsJSON);
    }
    return requestName;
}

void BaseWebServiceProtocol::processJsonContent(const char* startOfMessage, const xdoc::SNode& jsonContent,
                                                RequestInfo& requestInfo, HttpResponseStatus& httpStatus,
                                                String& contentType) const
{
    if (m_url.path().length() < 2)
    {
        generateFault(requestInfo.response.content(), httpStatus, contentType,
                      HTTPException(httpNotFoundErrorCode, "Not Found"), true);
    }
    else
    {
        Strings pathElements(m_url.path(), "/");
        String method(*pathElements.rbegin());

        try
        {
            jsonContent->load(xdoc::DataFormat::JSON, startOfMessage);
        }
        catch (const Exception& e)
        {
            generateFault(requestInfo.response.content(), httpStatus, contentType,
                          HTTPException(httpInvalidContentErrorCode, "Invalid JSON content: " + String(e.what())),
                          true);
        }

        jsonContent->set("rest_method_name", method);
        for (const auto& [name, value]: m_url.params())
        {
            jsonContent->set(name, value);
        }
    }
}
