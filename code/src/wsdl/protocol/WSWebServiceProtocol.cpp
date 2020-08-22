/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>
#include "sptk5/wsdl/protocol/WSWebServiceProtocol.h"

using namespace std;
using namespace sptk;

WSWebServiceProtocol::WSWebServiceProtocol(HttpReader& httpReader, const URL& url, WSRequest& service,
                                           const Host& host, bool allowCORS, bool keepAlive,
                                           bool suppressHttpStatus)
: WSProtocol(&httpReader.socket(), httpReader.getHttpHeaders()),
  m_httpReader(httpReader),
  m_service(service),
  m_url(url),
  m_host(host),
  m_allowCORS(allowCORS),
  m_keepAlive(keepAlive),
  m_suppressHttpStatus(suppressHttpStatus)
{
}

xml::Node* WSWebServiceProtocol::getFirstChildElement(const xml::Node* element) const
{
    for (auto* node: *element) {
        if (node->isElement())
            return node;
    }
    return nullptr;
}

xml::Node* WSWebServiceProtocol::findRequestNode(const xml::Document& message, const String& messageType) const
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

void WSWebServiceProtocol::generateFault(Buffer& output, HttpResponseStatus& httpStatus, String& contentType,
                                         const HTTPException& e,
                                         bool jsonOutput) const
{
    httpStatus.code = e.statusCode();
    httpStatus.description = e.statusText();

    String errorText(e.what());

    // Remove possibly injected scripts
    errorText = errorText.replace("<script.*</script>", "");

    if (jsonOutput) {
        contentType = "application/json";

        json::Document error;
        error.root().set("error", errorText);
        error.root().set("status_code", (int) e.statusCode());
        error.root().set("status_text", e.statusText());
        error.exportTo(output, true);
    } else {
        contentType = "application/xml";

        xml::Document error;
        auto* xmlEnvelope = new xml::Element(&error, "soap:Envelope");
        xmlEnvelope->setAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");

        auto* xmlBody = new xml::Element(xmlEnvelope, "soap:Body");
        auto* faultNode = new xml::Element(xmlBody, "soap:Fault");

        auto* faultcodeNode = new xml::Element(faultNode, "faultCode");
        faultcodeNode->text("soap:Client");

        auto* faultstringNode = new xml::Element(faultNode, "faultString");
        faultstringNode->text(e.what());

        error.save(output, 2);
    }
}

String WSWebServiceProtocol::processMessage(Buffer& output, xml::Document& xmlContent, json::Document& jsonContent,
                                            const SHttpAuthentication& authentication, bool requestIsJSON,
                                            HttpResponseStatus& httpResponseStatus, String& contentType)
{
    String requestName("Error");
    httpResponseStatus.code = 200;
    httpResponseStatus.description = "OK";
    contentType = "text/xml; charset=utf-8";
    try {
        auto* pXmlContent = requestIsJSON? nullptr: &xmlContent;
        auto* pJsonContent = requestIsJSON? &jsonContent: nullptr;
        m_service.processRequest(pXmlContent, pJsonContent, authentication.get(), requestName);
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

void WSWebServiceProtocol::RESTtoSOAP(const URL& url, const char* startOfMessage, xml::Document& message) const
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

static void substituteHostname(Buffer& page, const Host& host)
{
    xml::Document wsdl;
    wsdl.load(page);
    xml::Node* node = wsdl.findFirst("soap:address");
    if (node == nullptr) {
        throw Exception("Can't find <soap:address> in WSDL file");
    }
    String location = (String) node->getAttribute("location", "");
    if (location.empty())
        throw Exception("Can't find location attribute of <soap:address> in WSDL file");
    stringstream listener;
    listener << "http://" << host.toString() << "/";
    location = location.replace("http://([^\\/]+)/", listener.str());
    node->setAttribute("location", location);
    wsdl.save(page, 2);
}

void WSWebServiceProtocol::processXmlContent(const char* startOfMessage, xml::Document& xmlContent,
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

void WSWebServiceProtocol::processJsonContent(const char* startOfMessage, json::Document& jsonContent,
                                              RequestInfo& requestInfo, HttpResponseStatus& httpStatus,
                                              String& contentType) const
{
    if (m_url.path().length() < 2)
        generateFault(requestInfo.response.content(), httpStatus, contentType,
                      HTTPException(404, "Not Found"),
                      true);
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
        for (auto& itor: m_url.params())
            jsonContent.root()[itor.first] = itor.second;
    }
}

RequestInfo WSWebServiceProtocol::process()
{
    HttpResponseStatus  httpStatus { 200, "OK" };
    bool                returnWSDL = false;

    if (m_httpReader.getRequestType() != "POST")
        throw HTTPException(405, "Only POST method is supported");

    const Buffer& contentBuffer = m_httpReader.output();
    m_httpReader.readAll(chrono::seconds(30));

    RequestInfo requestInfo;

    String contentEncoding;
    String contentType(m_httpReader.httpHeader("Content-Type"));
    bool urlEncoded = contentType.find("x-www-form-urlencoded") != string::npos;
    bool requestIsJSON = true;
    if (urlEncoded)
        contentEncoding = "x-www-form-urlencoded";
    else
        contentEncoding = m_httpReader.httpHeader("Content-Encoding");
    requestInfo.request.input(contentBuffer, contentEncoding);

    auto authentication = getAuthentication();

    auto* startOfMessage = requestInfo.request.content().data();
    auto* endOfMessage = startOfMessage + requestInfo.request.content().bytes();

    while (startOfMessage != endOfMessage && (unsigned char)*startOfMessage < 33)
        ++startOfMessage;

    xml::Document xmlContent;
    json::Document jsonContent;

    if (startOfMessage != endOfMessage) {
        *endOfMessage = 0;
        if (*startOfMessage == '<') {
            contentType = "application/xml; charset=utf-8";
            requestIsJSON = false;
            processXmlContent(startOfMessage, xmlContent, jsonContent);
        }
        else if (*startOfMessage == '{' || *startOfMessage == '[') {
            contentType = "application/json; charset=utf-8";
            processJsonContent(startOfMessage, jsonContent, requestInfo, httpStatus,
                               contentType);
        }
        else {
            generateFault(requestInfo.response.content(), httpStatus, contentType,
                          HTTPException(406, "Expect JSON or XML content"),
                          requestIsJSON);
        }
    } else {
        // Empty request content
        if (m_url.params().has("wsdl")) {
            // Requested WSDL content
            returnWSDL = true;
            requestInfo.response.content().set(m_service.wsdl());
            substituteHostname(requestInfo.response.content(), m_host);
            requestInfo.name = "wsdl";
        } else {
            // Regular request w/o content
            RESTtoSOAP(m_url, "", xmlContent);
        }
    }

    if (!returnWSDL && httpStatus.code < 400) {
        requestInfo.name = processMessage(requestInfo.response.content(), xmlContent, jsonContent, authentication,
                                          requestIsJSON,
                                          httpStatus, contentType);
    }

    Strings clientAcceptEncoding(header("accept-encoding"), "[,\\s]+", Strings::SM_REGEXP);

    // For errors, always return uncompressed content
    if (httpStatus.code >= 400)
        clientAcceptEncoding.clear();

    if (httpStatus.code == 500)
        clientAcceptEncoding.clear();

    Buffer outputData = requestInfo.response.output(clientAcceptEncoding);
    contentEncoding = requestInfo.response.contentEncoding();

    Buffer response;
    response.append("HTTP/1.1 ");
    if (m_suppressHttpStatus && httpStatus.code >= 400)
        response.append("202 Accepted\r\n");
    else
        response.append(to_string(httpStatus.code) + " " + httpStatus.description + "\r\n");
    response.append("Content-Type: " + contentType + "\r\n");
    response.append("Content-Length: " + to_string(outputData.bytes()) + "\r\n");
    if (m_keepAlive)
        response.append("Connection: keep-alive\r\n");
    if (m_allowCORS)
        response.append("Access-Control-Allow-Origin: *\r\n");
    if (!contentEncoding.empty())
        response.append("Content-Encoding: " + contentEncoding + "\r\n");

    // OWASP-suggested headers
    response.append("X-Content-Type-Options: nosniff\r\n");

    response.append("\r\n", 2);
    response.append(outputData);
    socket().write(response);

    return requestInfo;
}

shared_ptr<HttpAuthentication> WSWebServiceProtocol::getAuthentication()
{
    shared_ptr<HttpAuthentication> authentication;
    auto itor = headers().find("authorization");
    if (itor != headers().end()) {
        String value(itor->second);
        authentication = make_shared<HttpAuthentication>(value);
    }
    return authentication;
}

int WSWebServiceProtocol::getContentLength()
{
    int contentLength = -1; // Undefined
    if (m_url.params().has("wsdl"))
        contentLength = 0;

    auto itor = headers().find("Content-Length");
    if (itor != headers().end())
        contentLength = string2int(itor->second);
    return contentLength;
}

