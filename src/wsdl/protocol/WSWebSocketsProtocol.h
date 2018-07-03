/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSWebSocketsProtocol.h - description                   ║
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

#ifndef __WSWEBSOCKETSPROTOCOL_H__
#define __WSWEBSOCKETSPROTOCOL_H__

#include "WSProtocol.h"
#include <sptk5/cnet>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief WebSockets protocol message
///
/// Provides WebSockets message encode and decode methods
class WSWebSocketsMessage
{
    uint32_t    m_opcode;       ///< WebSockets operation code
    Buffer      m_payload;      ///< Message payload
    bool        m_finalMessage; ///< 'message is final' flag

public:
    enum OpCode {
        OC_CONTINUATION     = 0,
        OC_TEXT             = 1,
        OC_BINARY           = 2,
        OC_CONNECTION_CLOSE = 8,
        OC_PING1            = 9,
        OC_PING2            = 10
    };

public:
    /// @brief Default constructor
    WSWebSocketsMessage();

    /// @brief Return message payload buffer
    const Buffer& payload();

    /// @brief Decode incoming data into message payload
    /// @param incomingData const char*, Incoming data received from WebSockets
    void decode(const char* incomingData);

    /// @brief Encode a payload into WebSockets frame
    /// @param payload String, Message to encode
    /// @param opcode OpCode, WebSockets operation code
    /// @param final bool, 'message is final' flag
    /// @param output Buffer, WebSockets frame, ready to send
    static void encode(String payload, OpCode opcode, bool final, Buffer& output);
};

/// @brief WebSockets connection handler
///
/// Treats connection as WebSockets, implementing WebSockets
/// handshake and client session. Session stays connected until
/// client disconnects.
class WSWebSocketsProtocol : public WSProtocol
{
public:
    /// @brief Constructor
    /// @param socket TCPSocket*, Connection socket
    /// @param headers const std::map<String,String>&, Connection HTTP headers
    WSWebSocketsProtocol(TCPSocket* socket, const HttpHeaders& headers);

    /// @brief Process method
    ///
    /// Implements WebSockets session
    void process() override;
};

class WSNotification
{
    const std::map<String,String>   m_headers;
    String                          m_data;
};

class WSNotificationManager
{
    Strings  m_queues;
public:
    WSNotificationManager() = default;
    //bool receive(Buffer& message);
    //bool send(const std::map<String,String>& headers, String data);
};

} // namespace sptk

#endif
