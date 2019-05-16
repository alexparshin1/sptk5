/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPMQClient.h - description                            ║
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

#ifndef __TCP_MQ_CLIENT_H__
#define __TCP_MQ_CLIENT_H__

#include <smq/clients/BaseMQClient.h>
#include <smq/protocols/MQProtocol.h>

namespace smq {

class SP_EXPORT TCPMQClient : public BaseMQClient
{
    mutable sptk::SharedMutex                 m_mutex;
    std::shared_ptr<sptk::TCPSocket>          m_socket;           ///< TCP or SSL connection socket
    std::shared_ptr<MQProtocol>               m_protocol;         ///< MQ protocol
    static sptk::SharedSocketEvents           smqSocketEvents;    ///< Shared event manager

    /**
     * Callback function that receives socket events
     * @param userData          Socket event data, here - pointer to SMQ client object
     * @param eventType         Socket event type
     */
    static void smqSocketEventCallback(void *userData, sptk::SocketEventType eventType);

    /**
     * Initialize shared event manager
     * @return
     */
    static sptk::SharedSocketEvents& initSocketEvents();

protected:

    /**
     * Constructor
     * @param clientId          Unique client id
     */
    TCPMQClient(MQProtocolType protocolType, const sptk::String& clientId);

public:

    ~TCPMQClient() override;

    MQProtocol& protocol();

    void loadSslKeys(const sptk::SSLKeys& keys) override;

    /**
     * Check if client is connected to server
     * @return true if client is connected to server
     */
    bool connected() const override { return m_socket? m_socket->active(): false; }

protected:

    sptk::TCPSocket& socket()
    {
        return *m_socket;
    }

    virtual void socketEvent(sptk::SocketEventType eventType) = 0;

    void createConnection(const sptk::Host& server, bool encrypted, std::chrono::milliseconds timeout);

    void destroyConnection();
};

} // namespace sptk

#endif
