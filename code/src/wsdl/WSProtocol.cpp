/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSProtocol.cpp - description                           ║
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

#include <sptk5/wsdl/WSProtocol.h>

using namespace std;
using namespace sptk;

WSStaticHttpProtocol::WSStaticHttpProtocol(TCPSocket *socket, String url, const std::map<String,String>& headers, String staticFilesDirectory)
: WSProtocol(socket, headers), m_url(url), m_staticFilesDirectory(staticFilesDirectory)
{
}

void WSStaticHttpProtocol::process()
{
    Buffer page;
    try {
        page.loadFromFile(m_staticFilesDirectory + m_url);
        m_socket.write("HTTP/1.1 200 OK\n");
        m_socket.write("Content-Type: text/html; charset=utf-8\n");
        m_socket.write("Content-Length: " + int2string(page.bytes()) + "\n\n");
        m_socket.write(page);
    }
    catch (...) {
        string text("<html><head><title>Not Found</title></head><body>Sorry, the page you requested was not found.</body></html>\n");
        m_socket.write("HTTP/1.1 404 Not Found\n");
        m_socket.write("Content-Type: text/html; charset=utf-8\n");
        m_socket.write("Content-length: " + int2string(text.length()) + "\n\n");
        m_socket.write(text);
    }
}
//=============================================================================
WSWebSocketMessage::WSWebSocketMessage()
: m_opcode(0), m_finalMessage(true)
{
}

const Buffer& WSWebSocketMessage::payload()
{
    return m_payload;
}

static uint64_t ntoh64(uint64_t data)
{
    union {
        uint64_t    u64;
        uint32_t    u32[2];
    } transform, output;

    transform.u64 = data;
    output.u32[0] = ntohl(transform.u32[1]);
    output.u32[1] = ntohl(transform.u32[0]);

    return output.u64;
}

static uint64_t hton64(uint64_t data)
{
    union {
        uint64_t    u64;
        uint32_t    u32[2];
    } transform, output;

    transform.u64 = data;
    output.u32[0] = htonl(transform.u32[1]);
    output.u32[1] = htonl(transform.u32[0]);

    return output.u64;
}

void WSWebSocketMessage::decode(const char* incomingData)
{
    const uint8_t* ptr = (const uint8_t*)incomingData;

    m_finalMessage = *ptr & 0x80;
    m_opcode = *ptr & 0xF;

    ptr++;
    bool masked = *ptr & 0x80;
    uint64_t payloadLength = (*ptr) & 0x7F;
    switch (payloadLength) {
        default:    ptr++; break;
        case 126:   ptr++; payloadLength = ntohs(*(const uint16_t*)ptr); ptr += 2; break;
        case 127:   ptr++; payloadLength = ntoh64(*(const uint64_t*)ptr); ptr += 8; break;
    }

    m_payload.checkSize(payloadLength);
    m_payload.bytes(payloadLength);

    if (masked) {
        uint8_t mask[4];
        *(uint32_t *)mask = *(const uint32_t*)ptr;
        ptr += 4;
        char* dest = m_payload.data();
        for (uint64_t i = 0; i < payloadLength; i++) {
            dest[i] = ptr[i] ^ mask[i % 4];
        }
    } else
        m_payload.set((const char*)ptr, payloadLength);
}

void WSWebSocketMessage::encode(String payload, int opcode, bool final, Buffer& output)
{
    output.reset(payload.length() + 10);

    uint8_t* ptr = (uint8_t*) output.data();

    *ptr = opcode & 0xF;
    if (final)
        *ptr |= 0x80;

    ptr++;

    if (payload.length() < 126) {
        *ptr = (uint8_t) payload.length();
        ptr++;
    }
    else if (payload.length() <= 32767) {
        *(uint16_t*)ptr = htons(payload.length());
        ptr += 2;
    }
    else {
        *(uint64_t*)ptr = payload.length();
        ptr += 8;
    }

    output.bytes(ptr - (uint8_t*) output.c_str());
    output.append(payload);
}
//=============================================================================
WSWebSocketsProtocol::WSWebSocketsProtocol(TCPSocket *socket, const std::map<String,String>& headers)
: WSProtocol(socket, headers)
{

}

void WSWebSocketsProtocol::process()
{
}

WSWebServiceProtocol::WSWebServiceProtocol(TCPSocket *socket, const std::map<String,String>& headers, WSRequest& service)
: WSProtocol(socket, headers), m_service(service)
{
}

void WSWebServiceProtocol::process()
{
    int contentLength = 0;
    map<String,String>::const_iterator itor = m_headers.find("Content-Length");
    if (itor != m_headers.end())
        contentLength = string2int(itor->second);

    m_socket.write("<?xml version='1.0' encoding='UTF-8'?><server name='" + m_service.title() + "' version='1.0'/>\n");

    const char* startOfMessage = NULL;
    const char* endOfMessage = NULL;

    Buffer data;

    if (contentLength) {
        m_socket.read(data, contentLength);
        startOfMessage = data.c_str();
        endOfMessage = startOfMessage + data.bytes();
    } else {
        uint32_t socketBytes = m_socket.socketBytes();
        if (!socketBytes) {
            if (!m_socket.readyToRead(30000))
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
            if (!startOfMessage) {
                startOfMessage = strstr(data.c_str(), "<?xml");
                if (!startOfMessage) {
                    startOfMessage = strstr(data.c_str(), "Envelope");
                    if (startOfMessage)
                        while (*startOfMessage != '<' && startOfMessage > data.c_str())
                            startOfMessage--;
                }
                if (!startOfMessage)
                    throwException("Message start <?xml> not found");
            }
            endOfMessage = strstr(startOfMessage, endOfMessageMark);
        } while (!endOfMessage);

        // Message received, processing it
        endOfMessage += strlen(endOfMessageMark);
    }

    sptk::xml::Document message;
    if (endOfMessage)
        *(char *) endOfMessage = 0;
    message.load(startOfMessage);

    //cout << startOfMessage << endl << endl;

    Buffer output;
    m_service.processRequest(&message);
    message.save(output);

    //cout << output.c_str() << endl;

    m_socket.write(output);
}
