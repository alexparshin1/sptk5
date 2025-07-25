/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQClient.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SMQ_CLIENT_H__
#define __SMQ_CLIENT_H__

#include <sptk5/cthreads>
#include <sptk5/cnet>
#include <smq/protocols/SMQProtocol.h>
#include <smq/clients/TCPMQClient.h>
#include <smq/clients/BaseMQClient.h>
#include <smq/protocols/MQLastWillMessage.h>

namespace sptk {

class SP_EXPORT SMQClient : public TCPMQClient
{
    mutable SharedMutex                     m_mutex;            ///< Mutex that protects internal data
    Host                                    m_server;           ///< SMQ server host
    String                                  m_username;         ///< Connection user name
    String                                  m_password;         ///< Connection password
    std::unique_ptr<MQLastWillMessage>      m_lastWillMessage;  ///< Optional last will message
protected:
    void socketEvent(SocketEventType eventType) override;

public:

    /**
     * Constructor
     * @param clientId          Unique client id
     */
    SMQClient(MQProtocolType protocolType, const String& clientId);

    /**
     * Destructor
     */
    ~SMQClient() override = default;

    /**
     * Set last will message
     *
     * Last will message is published when client abnormally terminates connection to server.
     * This message only has effect on the next connect operation(s).
     * @param lastWillMessage   Last will message
     */
    void setLastWillMessage(std::unique_ptr<MQLastWillMessage>& lastWillMessage);

    /*
     * Connect to MQ server
     * @param server            MQ server host and port
     * @param user              MQ server username.
     * @param password          MQ server password.
     * @param encrypted         Use encrypted connection. If true, then SSL keys must be loaded prior to this call.
     * @param timeout           Operation timeout
     */
    void connect(const Host& server, const String& username, const String& password, bool encrypted,
                 std::chrono::milliseconds timeout) override;

    /**
     * Disconnect from server
     * @param immediate         If false then disconnect as defined by server protocol. Otherwise, just terminate connection.
     */
    void disconnect(bool immediate) override;

    /**
     * Subscribe to a queue or topic
     * @param destination       Destination queue or topic name
     */
    void subscribe(const String& destination, std::chrono::milliseconds timeout) override;

    /**
     * Un-subscribe from a queue or topic
     * @param destination       Destination queue or topic name
     */
    void unsubscribe(const String& destination, std::chrono::milliseconds timeout) override;

    /**
     * Send message
     * @param destination       Queue or topic name
     * @param message           Message
     * @param timeout           Operation timeout
     */
    void send(const String& destination, SMessage& message, std::chrono::milliseconds timeout) override;
};

}

#endif
