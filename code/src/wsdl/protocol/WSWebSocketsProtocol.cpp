/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSWebSocketsProtocol.cpp - description                 ║
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

#include "sptk5/wsdl/protocol/WSWebSocketsProtocol.h"
#include <sptk5/Base64.h>

using namespace std;
using namespace sptk;

const Buffer& WSWebSocketsMessage::payload()
{
    return m_payload;
}

static uint64_t ntoh64(uint64_t data)
{
    union {
        uint64_t    m_uint64;
        uint32_t    m_uint32[2];
    } input, output;

    input.m_uint64 = data;

    output.m_uint32[0] = ntohl(input.m_uint32[1]);
    output.m_uint32[1] = ntohl(input.m_uint32[0]);

    return output.m_uint64;
}

void WSWebSocketsMessage::decode(const char* incomingData)
{
    auto*  ptr = (const uint8_t*) incomingData;

    m_finalMessage = (*ptr & 0x80) != 0;
    m_opcode = uint32_t(*ptr & 0xF);

    ptr++;
    bool masked = (*ptr & 0x80) != 0;
    auto payloadLength = uint64_t((*ptr) & 0x7F);
    switch (payloadLength) {
        case 126:   ptr++; payloadLength = ntohs(*(const uint16_t*)ptr); ptr += 2; break;
        case 127:   ptr++; payloadLength = ntoh64(*(const uint64_t*)ptr); ptr += 8; break;
        default:    ptr++; break;
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

void WSWebSocketsMessage::encode(String payload, OpCode opcode, bool finalMessage, Buffer& output)
{
    output.reset(payload.length() + 10);

    auto*  ptr = (uint8_t*) output.data();

    *ptr = opcode & 0xF;
    if (finalMessage)
        *ptr |= 0x80;

    ptr++;

    if (payload.length() < 126) {
        *ptr = (uint8_t) payload.length();
        ptr++;
    }
    else if (payload.length() <= 32767) {
        *(uint16_t*)ptr = htons((uint16_t)payload.length());
        ptr += 2;
    }
    else {
        *(uint64_t*)ptr = payload.length();
        ptr += 8;
    }

    output.bytes(ptr - (uint8_t*) output.data());
    output.append(payload);
}

WSWebSocketsProtocol::WSWebSocketsProtocol(TCPSocket* socket, const HttpHeaders& headers)
: WSProtocol(socket, headers)
{

}

RequestInfo WSWebSocketsProtocol::process()
{
    RequestInfo requestInfo;
    try {
        String clientKey = headers()["Sec-WebSocket-Key"];
        String socketVersion = headers()["Sec-WebSocket-Version"];
        if (clientKey.empty() || socketVersion != "13")
            throw Exception("WebSocket protocol is missing or has invalid Sec-WebSocket-Key or Sec-WebSocket-Version headers");

        String websocketProtocol = headers()["Sec-WebSocket-Protocol"];

        // Generate server response key from client key
        String responseKey = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        unsigned char obuf[20];
        SHA1((const unsigned char*)responseKey.c_str(), responseKey.length(), obuf);
        Buffer responseKeySHA(obuf, 20);
        Buffer responseKeyEncoded;
        Base64::encode(responseKeyEncoded, responseKeySHA);
        responseKey = responseKeyEncoded.c_str();

        socket().write("HTTP/1.1 101 Switching Protocols\r\n");
        socket().write("Upgrade: websocket\r\n");
        socket().write("Connection: Upgrade\r\n");
        socket().write("Sec-WebSocket-Accept: " + responseKey + "\r\n");
        socket().write("Sec-WebSocket-Protocol: " + websocketProtocol + "\r\n");
        socket().write("\r\n");

        if (socket().readyToRead(chrono::seconds(30))) {
            Buffer message;

            while (socket().socketBytes() == 0)
                this_thread::sleep_for(chrono::milliseconds(100));

            size_t available = socket().socketBytes();
            socket().read(message, available);
            WSWebSocketsMessage msg;
            msg.decode(message.c_str());

            COUT(msg.payload().c_str() << endl)

            WSWebSocketsMessage::encode("Hello", WSWebSocketsMessage::OC_TEXT, true, message);
            socket().write(message);

            WSWebSocketsMessage::encode("World", WSWebSocketsMessage::OC_TEXT, true, message);
            socket().write(message);
        }
    }
    catch (const Exception& e) {
        string text("<html><head><title>Error processing request</title></head><body>" + e.message() + "</body></html>\n");
        socket().write("HTTP/1.1 400 Bad Request\n");
        socket().write("Content-Type: text/html; charset=utf-8\n");
        socket().write("Content-length: " + int2string(text.length()) + "\n\n");
        socket().write(text);
        socket().close();
    }

    return requestInfo;
}
