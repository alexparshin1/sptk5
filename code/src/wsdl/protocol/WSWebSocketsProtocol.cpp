/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "sptk5/wsdl/protocol/WSWebSocketsProtocol.h"
#include "sptk5/wsdl/protocol/WSWebSocketStream.h"

#include <sptk5/Base64.h>

#include <array>
#include <chrono>
#include <cstring>
#include <random>

using namespace std;
using namespace sptk;

namespace {
constexpr uint8_t finalBitMask = 0x80;
constexpr uint8_t reservedBitsMask = 0x70;
constexpr uint8_t opcodeBitMask = 0x0F;
constexpr uint8_t maskedBitMask = 0x80;
constexpr uint8_t payloadLengthBitMask = 0x7F;
constexpr uint8_t lengthIsTwoBytes = 126;
constexpr uint8_t lengthIsEightBytes = 127;
constexpr size_t  maskLength = 4;

/// The GUID that RFC 6455 appends to the client's key before hashing it.
constexpr const char* webSocketsGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

bool isControlOpcode(WSWebSocketsMessage::OpCode opcode)
{
    return static_cast<uint8_t>(opcode) >= 0x8;
}

bool isKnownOpcode(uint8_t opcode)
{
    switch (opcode)
    {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x8:
        case 0x9:
        case 0xA:
            return true;
        default:
            return false;
    }
}

uint16_t readBigEndian16(const uint8_t* data)
{
    return static_cast<uint16_t>((static_cast<uint16_t>(data[0]) << 8) | data[1]);
}

uint64_t readBigEndian64(const uint8_t* data)
{
    uint64_t value = 0;
    for (size_t i = 0; i < 8; ++i)
    {
        value = (value << 8) | data[i];
    }
    return value;
}

/// A fresh mask for every frame: RFC 6455 requires it to be unpredictable, and reusing one lets an
/// observer recover the payload by comparing frames.
array<uint8_t, maskLength> randomMask()
{
    static thread_local mt19937 generator(random_device{}());
    static thread_local uniform_int_distribution<int> distribution(0, 255);
    array<uint8_t, maskLength> mask {};
    for (auto& byte: mask)
    {
        byte = static_cast<uint8_t>(distribution(generator));
    }
    return mask;
}

void appendFrameHeader(Buffer& output, uint64_t payloadLength, WSWebSocketsMessage::OpCode opcode, bool finalMessage,
                       const uint8_t* maskKey)
{
    uint8_t header[2 + 8 + maskLength];
    size_t  headerLength = 0;

    header[headerLength++] =
        static_cast<uint8_t>((finalMessage ? finalBitMask : 0) | (static_cast<uint8_t>(opcode) & opcodeBitMask));

    const auto maskBit = maskKey == nullptr ? uint8_t(0) : maskedBitMask;
    if (payloadLength < lengthIsTwoBytes)
    {
        header[headerLength++] = static_cast<uint8_t>(maskBit | static_cast<uint8_t>(payloadLength));
    }
    else if (payloadLength <= 0xFFFF)
    {
        header[headerLength++] = static_cast<uint8_t>(maskBit | lengthIsTwoBytes);
        header[headerLength++] = static_cast<uint8_t>(payloadLength >> 8);
        header[headerLength++] = static_cast<uint8_t>(payloadLength & 0xFF);
    }
    else
    {
        header[headerLength++] = static_cast<uint8_t>(maskBit | lengthIsEightBytes);
        for (int shift = 56; shift >= 0; shift -= 8)
        {
            header[headerLength++] = static_cast<uint8_t>((payloadLength >> shift) & 0xFF);
        }
    }

    if (maskKey != nullptr)
    {
        memcpy(header + headerLength, maskKey, maskLength);
        headerLength += maskLength;
    }

    output.append(header, headerLength);
}
} // namespace

size_t WSWebSocketsMessage::decode(const uint8_t* data, size_t available)
{
    m_payload.reset();
    m_status = 0;
    m_masked = false;
    m_declaredPayloadLength = 0;

    if (available < 2)
    {
        return 0;
    }

    const uint8_t first = data[0];
    if ((first & reservedBitsMask) != 0)
    {
        throw Exception("WebSockets frame has reserved bits set, which this implementation never negotiates");
    }

    const uint8_t opcodeValue = first & opcodeBitMask;
    if (!isKnownOpcode(opcodeValue))
    {
        throw Exception("WebSockets frame has an unknown opcode " + to_string(opcodeValue));
    }

    m_opcode = static_cast<OpCode>(opcodeValue);
    m_finalMessage = (first & finalBitMask) != 0;
    m_masked = (data[1] & maskedBitMask) != 0;

    if (isControlOpcode(m_opcode) && !m_finalMessage)
    {
        throw Exception("WebSockets control frame may not be fragmented");
    }

    uint64_t payloadLength = data[1] & payloadLengthBitMask;
    size_t   offset = 2;
    if (payloadLength == lengthIsTwoBytes)
    {
        if (available < offset + 2)
        {
            return 0;
        }
        payloadLength = readBigEndian16(data + offset);
        offset += 2;
    }
    else if (payloadLength == lengthIsEightBytes)
    {
        if (available < offset + 8)
        {
            return 0;
        }
        payloadLength = readBigEndian64(data + offset);
        offset += 8;
        if (payloadLength > maxPayloadLength)
        {
            throw Exception("WebSockets frame declares a payload longer than 2^63-1 bytes");
        }
    }

    if (isControlOpcode(m_opcode))
    {
        if (payloadLength > maxControlPayloadLength)
        {
            throw Exception("WebSockets control frame carries more than 125 bytes");
        }
        if (m_opcode == OpCode::CONNECTION_CLOSE && payloadLength == 1)
        {
            throw Exception("WebSockets close frame carries a status code of one byte");
        }
    }

    const size_t headerLength = offset + (m_masked ? maskLength : 0);
    m_declaredPayloadLength = payloadLength;
    if (available < headerLength || payloadLength > available - headerLength)
    {
        // The frame is still arriving. Nothing is consumed, so the caller keeps the bytes.
        return 0;
    }

    const auto* payload = data + headerLength;
    m_payload.set(payload, static_cast<size_t>(payloadLength));

    if (m_masked)
    {
        const uint8_t* maskKey = data + offset;
        for (size_t i = 0; i < payloadLength; ++i)
        {
            m_payload[i] ^= maskKey[i % maskLength];
        }
    }

    if (m_opcode == OpCode::CONNECTION_CLOSE && payloadLength >= 2)
    {
        m_status = readBigEndian16(m_payload.data());

        // A close frame's status code is not part of what a caller reads as the close reason, so
        // the payload is left holding the text alone. Control payloads are at most 125 bytes,
        // which is what makes a stack copy reasonable here.
        uint8_t reason[maxControlPayloadLength];
        const size_t reasonLength = static_cast<size_t>(payloadLength) - 2;
        memcpy(reason, m_payload.data() + 2, reasonLength);
        m_payload.set(reason, reasonLength);
    }

    return headerLength + static_cast<size_t>(payloadLength);
}

void WSWebSocketsMessage::encodeHeader(uint64_t payloadLength, OpCode opcode, bool finalMessage, Buffer& output)
{
    if (isControlOpcode(opcode))
    {
        if (!finalMessage)
        {
            throw Exception("WebSockets control frame may not be fragmented");
        }
        if (payloadLength > maxControlPayloadLength)
        {
            throw Exception("WebSockets control frame carries more than 125 bytes");
        }
    }
    if (payloadLength > maxPayloadLength)
    {
        throw Exception("WebSockets payload is longer than 2^63-1 bytes");
    }
    appendFrameHeader(output, payloadLength, opcode, finalMessage, nullptr);
}

void WSWebSocketsMessage::encode(const uint8_t* payload, size_t length, OpCode opcode, bool finalMessage,
                                 Buffer& output, bool masked)
{
    if (isControlOpcode(opcode))
    {
        if (!finalMessage)
        {
            throw Exception("WebSockets control frame may not be fragmented");
        }
        if (length > maxControlPayloadLength)
        {
            throw Exception("WebSockets control frame carries more than 125 bytes");
        }
    }

    if (!masked)
    {
        appendFrameHeader(output, length, opcode, finalMessage, nullptr);
        output.append(payload, length);
        return;
    }

    const auto mask = randomMask();
    appendFrameHeader(output, length, opcode, finalMessage, mask.data());

    const size_t start = output.bytes();
    output.append(payload, length);
    for (size_t i = 0; i < length; ++i)
    {
        output[start + i] ^= mask[i % maskLength];
    }
}

void WSWebSocketsMessage::encode(const String& payload, OpCode opcode, bool finalMessage, Buffer& output, bool masked)
{
    encode(reinterpret_cast<const uint8_t*>(payload.c_str()), payload.size(), opcode, finalMessage, output, masked);
}

namespace sptk::ws {
String acceptKey(const String& clientKey)
{
    const String combined = clientKey + webSocketsGuid;

    constexpr size_t digestLength = 20;
    Buffer          digest(digestLength);
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(),
         reinterpret_cast<unsigned char*>(digest.data()));

    Buffer encoded;
    Base64::encode(encoded, Buffer(digest.data(), digestLength));
    return {encoded.c_str(), encoded.bytes()};
}

String negotiateSubprotocol(const HttpHeaders& headers, const Strings& supported)
{
    const auto header = headers.find("Sec-WebSocket-Protocol");
    if (header == headers.end())
    {
        return {};
    }

    // The client sends a comma-separated list in order of preference.
    Strings requested;
    for (const auto& token: String(header->second).split(","))
    {
        const auto trimmed = token.trim();
        if (!trimmed.empty())
        {
            requested.push_back(trimmed);
        }
    }

    if (requested.empty())
    {
        return {};
    }

    if (supported.empty())
    {
        // A server with no opinion echoes the client's first choice, which is what a demo does.
        return requested.front();
    }

    for (const auto& offered: requested)
    {
        for (const auto& candidate: supported)
        {
            if (offered == candidate)
            {
                return offered;
            }
        }
    }
    return {};
}

void writeUpgradeResponse(Socket& socket, const String& accept, const String& subprotocol)
{
    socket.write(String("HTTP/1.1 101 Switching Protocols\r\n"));
    socket.write(String("Upgrade: websocket\r\n"));
    socket.write(String("Connection: Upgrade\r\n"));
    socket.write(String("Sec-WebSocket-Accept: ") + accept + "\r\n");
    if (!subprotocol.empty())
    {
        socket.write(String("Sec-WebSocket-Protocol: ") + subprotocol + "\r\n");
    }
    socket.write(String("\r\n"));
}
} // namespace sptk::ws

WSWebSocketsProtocol::WSWebSocketsProtocol(const shared_ptr<TCPSocket>& socket, const HttpHeaders& headers)
    : WSProtocol(socket, headers)
{
}

RequestInfo WSWebSocketsProtocol::process()
{
    RequestInfo requestInfo;
    try
    {
        const String clientKey = header("Sec-WebSocket-Key");
        const String version = header("Sec-WebSocket-Version");
        if (clientKey.empty() || version.trim() != "13")
        {
            throw Exception(
                "WebSocket protocol is missing or has invalid Sec-WebSocket-Key or Sec-WebSocket-Version headers");
        }

        ws::writeUpgradeResponse(socket(), ws::acceptKey(clientKey), ws::negotiateSubprotocol(headers(), {}));
        echoLoop();
    }
    catch (const Exception& e)
    {
        const string text(
            "<html><head><title>Error processing request</title></head><body>" + string(e.message()) +
            "</body></html>\n");
        socket().write("HTTP/1.1 400 Bad Request\n");
        socket().write("Content-Type: text/html; charset=utf-8\n");
        socket().write("Content-length: " + to_string(text.length()) + "\n\n");
        socket().write(text);
        socket().close();
    }

    return requestInfo;
}

void WSWebSocketsProtocol::echoLoop()
{
    constexpr auto pollTimeout = chrono::seconds(30);

    WSWebSocketStream stream;
    Buffer            chunk;

    auto flush = [this, &stream]() {
        while (stream.hasOutput())
        {
            const size_t sent = socket().write(stream.output());
            if (sent == 0)
            {
                break;
            }
            stream.consumeOutput(sent);
        }
    };

    while (!stream.closeReceived())
    {
        if (!socket().readyToRead(pollTimeout))
        {
            continue;
        }

        const size_t available = socket().socketBytes();
        if (available == 0)
        {
            break;
        }

        const size_t received = socket().read(chunk, available);
        if (received == 0)
        {
            break;
        }

        stream.feed(chunk.data(), received);

        // Echo every complete message back. Binary on the way out, whatever the client sent: this
        // is a transport demo, and a client that needs text framing is not what it is for.
        while (stream.available() > 0)
        {
            Buffer message;
            message.reserve(stream.available());
            const size_t length = stream.read(message.data(), stream.available());
            message.bytes(length);
            if (length > 0)
            {
                stream.write(message.data(), message.bytes());
            }
        }

        flush();
    }

    stream.close(1000);
    flush();
}

void WSWebSocketsProtocol::replyCloseConnectionRequest(uint16_t statusCode, const String& closeReason)
{
    constexpr size_t maxReasonLength = WSWebSocketsMessage::maxControlPayloadLength - 2;

    Buffer payload;
    payload.append(static_cast<uint8_t>(statusCode >> 8));
    payload.append(static_cast<uint8_t>(statusCode & 0xFF));
    payload.append(closeReason.c_str(), min(closeReason.size(), maxReasonLength));

    Buffer frame;
    WSWebSocketsMessage::encode(payload.data(), payload.bytes(), WSWebSocketsMessage::OpCode::CONNECTION_CLOSE, true,
                                frame);
    socket().write(frame);
}
