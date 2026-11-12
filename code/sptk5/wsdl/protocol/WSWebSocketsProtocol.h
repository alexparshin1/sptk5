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

#pragma once

#include "WSProtocol.h"
#include <sptk5/Buffer.h>
#include <sptk5/String.h>
#include <sptk5/cnet>

#include <cstdint>

namespace sptk {
/// @addtogroup wsdl WSDL-related Classes
/// @{

/**
 * @brief One WebSockets frame, RFC 6455.
 *
 * The class is a codec, not a session: it parses or produces a single frame and knows nothing
 * about the connection it travels on. A session that needs a byte stream over frames uses
 * WSWebSocketStream, which builds on this.
 *
 * Frames are produced without a mask, which is what a server must send. A masked frame is
 * produced only when the caller asks for one, which is what a client must send; the mask is
 * generated here, because a mask reused across frames defeats its purpose.
 */
class SP_EXPORT WSWebSocketsMessage
{
public:
    enum class OpCode : uint8_t
    {
        CONTINUATION = 0x0,
        TEXT = 0x1,
        BINARY = 0x2,
        CONNECTION_CLOSE = 0x8,
        PING = 0x9,
        PONG = 0xA
    };

    /// A control frame - close, ping, pong - may not carry more than this, and may not be fragmented.
    static constexpr size_t maxControlPayloadLength = 125;

    /// RFC 6455 forbids a 64-bit payload length with the most significant bit set.
    static constexpr uint64_t maxPayloadLength = (uint64_t(1) << 63) - 1;

    WSWebSocketsMessage() = default;

    /// Payload of the frame. For a close frame the two-byte status code is removed and kept in
    /// statusCode(), so the payload is the reason text alone.
    [[nodiscard]] const Buffer& payload() const
    {
        return m_payload;
    }

    [[nodiscard]] OpCode opcode() const
    {
        return m_opcode;
    }

    void opcode(OpCode code)
    {
        m_opcode = code;
    }

    /// Close status code, meaningful only for a CONNECTION_CLOSE frame that carried one.
    [[nodiscard]] uint16_t statusCode() const
    {
        return m_status;
    }

    /// True when the frame was marked final. A frame with this clear starts or continues a
    /// fragmented message.
    [[nodiscard]] bool isFinal() const
    {
        return m_finalMessage;
    }

    /// True when the frame arrived masked, which every client-to-server frame must be.
    [[nodiscard]] bool isMasked() const
    {
        return m_masked;
    }

    /// The payload length the frame's header declared. Known as soon as the header has arrived,
    /// even while the payload itself is still on its way, which is what lets a caller refuse an
    /// oversized frame before buffering it.
    [[nodiscard]] uint64_t declaredPayloadLength() const
    {
        return m_declaredPayloadLength;
    }

    /**
     * @brief Decodes the first frame in a buffer.
     *
     * Decoding is incremental: a frame that has not fully arrived is not an error, and the
     * caller keeps the bytes and tries again after the next read. Nothing past the frame is
     * inspected, so a buffer holding several frames is decoded one call at a time.
     *
     * @param data              Bytes received from the connection.
     * @param available         How many of them are valid.
     * @return Bytes consumed, or 0 when the frame is not complete yet.
     * @throws Exception when the bytes cannot be a valid frame: a reserved bit set, an unknown
     *                   opcode, a fragmented or oversized control frame, a 64-bit length with its
     *                   high bit set, or a close frame carrying exactly one byte.
     */
    size_t decode(const uint8_t* data, size_t available);

    /**
     * @brief Appends a frame header for a payload of the given length.
     *
     * Separated from encode() so that a large payload - a broker's PUBLISH, say - is not copied
     * into an intermediate buffer just to be framed.
     */
    static void encodeHeader(uint64_t payloadLength, OpCode opcode, bool finalMessage, Buffer& output);

    /**
     * @brief Appends a complete frame to the output buffer.
     * @param payload           Payload bytes.
     * @param length            Payload length.
     * @param opcode            Frame opcode.
     * @param finalMessage      'message is final' flag.
     * @param output            Buffer the frame is appended to.
     * @param masked            Produce a masked frame, as a client must.
     */
    static void encode(const uint8_t* payload, size_t length, OpCode opcode, bool finalMessage, Buffer& output,
                       bool masked = false);

    /// @brief Appends a complete frame to the output buffer, from a string payload.
    static void encode(const String& payload, OpCode opcode, bool finalMessage, Buffer& output, bool masked = false);

private:
    OpCode   m_opcode {OpCode::CONTINUATION};
    uint16_t m_status {0};
    Buffer   m_payload;
    bool     m_finalMessage {true};
    bool     m_masked {false};
    uint64_t m_declaredPayloadLength {0};
};

/**
 * @brief The HTTP part of a WebSockets connection: the request that asks for one, and the reply
 *        that switches protocols.
 *
 * It is kept apart from the session classes because a server that runs its own connection model
 * still has to send exactly this reply, and because the accept key is the one thing a WebSockets
 * implementation is most often wrong about.
 */
namespace ws {
/**
 * @brief The Sec-WebSocket-Accept value for a client's Sec-WebSocket-Key.
 * @param clientKey         The Sec-WebSocket-Key header the client sent.
 */
SP_EXPORT String acceptKey(const String& clientKey);

/**
 * @brief Picks the subprotocol to answer with.
 *
 * @param headers           Request headers.
 * @param supported         Subprotocols the server speaks, in order of preference. Empty means
 *                          "echo whatever the client asked for", which is what a demo server does.
 * @return The chosen subprotocol, or an empty string when they have none in common.
 */
SP_EXPORT String negotiateSubprotocol(const HttpHeaders& headers, const Strings& supported);

/**
 * @brief Writes the 101 Switching Protocols reply.
 * @param socket            Connection socket.
 * @param accept            Sec-WebSocket-Accept value.
 * @param subprotocol       Subprotocol to confirm, or empty for none.
 */
SP_EXPORT void writeUpgradeResponse(Socket& socket, const String& accept, const String& subprotocol);
} // namespace ws

/// WebSockets connection handler
///
/// Runs a WebSockets session for the WSDL server: it answers the upgrade request and then echoes
/// every message back, as binary, until the client closes. It exists to prove the transport and to
/// serve the WSDL "WS" protocol path; a server with a protocol of its own - an MQTT broker, say -
/// uses ws::acceptKey() and WSWebSocketStream directly, because it owns the connection loop.
class SP_EXPORT WSWebSocketsProtocol : public WSProtocol
{
public:
    /// @brief Constructor.
    /// @param socket TCPSocket*, Connection socket.
    /// @param headers HTTP headers.
    WSWebSocketsProtocol(const std::shared_ptr<TCPSocket>& socket, const HttpHeaders& headers);

    /// @brief Process method.
    /// Implements WebSockets session.
    RequestInfo process() override;

    /**
     * @brief Sends a close frame.
     * @param statusCode        Close status code.
     * @param closeReason       Reason text, truncated to what a control frame can carry.
     */
    void replyCloseConnectionRequest(uint16_t statusCode, const String& closeReason);

private:
    void echoLoop();
};

/// @}

} // namespace sptk
