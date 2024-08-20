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

#include "sptk5/wsdl/protocol/WSWebSocketsProtocol.h"
#include <sptk5/Base64.h>

using namespace std;
using namespace sptk;

static constexpr int finalBitMask(0x80);

const Buffer& WSWebSocketsMessage::payload() const
{
    return m_payload;
}

namespace {
uint64_t ntoh64(uint64_t data)
{
    enum class VarType
    {
        u32,
        u64
    };

    struct IntConvert
    {
        VarType type;
        union
        {
            uint64_t m_uint64;
            array<uint32_t, 2> m_uint32;
        } variant;
    };

    IntConvert input = {};
    IntConvert output = {};

    input.variant.m_uint64 = data;

    output.variant.m_uint32[0] = ntohl(input.variant.m_uint32[1]);
    output.variant.m_uint32[1] = ntohl(input.variant.m_uint32[0]);

    return output.variant.m_uint64;
}
} // namespace

void WSWebSocketsMessage::decode(const char* incomingData)
{
    constexpr int maskedBitMask(0x80);
    constexpr int opcodeBitMask(0xF);
    constexpr int payloadLengthBitMask(0x7F);
    constexpr char lengthIsTwoBytes(126);
    constexpr char lengthIsEightBytes(127);
    constexpr int eightBytes(8);

    const auto* ptr = (const uint8_t*) incomingData;

    m_finalMessage = (static_cast<int>(*ptr) & finalBitMask) != 0;
    m_opcode = static_cast<OpCode>((int) *ptr & opcodeBitMask);

    ++ptr;
    const bool masked = (static_cast<int>(*ptr) & maskedBitMask) != 0;
    auto       payloadLength = static_cast<uint64_t>((int) *ptr & payloadLengthBitMask);
    switch (payloadLength)
    {
        case lengthIsTwoBytes:
            ++ptr;
            payloadLength = ntohs(*(const uint16_t*) ptr);
            ptr += 2;
            break;
        case lengthIsEightBytes:
            ++ptr;
            payloadLength = ntoh64(*(const uint64_t*) ptr);
            ptr += eightBytes;
            break;
        default:
            ++ptr;
            break;
    }

    m_payload.checkSize(payloadLength);
    m_payload.bytes(payloadLength);

    if (masked)
    {
        array<uint8_t, 4> mask {};
        memcpy(mask.data(), ptr, sizeof(mask));
        ptr += 4;
        auto* dest = m_payload.data();
        array<uint8_t, 2> statusCodeBuffer = {};
        size_t j = 0;
        for (uint64_t i = 0; i < payloadLength; ++i)
        {
            const auto unmaskedByte = static_cast<uint8_t>((int) ptr[i] ^ mask[i % 4]);
            if (m_opcode == OpCode::CONNECTION_CLOSE && i < 2)
            {
                statusCodeBuffer[i] = unmaskedByte;
                continue;
            }
            dest[j] = unmaskedByte;
            ++j;
        }
        m_payload.bytes(j);
        if (m_opcode == OpCode::CONNECTION_CLOSE)
        {
            m_status = ntohs(*(const uint16_t*) statusCodeBuffer.data());
        }
    }
    else
    {
        if (m_opcode == OpCode::CONNECTION_CLOSE)
        {
            m_status = ntohs(*(const uint16_t*) ptr);
            ptr += 2;
            payloadLength -= 2;
        }
        m_payload.set(ptr, payloadLength);
    }
}

void WSWebSocketsMessage::encode(const String& payload, OpCode opcode, bool finalMessage, Buffer& output)
{
    output.reset(payload.length() + 10);

    auto* ptr = output.data();

    *ptr = static_cast<int>(opcode) & 0xF;
    if (finalMessage)
    {
        *ptr = static_cast<uint8_t>((int) *ptr | finalBitMask);
    }

    ++ptr;

    if (payload.length() < 126)
    {
        *ptr = static_cast<uint8_t>(payload.length());
        ++ptr;
    }
    else if (payload.length() <= 32767)
    {
        *(uint16_t*) ptr = htons(static_cast<uint16_t>(payload.length()));
        ptr += 2;
    }
    else
    {
        *(uint64_t*) ptr = payload.length();
        ptr += 8;
    }

    output.bytes(ptr - output.data());
    output.append(payload);
}

WSWebSocketsMessage::OpCode WSWebSocketsMessage::opcode() const
{
    return m_opcode;
}

void WSWebSocketsMessage::opcode(OpCode code)
{
    m_opcode = code;
}

uint32_t WSWebSocketsMessage::statusCode() const
{
    return m_status;
}

bool WSWebSocketsMessage::isFinal() const
{
    return m_finalMessage;
}

WSWebSocketsProtocol::WSWebSocketsProtocol(TCPSocket* socket, const HttpHeaders& headers)
    : WSProtocol(socket, headers)
{
}

RequestInfo WSWebSocketsProtocol::process()
{
    constexpr size_t shaBufferLength(20);
    constexpr chrono::seconds thirtySeconds(30);
    constexpr int connectionTerminatedCode(1000);

    RequestInfo requestInfo;
    try
    {
        const String clientKey = headers()["Sec-WebSocket-Key"];
        if (const String socketVersion = headers()["Sec-WebSocket-Version"];
            clientKey.empty() || socketVersion != "13")
        {
            throw Exception(
                "WebSocket protocol is missing or has invalid Sec-WebSocket-Key or Sec-WebSocket-Version headers");
        }

        const String websocketProtocol = headers()["Sec-WebSocket-Protocol"];

        // Generate server response key from client key
        String responseKey = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        Buffer obuf(shaBufferLength);
        SHA1((const unsigned char*) responseKey.c_str(), responseKey.length(), obuf.data());
        const Buffer responseKeySHA(obuf.data(), shaBufferLength);
        Buffer responseKeyEncoded;
        Base64::encode(responseKeyEncoded, responseKeySHA);
        responseKey = responseKeyEncoded.c_str();

        socket().write("HTTP/1.1 101 Switching Protocols\r\n");
        socket().write("Upgrade: websocket\r\n");
        socket().write("Connection: Upgrade\r\n");
        socket().write("Sec-WebSocket-Accept: " + responseKey + "\r\n");
        if (!websocketProtocol.empty())
        {
            socket().write("Sec-WebSocket-Protocol: " + websocketProtocol + "\r\n");
        }
        socket().write("\r\n");

        bool connectionCloseRequestReplied = false;
        bool clientClosedConnection = false;
        while (!clientClosedConnection)
        {
            if (!socket().readyToRead(thirtySeconds))
            {
                continue;
            }

            const size_t available = socket().socketBytes();
            if (available == 0)
            {
                clientClosedConnection = true;
                continue;
            }

            Buffer message;
            if (socket().read(message, available) != available)
            {
                throw Exception("Incomplete read");
            }

            WSWebSocketsMessage msg;
            msg.decode(message.c_str());

            if (msg.opcode() == WSWebSocketsMessage::OpCode::CONNECTION_CLOSE)
            {
                replyCloseConnectionRequest(static_cast<uint16_t>(msg.statusCode()), "Connection closed by client request");
                connectionCloseRequestReplied = true;
                break;
            }

            COUT(static_cast<int>(msg.opcode()) << ": " << msg.payload().c_str());

            WSWebSocketsMessage::encode("Hello", WSWebSocketsMessage::OpCode::TEXT, true, message);
            socket().write(message);

            WSWebSocketsMessage::encode("World", WSWebSocketsMessage::OpCode::TEXT, true, message);
            socket().write(message);
        }

        if (!connectionCloseRequestReplied)
        {
            replyCloseConnectionRequest(connectionTerminatedCode, "Connection terminated");
        }
    }
    catch (const Exception& e)
    {
        const string text(
            "<html><head><title>Error processing request</title></head><body>" + e.message() + "</body></html>\n");
        socket().write("HTTP/1.1 400 Bad Request\n");
        socket().write("Content-Type: text/html; charset=utf-8\n");
        socket().write("Content-length: " + int2string(text.length()) + "\n\n");
        socket().write(text);
        socket().close();
    }

    return requestInfo;
}

void WSWebSocketsProtocol::replyCloseConnectionRequest(uint16_t statusCode, const String& closeReason)
{
    Buffer message;

    String reply("  ");
    *(uint16_t*) reply.data() = htons(statusCode);
    reply.append(closeReason);
    WSWebSocketsMessage::encode(reply, WSWebSocketsMessage::OpCode::CONNECTION_CLOSE, true, message);

    socket().write(message);
}
