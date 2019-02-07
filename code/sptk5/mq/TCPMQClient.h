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

#include <sptk5/mq/BaseMQClient.h>
#include <sptk5/mq/protocols/MQProtocol.h>

namespace sptk {

class TCPMQClient : public BaseMQClient
{
    mutable SharedMutex                 m_mutex;
    std::shared_ptr<TCPSocket>          m_socket;           ///< TCP or SSL connection socket
    std::shared_ptr<MQProtocol>         m_protocol;         ///< MQ protocol
    static SharedSocketEvents           smqSocketEvents;    ///< Shared event manager

private:
    /**
     * Callback function that receives socket events
     * @param userData          Socket event data, here - pointer to SMQ client object
     * @param eventType         Socket event type
     */
    static void smqSocketEventCallback(void *userData, SocketEventType eventType);

    /**
     * Initialize shared event manager
     * @return
     */
    static SharedSocketEvents& initSocketEvents();

protected:

    /**
     * Constructor
     * @param clientId          Unique client id
     */
    TCPMQClient(MQProtocolType protocolType, const String& clientId);

public:

    virtual ~TCPMQClient();

    MQProtocol& protocol();

    void loadSslKeys(const String& keyFile, const String& certificateFile, const String& password, const String& caFile,
                     int verifyMode, int verifyDepth) override;

    /**
     * Check if client is connected to server
     * @return true if client is connected to server
     */
    bool connected() const override { return m_socket->active(); }

protected:

    TCPSocket& socket()
    {
        return *m_socket;
    }

    virtual void socketEvent(SocketEventType eventType) = 0;

    void createConnection(const Host& server, bool encrypted, std::chrono::milliseconds timeout);

    void destroyConnection();
};

} // namespace sptk

#endif
