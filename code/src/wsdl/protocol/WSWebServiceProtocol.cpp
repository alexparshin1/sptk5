/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSWebServiceProtocol.cpp - description                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

using namespace std;
using namespace sptk;

WSWebServiceProtocol::WSWebServiceProtocol(TCPSocket* socket, const String& url, const HttpHeaders& headers,
                                           WSRequest& service, const String& hostname, uint16_t port)
: WSProtocol(socket, headers), m_service(service), m_url(url), m_hostname(hostname), m_port(port)
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

        auto* faultcodeNode = new xml::Element(faultNode, "faultcode");
        faultcodeNode->text("soap:Client");

        auto* faultstringNode = new xml::Element(faultNode, "faultcode");
        faultstringNode->text(e.what());

        error.save(output, 2);
    }
}

void WSWebServiceProtocol::processMessage(Buffer& output, xml::Document& message,
                                          shared_ptr<HttpAuthentication> authentication, bool requestIsJSON,
                                          size_t& httpStatusCode, String& httpStatusText, String& contentType)
{
    httpStatusCode = 200;
    httpStatusText = "OK";
    contentType = "text/xml; charset=utf-8";
    try {
        m_service.processRequest(&message, authentication.get());
        if (requestIsJSON) {
            xml::Node* methodElement = findRequestNode(message, "service response");

            // Converting XML response to JSON response
            json::Document jsonOutput;
            auto* jsonResponse = jsonOutput.root().set_object("response");
            methodElement->exportTo(*jsonResponse);
            jsonOutput.exportTo(output, false);
            contentType = "application/json";
        }
        else
            message.save(output, 2);
    }
    catch (const HTTPException& e) {
        generateFault(output, httpStatusCode, httpStatusText, contentType, e, false);
    }
}

void WSWebServiceProtocol::RESTtoSOAP(Strings& url, const char* startOfMessage, xml::Document& message) const
{
    // Converting JSON request to XML request
    json::Document jsonContent;
    String method(*url.rbegin());
    auto* xmlEnvelope = new xml::Element(message, "soap:Envelope");
    xmlEnvelope->setAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
    auto* xmlBody = new xml::Element(xmlEnvelope, "soap:Body");
    jsonContent.load(startOfMessage);
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

void WSWebServiceProtocol::process()
{
    String contentType = "text/xml; charset=utf-8";
    auto ctor = headers().find("Content-Type");
    if (ctor != headers().end())
        contentType = ctor->second;
    bool requestIsJSON = contentType.startsWith("application/json");

    auto contentLength = getContentLength();
    auto authentication = getAuthentication();

    char* startOfMessage = nullptr;
    char* endOfMessage = nullptr;

    Buffer data;

    if (contentLength > 0) {
        socket().read(data, contentLength);
        startOfMessage = data.data();
        endOfMessage = startOfMessage + data.bytes();
    }
    else if (contentLength == 0) {
        startOfMessage = data.data();
        endOfMessage = startOfMessage;
    } else {
        readMessage(data, startOfMessage, endOfMessage);
    }

    while (startOfMessage != endOfMessage && (unsigned char)*startOfMessage < 33)
        startOfMessage++;

    Buffer output;
    size_t httpStatusCode = 200;
    String httpStatusText = "OK";
    bool   returnWSDL = false;

    xml::Document message;
    json::Document jsonContent;

    if (startOfMessage != endOfMessage) {
        if (*startOfMessage == '<') {
            if (endOfMessage != nullptr)
                *endOfMessage = 0;
            message.load(startOfMessage);
            xml::Node* xmlRequest = findRequestNode(message, "API request");
            auto* jsonEnvelope = jsonContent.root().set_object(xmlRequest->name());
            xmlRequest->exportTo(*jsonEnvelope);
        } else if (*startOfMessage == '{' || *startOfMessage == '[') {
            Strings url(m_url, "/");
            if (url.size() < 2)
                throw Exception("Invalid url");
            RESTtoSOAP(url, startOfMessage, message);
        } else {
            generateFault(output, httpStatusCode, httpStatusText, contentType,
                          HTTPException(400, "Expect JSON content"),
                          requestIsJSON);
        }
    } else {
        // Empty request content
        if (m_url.endsWith("?wsdl")) {
            // Requested WSDL content
            returnWSDL = true;
            output.set(m_service.wsdl());
            substituteHostname(output, m_hostname, m_port);
        } else {
            // Regular request w/o content
            Strings url(m_url, "/");
            RESTtoSOAP(url, "", message);
        }
    }

    if (!returnWSDL && httpStatusCode < 400)
        processMessage(output, message, authentication, requestIsJSON, httpStatusCode, httpStatusText, contentType);

    stringstream response;
    response << "HTTP/1.1 " << httpStatusCode << " " << httpStatusText << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << output.bytes() << "\r\n\r\n";
    socket().write(response.str());
    socket().write(output);
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
    if (m_url.endsWith("?wsdl"))
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