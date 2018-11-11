/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSWebServiceProtocol.cpp - description                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
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

#include "WSWebServiceProtocol.h"

using namespace std;
using namespace sptk;

WSWebServiceProtocol::WSWebServiceProtocol(TCPSocket* socket, const String& url, const HttpHeaders& headers, WSRequest& service)
: WSProtocol(socket, headers), m_service(service), m_url(url)
{
}

void WSWebServiceProtocol::process()
{
    size_t contentLength = 0;
    auto itor = m_headers.find("Content-Length");
    if (itor != m_headers.end())
        contentLength = (size_t) string2int(itor->second);
/*
    cout << endl;
    for (auto htor: m_headers)
        cout << htor.first << ": " << htor.second << endl;
    cout << endl << endl;
*/
    unique_ptr<HttpAuthentication> authentication;
    itor = m_headers.find("authorization");
    if (itor != m_headers.end()) {
        String value(itor->second);
        authentication = unique_ptr<HttpAuthentication>(new HttpAuthentication(value));
    }

    const char* startOfMessage = nullptr;
    const char* endOfMessage = nullptr;

    Buffer data;

    if (contentLength != 0) {
        m_socket.read(data, contentLength);
        startOfMessage = data.c_str();
        endOfMessage = startOfMessage + data.bytes();
    } else {
        size_t socketBytes = m_socket.socketBytes();
        if (socketBytes == 0) {
            if (!m_socket.readyToRead(chrono::seconds(30)))
                throwException("Client disconnected");
            socketBytes = m_socket.socketBytes();
        }

        // If socket is signaled but empty - then other side closed connection
        if (socketBytes == 0)
            throwException("Client disconnected");

        uint32_t offset = 0;
        const char* endOfMessageMark = ":Envelope>";
        do {
            // Read all available data (appending to data buffer)
            data.checkSize(offset + socketBytes);
            socketBytes = (uint32_t) m_socket.read(data.data() + offset, (uint32_t) socketBytes);
            data.bytes(offset + socketBytes);
            //cout << data.c_str() << endl;
            if (startOfMessage == nullptr) {
                startOfMessage = strstr(data.c_str(), "<?xml");
                if (startOfMessage == nullptr) {
                    startOfMessage = strstr(data.c_str(), "Envelope");
                    if (startOfMessage != nullptr)
                        while (*startOfMessage != '<' && startOfMessage > data.c_str())
                            startOfMessage--;
                }
                if (startOfMessage == nullptr)
                    throwException("Message start <?xml> not found");
            }
            endOfMessage = strstr(startOfMessage, endOfMessageMark);
        } while (endOfMessage == nullptr);

        // Message received, processing it
        endOfMessage += strlen(endOfMessageMark);
    }

    while ((unsigned char)*startOfMessage < 33)
        startOfMessage++;

    xml::Document message;
    json::Document jsonContent;

    bool requestIsJSON = false;
    if (*startOfMessage == '<') {
        if (endOfMessage != nullptr)
            *(char*) endOfMessage = 0;
        message.load(startOfMessage);
        xml::Node* xmlEnvelope = message.findFirst("soap:Envelope", true);
        xml::Node* xmlBody = xmlEnvelope->findFirst("soap:Body", true);
        xml::Node* xmlRequest = *xmlBody->begin();

        auto jsonEnvelope = jsonContent.root().set_object(xmlRequest->name());
        xmlRequest->exportTo(*jsonEnvelope);
    }
    else if (*startOfMessage == '{' || *startOfMessage == '[') {
        requestIsJSON = true;
        Strings url(m_url, "/");
        if (url.size() < 2)
            throw Exception("Invalid url");
        // Converting JSON request to XML request
        String method(*url.rbegin());
        auto xmlEnvelope = new xml::Element(message, "soap:Envelope");
        xmlEnvelope->setAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
        auto xmlBody = new xml::Element(xmlEnvelope, "soap:Body");
        jsonContent.load(startOfMessage);
        jsonContent.root().exportTo("ns1:" + method, *xmlBody);
        Buffer buffer;
        message.save(buffer, true);
        //cout << string(buffer.c_str(), buffer.bytes()) << endl;
    }
    else
        throw Exception("Request content isn't XML or JSON");
    //cout << startOfMessage << endl << endl;

    Buffer output;
    size_t httpStatusCode = 200;
    String httpStatusText = "OK";
    String contentType = "text/xml; charset=utf-8";
    try {
        m_service.processRequest(&message, authentication.get());
        if (requestIsJSON) {
            // Converting XML response to JSON response
            xml::Node* bodyElement = message.findFirst("soap:Body");
            if (bodyElement == nullptr)
                throw Exception("Can't find soap:Body in service response");
            xml::Node* methodElement = *bodyElement->begin();
            json::Document jsonOutput;
            auto jsonResponse = jsonOutput.root().set_object("response");
            methodElement->exportTo(*jsonResponse);
            jsonOutput.exportTo(output, false);
            contentType = "application/json";
        }
        else
            message.save(output, 2);
    }
    catch (const HTTPException& e) {
        httpStatusCode = e.statusCode();
        httpStatusText = e.statusText();
        contentType = "application/json";

        json::Document error;
        error.root().set("error", e.what());
        error.root().set("status_code", (int) e.statusCode());
        error.root().set("status_text", e.statusText());
        error.exportTo(output, true);
    }

    stringstream response;
    response << "HTTP/1.1 " << httpStatusCode << " " << httpStatusText << "\n"
             << "Content-Type: " << contentType << "\n"
             << "Content-Length: " << output.bytes() << "\n\n";
    m_socket.write(response.str());
    m_socket.write(output);
}
