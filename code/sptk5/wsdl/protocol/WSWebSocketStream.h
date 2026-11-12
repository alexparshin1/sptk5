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

#include "WSWebSocketsProtocol.h"

#include <sptk5/Buffer.h>
#include <sptk5/String.h>

#include <cstdint>

namespace sptk {
/// @addtogroup wsdl WSDL-related Classes
/// @{

/**
 * @brief A byte stream carried over WebSockets frames.
 *
 * A protocol that already has a framing of its own - MQTT, for one - does not want the transport's
 * frames to reach it. It wants bytes, and it wants the transport's own concerns handled under it:
 * partial frames, frames split across reads, several frames in one read, fragmentation, ping and
 * pong, and a close that arrives in the middle of a message.
 *
 * This class is that layer. feed() takes whatever the socket read, read() hands back the payload
 * of complete messages and nothing else, write() frames an outgoing message, and the control
 * frames never surface at all - a ping is answered inside the class, a pong is remembered, and a
 * close is echoed once and recorded.
 *
 * Nothing here touches a socket: the caller reads into feed() and writes what output() holds, so
 * the class works over TCP, TLS or a pipe, and is tested without a network.
 *
 * A session ends in one of two ways. A peer asking to close shows up as closeReceived(); a protocol
 * violation queues the matching close frame and throws, after which write() does nothing, so the
 * caller drains output() and closes the connection.
 */
class SP_EXPORT WSWebSocketStream
{
public:
    /// Which end of the connection this is. It decides whether incoming frames must be masked and
    /// whether outgoing ones are, and nothing else.
    enum class Role
    {
        Server,
        Client
    };

    /// @param role             End of the connection.
    /// @param maxMessageSize   Largest message accepted from the peer, in bytes. A message longer
    ///                         than this is refused with close code 1009 rather than assembled.
    explicit WSWebSocketStream(Role role = Role::Server, size_t maxMessageSize = 256 * 1024 * 1024);

    WSWebSocketStream(const WSWebSocketStream&) = delete;
    WSWebSocketStream& operator=(const WSWebSocketStream&) = delete;

    /**
     * @brief Accepts bytes received from the connection and parses every complete frame in them.
     *
     * Bytes of an incomplete frame are kept until the rest arrives. Complete messages are appended
     * to what read() returns.
     *
     * @throws Exception on a protocol violation, after queueing the close frame that reports it.
     */
    void feed(const uint8_t* data, size_t length);

    /// Bytes of complete application messages waiting to be read.
    [[nodiscard]] size_t available() const;

    /**
     * @brief Copies waiting message bytes out.
     * @param buffer            Destination.
     * @param capacity          How much fits.
     * @return Bytes copied, 0 when nothing is waiting.
     */
    size_t read(uint8_t* buffer, size_t capacity);

    /**
     * @brief Frames one binary message for sending.
     *
     * After a close in either direction this does nothing: the connection is finished, and a
     * message written past that point would be lost anyway.
     */
    void write(const uint8_t* data, size_t length);

    /// Sends a ping. Its payload may not exceed 125 bytes.
    void ping(const String& payload = String());

    /// Sends a close frame. The first call wins; a later one does nothing.
    void close(uint16_t code = 1000, const String& reason = String());

    /// True once the peer has asked to close.
    [[nodiscard]] bool closeReceived() const
    {
        return m_closeReceived;
    }

    /// True once a close frame has been sent, by either side.
    [[nodiscard]] bool closeSent() const
    {
        return m_closeSent;
    }

    /// True when the session ended on a protocol violation rather than a clean close.
    [[nodiscard]] bool failed() const
    {
        return m_failed;
    }

    /// Status code of the peer's close frame, or of the one this class sent.
    [[nodiscard]] uint16_t closeCode() const
    {
        return m_closeCode;
    }

    /// Reason text of the peer's close frame, or of the one this class sent.
    [[nodiscard]] const String& closeReason() const
    {
        return m_closeReason;
    }

    /// True when framed bytes are waiting to be written to the connection.
    [[nodiscard]] bool hasOutput() const
    {
        return m_outputOffset < m_output.bytes();
    }

    /// Framed bytes waiting to be written to the connection.
    [[nodiscard]] Buffer& output()
    {
        return m_output;
    }

    /// Removes bytes the caller has sent.
    void consumeOutput(size_t length);

    [[nodiscard]] size_t maxMessageSize() const
    {
        return m_maxMessageSize;
    }

    void maxMessageSize(size_t value)
    {
        m_maxMessageSize = value;
    }

private:
    Role    m_role;
    size_t  m_maxMessageSize;

    Buffer m_input;          ///< Received bytes that do not yet form a complete frame.
    Buffer m_fragment;       ///< Payload of a fragmented message being assembled.
    bool   m_fragmenting {false};
    bool   m_fragmentIsText {false};

    Buffer m_inbox;          ///< Payload of complete messages, waiting to be read.
    size_t m_inboxOffset {0};

    Buffer m_output;         ///< Frames waiting to be written to the connection.
    size_t m_outputOffset {0};

    bool     m_closeReceived {false};
    bool     m_closeSent {false};
    bool     m_failed {false};
    uint16_t m_closeCode {1000};
    String   m_closeReason;

    void handleFrame(const WSWebSocketsMessage& frame);
    void appendToMessage(const Buffer& payload);
    void deliverMessage(const uint8_t* payload, size_t length, bool text);
    void queueFrame(const uint8_t* payload, size_t length, WSWebSocketsMessage::OpCode opcode);
    void compact(Buffer& buffer, size_t consumed);
    [[noreturn]] void fail(uint16_t code, const String& message);
};

/// @}

} // namespace sptk
