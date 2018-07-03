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

WSWebServiceProtocol::WSWebServiceProtocol(TCPSocket *socket, const HttpHeaders& headers, WSRequest& service)
: WSProtocol(socket, headers), m_service(service)
{
}

void WSWebServiceProtocol::process()
{
    size_t contentLength = 0;
    auto itor = m_headers.find("Content-Length");
    if (itor != m_headers.end())
        contentLength = (size_t) string2int(itor->second);

    cout << endl;
    for (auto itor: m_headers)
        cout << itor.first << ": " << itor.second << endl;
    cout << endl;

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

    sptk::XMLDocument message;
    if (endOfMessage != nullptr)
        *(char *) endOfMessage = 0;
    message.load(startOfMessage);

    //cout << startOfMessage << endl << endl;

    Buffer output;
    size_t httpStatusCode = 200;
    String httpStatusText = "OK";
    String contentType = "text/xml; charset=utf-8";
    try {
        m_service.processRequest(&message, authentication.get());
        message.save(output, 2);
    }
    catch (const HTTPException& e) {
        httpStatusCode = e.statusCode();
        httpStatusText = e.statusText();
        contentType = "application/json";

        json::Document error;
        error.root().add("error", e.what());
        error.root().add("status_code", (int) e.statusCode());
        error.root().add("status_text", e.statusText());
        error.exportTo(output, true);
    }

    stringstream response;
    response << "HTTP/1.1 " << httpStatusCode << " " << httpStatusText << "\n"
             << "Content-Type: " << contentType << "\n"
             << "Content-Length: " + int2string(output.bytes()) + "\n\n";
    m_socket.write(output);

    //cout << output.c_str() << endl;
}
