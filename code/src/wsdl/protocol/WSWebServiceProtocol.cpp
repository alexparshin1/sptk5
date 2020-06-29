/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSWebServiceProtocol.cpp - description                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
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
                                           const String& hostname, uint16_t port, bool allowCORS, bool keepAlive)
: WSProtocol(&httpReader.socket(), httpReader.getHttpHeaders()),
  m_httpReader(httpReader),
  m_service(service),
  m_url(url),
  m_hostname(hostname),
  m_port(port),
  m_allowCORS(allowCORS),
  m_keepAlive(keepAlive)
{
}

xml::Node* WSWebServiceProtocol::getFirstChildElement(const xml::Node* element) const
{
    xml::Node* methodElement = nullptr;
    for (auto* node: *element) {
        if (node->isElement()) {
            methodElement = node;
            break;
        }
    }
    return methodElement;
}

xml::Node* WSWebServiceProtocol::findRequestNode(const xml::Document& message, const String& messageType) const
{
    String ns = "soap";
    for (auto* node: message) {
        if (lowerCase(node->name()).endsWith(":envelope")) {
            size_t pos = node->name().find(':');
            ns = node->name().substr(0, pos);
        }
    }

    xml::Node* xmlBody = message.findFirst(ns + ":Body", true);
    if (xmlBody == nullptr)
        throw HTTPException(400, "Can't find " + ns + ":Body in " + messageType);
    xml::Node* xmlRequest = getFirstChildElement(xmlBody);
    if (xmlRequest == nullptr)
        throw HTTPException(400, "Can't find request data in " + messageType);

    return xmlRequest;
}

void WSWebServiceProtocol::generateFault(Buffer& output, size_t& httpStatusCode, String& httpStatusText,
                                         String& contentType, const HTTPException& e, bool jsonOutput)
{
    httpStatusCode = e.statusCode();
    httpStatusText = e.statusText();

    if (jsonOutput) {
        contentType = "application/json";

        json::Document error;
        error.root().set("error", e.what());
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
                                            SHttpAuthentication& authentication, bool requestIsJSON,
                                            size_t& httpStatusCode, String& httpStatusText, String& contentType)
{
    String requestName;
    httpStatusCode = 200;
    httpStatusText = "OK";
    contentType = "text/xml; charset=utf-8";
    try {
        auto* pXmlContent = requestIsJSON? nullptr: &xmlContent;
        auto* pJsonContent = requestIsJSON? &jsonContent: nullptr;
        requestName = m_service.processRequest(pXmlContent, pJsonContent, authentication.get());
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
        generateFault(output, httpStatusCode, httpStatusText, contentType, e, requestIsJSON);
        requestName = "Error";
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

static void substituteHostname(Buffer& page, const String& hostname, uint16_t port)
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
    listener << "http://" << hostname << ":" << to_string(port) << "/";
    location = location.replace("http://([^\\/]+)/", listener.str());
    node->setAttribute("location", location);
    wsdl.save(page, 2);
}

RequestInfo WSWebServiceProtocol::process()
{
    size_t httpStatusCode = 200;
    String httpStatusText = "OK";
    bool   returnWSDL = false;

    Buffer& contentBuffer = m_httpReader.output();
    m_httpReader.readAll(chrono::seconds(30));

    RequestInfo requestInfo;

    auto contentEncoding = m_httpReader.httpHeader("Content-Encoding");
    requestInfo.request.input(contentBuffer, contentEncoding);

    String contentType;
    bool requestIsJSON = false;
    switch (requestInfo.request.content()[0]) {
        case '<':
            contentType = "text/xml; charset=utf-8";
            break;
        case '{':
        case '[':
            contentType = "text/json; charset=utf-8";
            requestIsJSON = true;
            break;
        default:
            throw Exception("Message content is not JSON or XML");
    }

    auto authentication = getAuthentication();

    auto* startOfMessage = requestInfo.request.content().data();
    auto* endOfMessage = startOfMessage + requestInfo.request.content().bytes();

    while (startOfMessage != endOfMessage && (unsigned char)*startOfMessage < 33)
        startOfMessage++;

    xml::Document xmlContent;
    json::Document jsonContent;

    if (startOfMessage != endOfMessage) {
        if (*startOfMessage == '<') {
            if (endOfMessage != nullptr)
                *endOfMessage = 0;
            xmlContent.load(startOfMessage);
            xml::Node* xmlRequest = findRequestNode(xmlContent, "API request");
            auto* jsonEnvelope = jsonContent.root().add_object(xmlRequest->name());
            for (auto& itor: m_url.params()) {
                auto* paramNode = new xml::Element(xmlRequest, itor.second.c_str());
                paramNode->text(itor.second);
            }
            xmlRequest->exportTo(*jsonEnvelope);
        }
        else if (*startOfMessage == '{' || *startOfMessage == '[') {
            if (m_url.path().length() < 2)
                generateFault(requestInfo.response.content(), httpStatusCode, httpStatusText, contentType,
                              HTTPException(404, "Not Found"),
                              requestIsJSON);
            else {
                Strings pathElements(m_url.path(), "/");
                String method(*pathElements.rbegin());
                jsonContent.load(startOfMessage);
                jsonContent.root()["rest_method_name"] = method;
                for (auto& itor: m_url.params())
                    jsonContent.root()[itor.first] = itor.second;
            }
        }
        else {
            generateFault(requestInfo.response.content(), httpStatusCode, httpStatusText, contentType,
                          HTTPException(400, "Expect JSON content"),
                          requestIsJSON);
        }
    } else {
        // Empty request content
        if (m_url.params().has("wsdl")) {
            // Requested WSDL content
            returnWSDL = true;
            requestInfo.response.content().set(m_service.wsdl());
            substituteHostname(requestInfo.response.content(), m_hostname, m_port);
            requestInfo.name("wsdl");
        } else {
            // Regular request w/o content
            RESTtoSOAP(m_url, "", xmlContent);
        }
    }

    if (!returnWSDL && httpStatusCode < 400) {
        auto requestName = processMessage(requestInfo.response.content(), xmlContent, jsonContent, authentication,
                                          requestIsJSON,
                                          httpStatusCode, httpStatusText, contentType);
        requestInfo.name(requestName);
    }

    Strings clientAcceptEncoding(header("accept-encoding"), "[,\\s]+", Strings::SM_REGEXP);
    Buffer outputData = requestInfo.response.output(clientAcceptEncoding);
    contentEncoding = requestInfo.response.contentEncoding();

    Buffer response;
    response.append("HTTP/1.1 ");
    response.append(to_string(httpStatusCode) + " " + httpStatusText + "\r\n");
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

void WSWebServiceProtocol::readMessage(Buffer& data, char*& startOfMessage, char*& endOfMessage)
{
    size_t socketBytes = socket().socketBytes();
    if (socketBytes == 0) {
        if (!socket().readyToRead(chrono::seconds(30)))
        throwException("Client disconnected")
        socketBytes = socket().socketBytes();
    }

    // If socket is signaled but empty - then other side closed connection
    if (socketBytes == 0)
    throwException("Client disconnected")

    uint32_t offset = 0;
    const char* endOfMessageMark = ":Envelope>";
    do {
        // Read all available data (appending to data buffer)
        data.checkSize(offset + socketBytes);
        socketBytes = (uint32_t) socket().read(data.data() + offset, (uint32_t) socketBytes);
        data.bytes(offset + socketBytes);

        const char* endOfData = data.c_str() + data.bytes();
        if (startOfMessage == nullptr) {
            startOfMessage = strstr(data.data(), "<?xml");
            if (startOfMessage == nullptr) {
                startOfMessage = strstr(data.data(), "Envelope");
                if (startOfMessage != nullptr && startOfMessage < endOfData)
                    while (*startOfMessage != '<' && startOfMessage > data.c_str())
                        startOfMessage--;
            }
            if (startOfMessage == nullptr)
            throwException("Message start <?xml> not found")
        }
        endOfMessage = strstr(startOfMessage, endOfMessageMark);
    } while (endOfMessage == nullptr);

    // Message received, processing it
    endOfMessage += strlen(endOfMessageMark);
}