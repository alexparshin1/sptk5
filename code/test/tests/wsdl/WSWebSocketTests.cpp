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

#include <gtest/gtest.h>

#include <sptk5/wsdl/protocol/WSWebSocketStream.h>

#include <array>
#include <string>
#include <vector>

using namespace std;
using namespace sptk;

namespace {
using OpCode = WSWebSocketsMessage::OpCode;

/// Builds a client frame by hand, mask included.
///
/// Deliberately not using the encoder under test: a codec checked against itself proves only that
/// it is consistent, not that it follows RFC 6455. Every byte here comes from the specification.
vector<uint8_t> clientFrame(OpCode opcode, bool finalBit, const vector<uint8_t>& payload,
                            const array<uint8_t, 4>& mask = {0x37, 0xFA, 0x21, 0x3D})
{
    vector<uint8_t> frame;
    frame.push_back(static_cast<uint8_t>((finalBit ? 0x80 : 0x00) | static_cast<uint8_t>(opcode)));

    const size_t length = payload.size();
    if (length < 126)
    {
        frame.push_back(static_cast<uint8_t>(0x80 | length));
    }
    else if (length <= 0xFFFF)
    {
        frame.push_back(0x80 | 126);
        frame.push_back(static_cast<uint8_t>(length >> 8));
        frame.push_back(static_cast<uint8_t>(length & 0xFF));
    }
    else
    {
        frame.push_back(0x80 | 127);
        for (int shift = 56; shift >= 0; shift -= 8)
        {
            frame.push_back(static_cast<uint8_t>((length >> shift) & 0xFF));
        }
    }

    for (const auto byte: mask)
    {
        frame.push_back(byte);
    }
    for (size_t i = 0; i < length; ++i)
    {
        frame.push_back(static_cast<uint8_t>(payload[i] ^ mask[i % 4]));
    }
    return frame;
}

vector<uint8_t> clientFrame(OpCode opcode, const string& payload, bool finalBit = true)
{
    return clientFrame(opcode, finalBit, vector<uint8_t>(payload.begin(), payload.end()));
}

string payloadString(const Buffer& payload)
{
    return {payload.c_str(), payload.bytes()};
}

string readAvailable(WSWebSocketStream& stream)
{
    string  data(stream.available(), '\0');
    const size_t read = stream.read(reinterpret_cast<uint8_t*>(data.data()), data.size());
    data.resize(read);
    return data;
}

/// Takes the first queued frame out of a stream's output.
WSWebSocketsMessage popFrame(WSWebSocketStream& stream)
{
    WSWebSocketsMessage frame;
    const size_t consumed = frame.decode(stream.output().data(), stream.output().bytes());
    EXPECT_GT(consumed, 0u);
    stream.consumeOutput(consumed);
    return frame;
}

Buffer frameBuffer(OpCode opcode, const string& payload, bool finalBit = true)
{
    Buffer output;
    WSWebSocketsMessage::encode(reinterpret_cast<const uint8_t*>(payload.c_str()), payload.size(), opcode,
                                finalBit, output);
    return output;
}
} // namespace

//
// The frame codec.
//

TEST(SPTK_WSWebSocketsMessage, roundTripsEveryLengthClass)
{
    const vector<size_t> lengths {0, 1, 125, 126, 127, 65535, 65536, 200000};

    for (const auto length: lengths)
    {
        string payload;
        payload.reserve(length);
        for (size_t i = 0; i < length; ++i)
        {
            payload.push_back(static_cast<char>('a' + i % 26));
        }

        Buffer output;
        WSWebSocketsMessage::encode(reinterpret_cast<const uint8_t*>(payload.c_str()), payload.size(),
                                    OpCode::BINARY, true, output);

        WSWebSocketsMessage decoded;
        const size_t consumed = decoded.decode(output.data(), output.bytes());
        ASSERT_EQ(consumed, output.bytes()) << "length " << length;
        ASSERT_EQ(decoded.opcode(), OpCode::BINARY) << "length " << length;
        ASSERT_TRUE(decoded.isFinal()) << "length " << length;
        ASSERT_FALSE(decoded.isMasked()) << "length " << length;
        ASSERT_EQ(decoded.payload().bytes(), length) << "length " << length;
        ASSERT_EQ(payloadString(decoded.payload()), payload) << "length " << length;
    }
}

TEST(SPTK_WSWebSocketsMessage, usesTheShortestLengthFormThatFits)
{
    // 125 bytes still fits in the seven-bit field, and paying two extra header bytes for nothing
    // would be a violation of the minimal-length rule, not just a waste.
    const auto shortFrame = frameBuffer(OpCode::BINARY, string(125, 'x'));
    ASSERT_EQ(shortFrame.bytes(), 2u + 125u);
    ASSERT_EQ(static_cast<int>(shortFrame[1]), 125);

    const auto mediumFrame = frameBuffer(OpCode::BINARY, string(126, 'x'));
    ASSERT_EQ(mediumFrame.bytes(), 4u + 126u);
    ASSERT_EQ(static_cast<int>(mediumFrame[1]), 126);
    ASSERT_EQ(static_cast<int>(mediumFrame[2]), 0);
    ASSERT_EQ(static_cast<int>(mediumFrame[3]), 126);

    const auto longFrame = frameBuffer(OpCode::BINARY, string(65536, 'x'));
    ASSERT_EQ(longFrame.bytes(), 10u + 65536u);
    ASSERT_EQ(static_cast<int>(longFrame[1]), 127);
    for (size_t i = 2; i < 7; ++i)
    {
        ASSERT_EQ(static_cast<int>(longFrame[i]), 0);
    }
    ASSERT_EQ(static_cast<int>(longFrame[7]), 1); // 65536 is 0x00000000_00010000
    ASSERT_EQ(static_cast<int>(longFrame[8]), 0);
    ASSERT_EQ(static_cast<int>(longFrame[9]), 0);
}

TEST(SPTK_WSWebSocketsMessage, decodesTheSixtyFourBitLengthBigEndian)
{
    // What the previous encoder wrote here was the host's byte order and no length marker at all,
    // which a browser reads as a length of zero followed by garbage. The bytes below are what the
    // specification asks for.
    vector<uint8_t> frame {0x82, 0x7F, 0, 0, 0, 0, 0, 0, 0, 0};
    frame[9] = 3;
    frame.insert(frame.end(), {'a', 'b', 'c'});

    WSWebSocketsMessage decoded;
    ASSERT_EQ(decoded.decode(frame.data(), frame.size()), frame.size());
    ASSERT_EQ(payloadString(decoded.payload()), "abc");
}

TEST(SPTK_WSWebSocketsMessage, takesNoBytesUntilTheFrameIsComplete)
{
    const auto frame = frameBuffer(OpCode::BINARY, string(200, 'z'));

    for (size_t available = 0; available < frame.bytes(); ++available)
    {
        WSWebSocketsMessage decoded;
        ASSERT_EQ(decoded.decode(frame.data(), available), 0u) << "available " << available;
    }

    WSWebSocketsMessage decoded;
    ASSERT_EQ(decoded.decode(frame.data(), frame.bytes()), frame.bytes());
    ASSERT_EQ(decoded.payload().bytes(), 200u);
}

TEST(SPTK_WSWebSocketsMessage, stopsAtTheEndOfTheFrameItDecoded)
{
    const auto first = frameBuffer(OpCode::BINARY, "first");
    Buffer     joined;
    joined.append(first);
    joined.append(frameBuffer(OpCode::BINARY, "second"));

    WSWebSocketsMessage decoded;
    ASSERT_EQ(decoded.decode(joined.data(), joined.bytes()), first.bytes());
    ASSERT_EQ(payloadString(decoded.payload()), "first");

    WSWebSocketsMessage second;
    ASSERT_EQ(second.decode(joined.data() + first.bytes(), joined.bytes() - first.bytes()),
              joined.bytes() - first.bytes());
    ASSERT_EQ(payloadString(second.payload()), "second");
}

TEST(SPTK_WSWebSocketsMessage, unmasksAClientFrame)
{
    const auto frame = clientFrame(OpCode::BINARY, "the quick brown fox");

    WSWebSocketsMessage decoded;
    ASSERT_EQ(decoded.decode(frame.data(), frame.size()), frame.size());
    ASSERT_TRUE(decoded.isMasked());
    ASSERT_EQ(payloadString(decoded.payload()), "the quick brown fox");
}

TEST(SPTK_WSWebSocketsMessage, separatesTheCloseCodeFromTheReason)
{
    vector<uint8_t> payload {0x03, 0xE9}; // 1001 - going away
    for (const char c: string("maintenance"))
    {
        payload.push_back(static_cast<uint8_t>(c));
    }

    const auto frame = clientFrame(OpCode::CONNECTION_CLOSE, true, payload);

    WSWebSocketsMessage decoded;
    ASSERT_EQ(decoded.decode(frame.data(), frame.size()), frame.size());
    ASSERT_EQ(decoded.opcode(), OpCode::CONNECTION_CLOSE);
    ASSERT_EQ(decoded.statusCode(), 1001);
    ASSERT_EQ(payloadString(decoded.payload()), "maintenance");
}

TEST(SPTK_WSWebSocketsMessage, rejectsMalformedFrames)
{
    const auto rejects = [](const vector<uint8_t>& bytes) {
        WSWebSocketsMessage decoded;
        EXPECT_THROW(decoded.decode(bytes.data(), bytes.size()), Exception);
    };

    // A reserved bit set without an extension that negotiated it.
    rejects({0xC2, 0x00});

    // Opcodes 3-7 and 0xB-0xF are reserved and have no meaning.
    rejects({0x83, 0x00});
    rejects({0x8B, 0x00});

    // A control frame may not be fragmented...
    rejects({0x09, 0x00});

    // ...nor carry more than 125 bytes.
    rejects(clientFrame(OpCode::PING, string(126, 'x')));

    // A close frame whose payload is a single byte cannot hold a status code.
    rejects({0x88, 0x01, 0x00});

    // A 64-bit length with the high bit set is explicitly forbidden.
    rejects({0x82, 0x7F, 0x80, 0, 0, 0, 0, 0, 0, 0});
}

TEST(SPTK_WSWebSocketsMessage, encodesAFrameThatCarriesItsOwnMask)
{
    Buffer output;
    WSWebSocketsMessage::encode(reinterpret_cast<const uint8_t*>("masked payload"), 14, OpCode::BINARY, true,
                                output, true);
    ASSERT_EQ(static_cast<int>(output[1]) & 0x80, 0x80);

    WSWebSocketsMessage decoded;
    ASSERT_EQ(decoded.decode(output.data(), output.bytes()), output.bytes());
    ASSERT_TRUE(decoded.isMasked());
    ASSERT_EQ(payloadString(decoded.payload()), "masked payload");
}

//
// The upgrade handshake.
//

TEST(SPTK_WebSocketsHandshake, producesTheAcceptValueFromTheSpecification)
{
    // RFC 6455, section 1.3.
    ASSERT_EQ(ws::acceptKey("dGhlIHNhbXBsZSBub25jZQ=="), "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

TEST(SPTK_WebSocketsHandshake, negotiatesTheFirstCommonSubprotocol)
{
    HttpHeaders headers;
    headers["Sec-WebSocket-Protocol"] = "mqtt, mqttv3.1";

    ASSERT_EQ(ws::negotiateSubprotocol(headers, {"mqttv3.1", "mqtt"}), "mqtt");
    ASSERT_EQ(ws::negotiateSubprotocol(headers, {}), "mqtt");
    ASSERT_TRUE(ws::negotiateSubprotocol(headers, {"something-else"}).empty());
}

TEST(SPTK_WebSocketsHandshake, buildsTheUpgradeReplyWithoutAProtocolWhenNoneWasAgreed)
{
    const auto reply = ws::upgradeResponse(ws::acceptKey("dGhlIHNhbXBsZSBub25jZQ=="), {});

    ASSERT_TRUE(reply.starts_with("HTTP/1.1 101 Switching Protocols\r\n"));
    ASSERT_NE(reply.find("Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n"), std::string::npos);
    // No subprotocol agreed means no header at all, not an empty one: a client reads an empty
    // header as a protocol of its own choosing and then refuses every frame it receives.
    ASSERT_EQ(reply.find("Sec-WebSocket-Protocol"), std::string::npos);
    ASSERT_TRUE(reply.ends_with("\r\n\r\n"));
}

TEST(SPTK_WebSocketsHandshake, buildsTheUpgradeReplyWithTheAgreedProtocol)
{
    const auto reply = ws::upgradeResponse("accept", "mqtt");

    ASSERT_NE(reply.find("Sec-WebSocket-Protocol: mqtt\r\n"), std::string::npos);
    ASSERT_TRUE(reply.ends_with("\r\n\r\n"));
}

//
// The byte stream over frames.
//

TEST(SPTK_WSWebSocketStream, takesAFrameSplitAcrossReads)
{
    const auto frame = clientFrame(OpCode::BINARY, "split across reads");

    WSWebSocketStream stream;
    for (size_t i = 0; i < frame.size(); ++i)
    {
        stream.feed(frame.data() + i, 1);
        if (i + 1 < frame.size())
        {
            ASSERT_EQ(stream.available(), 0u) << "after byte " << i;
        }
    }

    ASSERT_EQ(readAvailable(stream), "split across reads");
}

TEST(SPTK_WSWebSocketStream, takesTwoFramesInOneRead)
{
    const auto first = clientFrame(OpCode::BINARY, "one");
    const auto second = clientFrame(OpCode::BINARY, "two");

    vector<uint8_t> joined(first);
    joined.insert(joined.end(), second.begin(), second.end());

    WSWebSocketStream stream;
    stream.feed(joined.data(), joined.size());
    ASSERT_EQ(readAvailable(stream), "onetwo");
}

TEST(SPTK_WSWebSocketStream, answersAPingWithTheSamePayload)
{
    const auto ping = clientFrame(OpCode::PING, "keepalive");

    WSWebSocketStream stream;
    stream.feed(ping.data(), ping.size());

    ASSERT_EQ(stream.available(), 0u);
    const auto pong = popFrame(stream);
    ASSERT_EQ(pong.opcode(), OpCode::PONG);
    ASSERT_FALSE(pong.isMasked());
    ASSERT_EQ(payloadString(pong.payload()), "keepalive");
    ASSERT_FALSE(stream.hasOutput());
}

TEST(SPTK_WSWebSocketStream, assemblesAFragmentedMessageAroundAControlFrame)
{
    WSWebSocketStream stream;

    const auto first = clientFrame(OpCode::BINARY, "frag", false);
    const auto ping = clientFrame(OpCode::PING, "p");
    const auto last = clientFrame(OpCode::CONTINUATION, "mented");

    vector<uint8_t> joined(first);
    joined.insert(joined.end(), ping.begin(), ping.end());
    joined.insert(joined.end(), last.begin(), last.end());

    stream.feed(joined.data(), joined.size());

    // The ping is answered even though it arrived in the middle of a message, and the message
    // itself comes out whole.
    ASSERT_EQ(readAvailable(stream), "fragmented");
    const auto pong = popFrame(stream);
    ASSERT_EQ(pong.opcode(), OpCode::PONG);
}

TEST(SPTK_WSWebSocketStream, echoesTheCloseAndKeepsThePeersReason)
{
    vector<uint8_t> payload {0x03, 0xE9}; // 1001
    for (const char c: string("maintenance"))
    {
        payload.push_back(static_cast<uint8_t>(c));
    }
    const auto closeFrame = clientFrame(OpCode::CONNECTION_CLOSE, true, payload);

    WSWebSocketStream stream;
    stream.feed(closeFrame.data(), closeFrame.size());

    ASSERT_TRUE(stream.closeReceived());
    ASSERT_TRUE(stream.closeSent());
    ASSERT_EQ(stream.closeCode(), 1001u);
    ASSERT_EQ(stream.closeReason(), "maintenance");

    const auto reply = popFrame(stream);
    ASSERT_EQ(reply.opcode(), OpCode::CONNECTION_CLOSE);
    ASSERT_EQ(reply.statusCode(), 1001);
    ASSERT_TRUE(payloadString(reply.payload()).empty());
}

TEST(SPTK_WSWebSocketStream, refusesAnUnmaskedFrameFromAClient)
{
    Buffer output;
    WSWebSocketsMessage::encode(reinterpret_cast<const uint8_t*>("unmasked"), 8, OpCode::BINARY, true, output);

    WSWebSocketStream stream;
    EXPECT_THROW(stream.feed(output.data(), output.bytes()), Exception);
    ASSERT_TRUE(stream.failed());

    const auto close = popFrame(stream);
    ASSERT_EQ(close.opcode(), OpCode::CONNECTION_CLOSE);
    ASSERT_EQ(close.statusCode(), 1002);
}

TEST(SPTK_WSWebSocketStream, refusesAContinuationWithoutAMessage)
{
    const auto frame = clientFrame(OpCode::CONTINUATION, "orphan");

    WSWebSocketStream stream;
    EXPECT_THROW(stream.feed(frame.data(), frame.size()), Exception);
    ASSERT_EQ(popFrame(stream).statusCode(), 1002);
}

TEST(SPTK_WSWebSocketStream, refusesANewMessageInsideAFragmentedOne)
{
    const auto start = clientFrame(OpCode::BINARY, "half", false);
    const auto other = clientFrame(OpCode::BINARY, "whole");

    WSWebSocketStream stream;
    stream.feed(start.data(), start.size());
    EXPECT_THROW(stream.feed(other.data(), other.size()), Exception);
    ASSERT_EQ(popFrame(stream).statusCode(), 1002);
}

TEST(SPTK_WSWebSocketStream, refusesAMessageBeyondTheConfiguredLimit)
{
    WSWebSocketStream stream(WSWebSocketStream::Role::Server, 10);
    const auto        frame = clientFrame(OpCode::BINARY, "more than ten bytes");

    EXPECT_THROW(stream.feed(frame.data(), frame.size()), Exception);
    ASSERT_EQ(popFrame(stream).statusCode(), 1009);
}

TEST(SPTK_WSWebSocketStream, refusesAnOversizedFrameAsSoonAsItsHeaderArrives)
{
    // The header names a payload of 100000 bytes; only the header is sent. Nothing should be
    // buffered guessing that the payload will arrive.
    const auto frame = clientFrame(OpCode::BINARY, string(100000, 'x'));

    WSWebSocketStream stream(WSWebSocketStream::Role::Server, 1000);
    const vector<uint8_t> headerOnly(frame.begin(), frame.begin() + 10);
    EXPECT_THROW(stream.feed(headerOnly.data(), headerOnly.size()), Exception);
    ASSERT_EQ(popFrame(stream).statusCode(), 1009);
}

TEST(SPTK_WSWebSocketStream, refusesTextThatIsNotUtf8)
{
    const vector<uint8_t> invalid {0xC3, 0x28}; // a two-byte lead followed by a non-continuation
    const auto           frame = clientFrame(OpCode::TEXT, true, invalid);

    WSWebSocketStream stream;
    EXPECT_THROW(stream.feed(frame.data(), frame.size()), Exception);
    ASSERT_EQ(popFrame(stream).statusCode(), 1007);
}

TEST(SPTK_WSWebSocketStream, acceptsValidUtf8Text)
{
    // Cyrillic and an emoji: two-, three- and four-byte sequences.
    const string text = "Привет, мир! \xF0\x9F\x98\x80";
    const auto   frame = clientFrame(OpCode::TEXT, text);

    WSWebSocketStream stream;
    stream.feed(frame.data(), frame.size());
    ASSERT_EQ(readAvailable(stream), text);
}

TEST(SPTK_WSWebSocketStream, aClientMasksWhatItWrites)
{
    WSWebSocketStream server;
    WSWebSocketStream client(WSWebSocketStream::Role::Client);

    const string message = "from the client";
    client.write(reinterpret_cast<const uint8_t*>(message.c_str()), message.size());
    ASSERT_TRUE(client.hasOutput());

    server.feed(client.output().data(), client.output().bytes());
    ASSERT_EQ(readAvailable(server), message);
}

TEST(SPTK_WSWebSocketStream, roundTripsAMessageLargerThanOneSixtyFourKilobytes)
{
    string message(200000, '\0');
    for (size_t i = 0; i < message.size(); ++i)
    {
        message[i] = static_cast<char>(i % 251);
    }

    WSWebSocketStream server;
    WSWebSocketStream client(WSWebSocketStream::Role::Client);

    client.write(reinterpret_cast<const uint8_t*>(message.c_str()), message.size());
    server.feed(client.output().data(), client.output().bytes());

    ASSERT_EQ(readAvailable(server), message);
}

TEST(SPTK_WSWebSocketStream, writesNothingAfterAClose)
{
    WSWebSocketStream stream;
    stream.close(1000, "done");
    const size_t queued = stream.output().bytes();

    stream.write(reinterpret_cast<const uint8_t*>("lost"), 4);
    ASSERT_EQ(stream.output().bytes(), queued);
}

TEST(SPTK_WSWebSocketStream, aClientAnswersWithAMaskedClose)
{
    WSWebSocketStream client(WSWebSocketStream::Role::Client);
    client.close(1000, "bye");

    WSWebSocketsMessage decoded;
    ASSERT_EQ(decoded.decode(client.output().data(), client.output().bytes()), client.output().bytes());
    ASSERT_TRUE(decoded.isMasked());
    ASSERT_EQ(decoded.opcode(), OpCode::CONNECTION_CLOSE);
    ASSERT_EQ(decoded.statusCode(), 1000);
    ASSERT_EQ(payloadString(decoded.payload()), "bye");
}
