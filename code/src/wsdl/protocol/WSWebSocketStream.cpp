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

#include "sptk5/wsdl/protocol/WSWebSocketStream.h"

#include <cstring>

using namespace std;
using namespace sptk;

namespace {
constexpr uint16_t closeNormal = 1000;
constexpr uint16_t closeProtocolError = 1002;
constexpr uint16_t closeInvalidUTF8 = 1007;
constexpr uint16_t closeMessageTooBig = 1009;

/// A close frame carries the status code in its first two bytes; the reason may not push the
/// payload past the control-frame limit.
constexpr size_t maxCloseReasonLength = WSWebSocketsMessage::maxControlPayloadLength - 2;

/**
 * @brief Reports whether the bytes are a well-formed UTF-8 sequence.
 *
 * RFC 6455 requires a text message to be valid UTF-8, and a browser will not send one that is not.
 * Checking it is cheap and turns a garbled message into the close code the RFC prescribes - 1007 -
 * instead of passing bytes to a protocol that has no use for them.
 */
bool isValidUtf8(const uint8_t* data, size_t length)
{
    size_t i = 0;
    while (i < length)
    {
        const uint8_t first = data[i];
        if (first < 0x80)
        {
            ++i;
            continue;
        }

        size_t   continuation = 0;
        uint32_t codePoint = 0;
        uint32_t minimum = 0;
        if ((first & 0xE0) == 0xC0)
        {
            continuation = 1;
            codePoint = first & 0x1F;
            minimum = 0x80;
        }
        else if ((first & 0xF0) == 0xE0)
        {
            continuation = 2;
            codePoint = first & 0x0F;
            minimum = 0x800;
        }
        else if ((first & 0xF8) == 0xF0)
        {
            continuation = 3;
            codePoint = first & 0x07;
            minimum = 0x10000;
        }
        else
        {
            // A continuation byte where a lead byte belongs, or one of the five- and six-byte
            // forms that UTF-8 dropped.
            return false;
        }

        if (i + continuation >= length)
        {
            return false;
        }

        for (size_t j = 1; j <= continuation; ++j)
        {
            const uint8_t byte = data[i + j];
            if ((byte & 0xC0) != 0x80)
            {
                return false;
            }
            codePoint = (codePoint << 6) | (byte & 0x3F);
        }

        // Overlong forms, surrogates and code points past the last plane are all invalid.
        if (codePoint < minimum || (codePoint >= 0xD800 && codePoint <= 0xDFFF) || codePoint > 0x10FFFF)
        {
            return false;
        }

        i += continuation + 1;
    }
    return true;
}
} // namespace

WSWebSocketStream::WSWebSocketStream(Role role, size_t maxMessageSize)
    : m_role(role)
    , m_maxMessageSize(maxMessageSize)
{
}

void WSWebSocketStream::feed(const uint8_t* data, size_t length)
{
    if (data == nullptr || length == 0)
    {
        return;
    }

    m_input.append(data, length);

    size_t consumed = 0;
    try
    {
        while (consumed < m_input.bytes())
        {
            WSWebSocketsMessage frame;
            const size_t frameLength = frame.decode(m_input.data() + consumed, m_input.bytes() - consumed);
            if (frameLength == 0)
            {
                // The header may already have named a payload longer than this session accepts.
                // Refusing it here is what keeps a bad length from making the broker buffer the
                // bytes that would prove it.
                if (frame.declaredPayloadLength() > m_maxMessageSize)
                {
                    fail(closeMessageTooBig, "WebSockets frame declares a payload larger than the configured limit");
                }
                break;
            }
            consumed += frameLength;

            if (m_closeReceived || m_failed)
            {
                // Everything after a close is noise: the peer may have sent a message before its
                // close frame arrived, and parsing it would deliver data to a session that is over.
                continue;
            }
            handleFrame(frame);
        }
    }
    catch (...)
    {
        compact(m_input, consumed);
        throw;
    }

    compact(m_input, consumed);
}

void WSWebSocketStream::handleFrame(const WSWebSocketsMessage& frame)
{
    using OpCode = WSWebSocketsMessage::OpCode;

    const bool mustBeMasked = m_role == Role::Server;
    if (frame.isMasked() != mustBeMasked)
    {
        fail(closeProtocolError, mustBeMasked
                                      ? "WebSockets frame from a client is not masked"
                                      : "WebSockets frame from a server is masked");
    }

    switch (frame.opcode())
    {
        case OpCode::PING:
            queueFrame(frame.payload().data(), frame.payload().bytes(), OpCode::PONG);
            return;

        case OpCode::PONG:
            return;

        case OpCode::CONNECTION_CLOSE:
            // Record the peer's side first, so that the echo below cannot overwrite it: what the
            // caller asks afterwards is why the peer closed, not what this side answered.
            m_closeReceived = true;
            m_fragmenting = false;
            m_fragment.reset();
            m_closeCode = frame.statusCode() != 0 ? frame.statusCode() : closeNormal;
            m_closeReason = String(frame.payload().c_str(), frame.payload().bytes());
            // Echo the code the peer sent, but not its reason: the connection is closing and
            // repeating the text tells the peer nothing it does not know.
            close(m_closeCode, String());
            return;

        case OpCode::CONTINUATION:
            if (!m_fragmenting)
            {
                fail(closeProtocolError, "WebSockets continuation frame without a message to continue");
            }
            appendToMessage(frame.payload());
            if (frame.isFinal())
            {
                m_fragmenting = false;
                deliverMessage(m_fragment.data(), m_fragment.bytes(), m_fragmentIsText);
                m_fragment.reset();
            }
            return;

        case OpCode::TEXT:
        case OpCode::BINARY:
            if (m_fragmenting)
            {
                fail(closeProtocolError, "WebSockets data frame in the middle of a fragmented message");
            }
            if (frame.isFinal())
            {
                deliverMessage(frame.payload().data(), frame.payload().bytes(), frame.opcode() == OpCode::TEXT);
            }
            else
            {
                m_fragmenting = true;
                m_fragmentIsText = frame.opcode() == OpCode::TEXT;
                m_fragment.reset();
                appendToMessage(frame.payload());
            }
            return;
    }
}

void WSWebSocketStream::appendToMessage(const Buffer& payload)
{
    if (m_fragment.bytes() + payload.bytes() > m_maxMessageSize)
    {
        fail(closeMessageTooBig, "WebSockets message is larger than the configured limit");
    }
    m_fragment.append(payload);
}

void WSWebSocketStream::deliverMessage(const uint8_t* payload, size_t length, bool text)
{
    if (length > m_maxMessageSize)
    {
        fail(closeMessageTooBig, "WebSockets message is larger than the configured limit");
    }
    if (text && !isValidUtf8(payload, length))
    {
        fail(closeInvalidUTF8, "WebSockets text message is not valid UTF-8");
    }
    m_inbox.append(payload, length);
}

void WSWebSocketStream::queueFrame(const uint8_t* payload, size_t length, WSWebSocketsMessage::OpCode opcode)
{
    WSWebSocketsMessage::encode(payload, length, opcode, true, m_output, m_role == Role::Client);
}

void WSWebSocketStream::write(const uint8_t* data, size_t length)
{
    if (m_closeSent || m_closeReceived || m_failed)
    {
        return;
    }
    queueFrame(data, length, WSWebSocketsMessage::OpCode::BINARY);
}

void WSWebSocketStream::ping(const String& payload)
{
    if (payload.size() > WSWebSocketsMessage::maxControlPayloadLength)
    {
        throw Exception("A WebSockets ping may not carry more than 125 bytes");
    }
    if (m_closeSent || m_closeReceived || m_failed)
    {
        return;
    }
    queueFrame(reinterpret_cast<const uint8_t*>(payload.c_str()), payload.size(), WSWebSocketsMessage::OpCode::PING);
}

void WSWebSocketStream::close(uint16_t code, const String& reason)
{
    if (m_closeSent || m_failed)
    {
        return;
    }

    const size_t reasonLength = min(reason.size(), maxCloseReasonLength);
    Buffer       payload;
    payload.append(static_cast<uint8_t>(code >> 8));
    payload.append(static_cast<uint8_t>(code & 0xFF));
    payload.append(reason.c_str(), reasonLength);

    WSWebSocketsMessage::encode(payload.data(), payload.bytes(), WSWebSocketsMessage::OpCode::CONNECTION_CLOSE,
                                true, m_output, m_role == Role::Client);
    m_closeSent = true;
    if (!m_closeReceived)
    {
        m_closeCode = code;
        m_closeReason = reason.substr(0, reasonLength);
    }
}

size_t WSWebSocketStream::available() const
{
    return m_inbox.bytes() - m_inboxOffset;
}

size_t WSWebSocketStream::read(uint8_t* buffer, size_t capacity)
{
    const size_t waiting = available();
    if (waiting == 0 || capacity == 0)
    {
        return 0;
    }

    const size_t length = min(waiting, capacity);
    memcpy(buffer, m_inbox.data() + m_inboxOffset, length);
    m_inboxOffset += length;

    if (m_inboxOffset == m_inbox.bytes())
    {
        m_inbox.reset();
        m_inboxOffset = 0;
    }
    else if (m_inboxOffset > m_inbox.bytes() / 2)
    {
        compact(m_inbox, m_inboxOffset);
        m_inboxOffset = 0;
    }

    return length;
}

void WSWebSocketStream::consumeOutput(size_t length)
{
    m_outputOffset += min(length, m_output.bytes() - m_outputOffset);
    if (m_outputOffset > 0)
    {
        // Always compacted, not compacted lazily as the read side is: output() hands out the whole
        // buffer, and a caller that cannot see the offset would resend bytes already written.
        compact(m_output, m_outputOffset);
        m_outputOffset = 0;
    }
}

void WSWebSocketStream::compact(Buffer& buffer, size_t consumed)
{
    if (consumed == 0)
    {
        return;
    }
    const size_t remaining = buffer.bytes() - consumed;
    if (remaining > 0)
    {
        memmove(buffer.data(), buffer.data() + consumed, remaining);
    }
    buffer.bytes(remaining);
}

void WSWebSocketStream::fail(uint16_t code, const String& message)
{
    if (!m_failed)
    {
        // The close frame goes out before the flag does, because close() refuses to work on a
        // session already marked failed.
        close(code, message);
        m_failed = true;
    }
    throw Exception(message);
}
