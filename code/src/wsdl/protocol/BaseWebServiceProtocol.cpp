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

#include "sptk5/wsdl/protocol/WSWebServiceProtocol.h"
#include <sptk5/ZLib.h>
#include <sptk5/Brotli.h>
#include <sptk5/wsdl/protocol/BaseWebServiceProtocol.h>

using namespace std;
using namespace sptk;

BaseWebServiceProtocol::BaseWebServiceProtocol(TCPSocket* socket, const HttpHeaders& headers,
                                               sptk::WSServices& services, const URL& url)
: WSProtocol(socket, headers),
  m_services(services),
  m_url(url)
{
}

xml::Node* BaseWebServiceProtocol::getFirstChildElement(const xml::Node* element)
{
    for (auto* node: *element) {
        if (node->isElement())
            return node;
    }
    return nullptr;
}

xml::Node* BaseWebServiceProtocol::findRequestNode(const xml::Document& message, const String& messageType) const
{
    String ns = "soap";
    for (const auto* node: message) {
        if (lowerCase(node->name()).endsWith(":envelope")) {
            size_t pos = node->name().find(':');
            ns = node->name().substr(0, pos);
        }
    }

    const xml::Node* xmlBody = message.findFirst(ns + ":Body", true);
    if (xmlBody == nullptr)
        throw HTTPException(400, "Can't find " + ns + ":Body in " + messageType);

    xml::Node* xmlRequest = getFirstChildElement(xmlBody);
    if (xmlRequest == nullptr)
        throw HTTPException(400, "Can't find request data in " + messageType);

    return xmlRequest;
}

void BaseWebServiceProtocol::RESTtoSOAP(const URL& url, const char* startOfMessage, xml::Document& message)
{
    // Converting JSON request to XML request
    json::Document jsonContent;
    Strings pathElements(url.path(), "/");
    String method(*pathElements.rbegin());
    auto* xmlEnvelope = new xml::Element(message, "soap:Envelope");
    xmlEnvelope->setAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
    auto* xmlBody = new xml::Element(xmlEnvelope, "soap:Body");
    jsonContent.load(startOfMessage);
    for (auto& itor: url.params())
        jsonContent.root()[itor.first] = itor.second;
    jsonContent.root().exportTo("ns1:" + method, *xmlBody);
}

void BaseWebServiceProtocol::processXmlContent(const char* startOfMessage, xml::Document& xmlContent,
                                               json::Document& jsonContent) const
{
    try {
        xmlContent.load(startOfMessage, false);
    }
    catch (const Exception& e) {
        throw HTTPException(406, "Invalid XML content: " + String(e.what()));
    }

    xml::Node* xmlRequest = findRequestNode(xmlContent, "API request");
    auto* jsonEnvelope = jsonContent.root().add_object(xmlRequest->name());
    for (auto& itor: m_url.params()) {
        auto* paramNode = new xml::Element(xmlRequest, itor.second.c_str());
        paramNode->text(itor.second);
    }
    xmlRequest->exportTo(*jsonEnvelope);
}

String BaseWebServiceProtocol::processMessage(Buffer& output, xml::Document& xmlContent, json::Document& jsonContent,
                                              const SHttpAuthentication& authentication, bool requestIsJSON,
                                              HttpResponseStatus& httpResponseStatus, String& contentType) const
{
    String requestName("Error");
    httpResponseStatus.code = 200;
    httpResponseStatus.description = "OK";
    contentType = "text/xml; charset=utf-8";
    try {
        auto* pXmlContent = requestIsJSON? nullptr: &xmlContent;
        auto* pJsonContent = requestIsJSON? &jsonContent: nullptr;
        auto& service = m_services.get(m_url.location());
        service.processRequest(pXmlContent, pJsonContent, authentication.get(), requestName);
        if (pJsonContent) {
            pJsonContent->exportTo(output, false);
            contentType = "application/json";
        }
        else {
            xmlContent.save(output, 2);
            contentType = "application/xml";
        }
    }
    catch (const HTTPException& e) {
        generateFault(output, httpResponseStatus, contentType, e, requestIsJSON);
    }
    return requestName;
}

void BaseWebServiceProtocol::processJsonContent(const char* startOfMessage, json::Document& jsonContent,
                                                RequestInfo& requestInfo, HttpResponseStatus& httpStatus,
                                                String& contentType) const
{
    if (m_url.path().length() < 2)
        generateFault(requestInfo.response.content(), httpStatus, contentType,
                      HTTPException(404, "Not Found"), true);
    else {
        Strings pathElements(m_url.path(), "/");
        String method(*pathElements.rbegin());

        try {
            jsonContent.load(startOfMessage);
        }
        catch (const Exception& e) {
            generateFault(requestInfo.response.content(), httpStatus, contentType,
                          HTTPException(406, "Invalid JSON content: " + String(e.what())),
                          true);
        }

        jsonContent.root()["rest_method_name"] = method;
        for (const auto& itor: m_url.params())
            jsonContent.root()[itor.first] = itor.second;
    }
}