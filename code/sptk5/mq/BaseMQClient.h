/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseMQClient.h - description                           ║
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

#ifndef __BASE_MQ_CLIENT_H__
#define __BASE_MQ_CLIENT_H__

#include <sptk5/cutils>
#include <sptk5/cnet>
#include <sptk5/mq/Message.h>
#include <sptk5/mq/protocols/MQProtocol.h>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/cthreads>
#include "TCPMQClient.h"

namespace sptk {

/**
 * Base MQ client interface
 *
 * All derived MQ clients must implement this interface.
 */
class BaseMQClient
{
    /**
     * Mutex that protects internal data
     */
    mutable SharedMutex             m_mutex;

    /**
     * Unique client id
     */
    String                          m_clientId;

    /**
     * Connection host
     */
    Host                            m_host;

    /**
     * Flag: connected to MQ server
     */
    bool                            m_connected {false};

    /**
     * Flag: disable reconnects
     */
    bool                            m_enabled {true};

    /**
     * Queue of incoming messages
     */
    SynchronizedQueue<SMessage>     m_incomingMessages;

    /**
     * MQ protocol type
     */
    const MQProtocolType            m_protocolType;

protected:

    /**
     * Preview received messages before placing them in the received messages queue.
     * Message may be modified, or ignored.
     * @param message           Received message
     * @return true if message should be placed to the received messages queue.
     */
    virtual bool previewMessage(Message& message) { return true; }

    /**
     * Place received message into received messages queue.
     * @param message           Received message
     * @return true if message should be placed to the received messages queue.
     */
    void acceptMessage(SMessage& message);

public:

    /**
     * Constructor
     * @param protocolType      MQ protocol type
     * @param clientId          Unique client id
     */
    BaseMQClient(MQProtocolType protocolType, const String& clientId);

    /**
     * Destructor
     */
    virtual ~BaseMQClient() = default;

    /*
     * Connect to MQ server
     * @param server            MQ server host and port
     * @param user              MQ server username.
     * @param password          MQ server password.
     * @param encrypted         Use encrypted connection. If true, then SSL keys must be loaded prior to this call.
     * @param timeout           Operation timeout
     */
    virtual void connect(const Host& server, const String& username, const String& password, bool encrypted,
                         std::chrono::milliseconds timeout) = 0;

    /**
     * Disconnect from server
     * @param immediate         If false then disconnect as defined by server protocol. Otherwise, just terminate connection.
     */
    virtual void disconnect(bool immediate) = 0;

    /**
     * Load SSL private key and certificate(s).
     *
     * Should be called at least once to load PEM keys and certificates required for SSL authentication.
     * @param keyFile           Private key file name
     * @param certificateFile   Certificate file name
     * @param password          Optional private key file password
     * @param caFile            Optional root certificate file name
     * @param verifyMode        SSL verify mode, minimal is SSL_VERIFY_NONE
     * @param verifyDepth       SSL verify depth, minimal is 0
     */
    virtual void loadSslKeys(const String& keyFile, const String& certificateFile, const String& password,
                             const String& caFile, int verifyMode, int verifyDepth) = 0;

    /**
     * Send message
     * @param destination       Queue or topic name
     * @param message           Message
     * @param timeout           Operation timeout
     */
    virtual void send(const String& destination, SMessage& message, std::chrono::milliseconds timeout) = 0;

    /**
     * Receive a message
     * @param timeout           Operation timeout
     * @returns shared ptr message, that is false if timeout
     */
    virtual SMessage getMessage(std::chrono::milliseconds timeout);

    /**
     * Subscribe to queue or topic
     * @param destination       Queue or topic name
     * @param timeout           Operation timeout
     */
    virtual void subscribe(const String& destination, std::chrono::milliseconds timeout) = 0;

    /**
     * Un-subscribe from queue or topic
     * @param destination       Queue or topic name
     * @param timeout           Operation timeout
     */
    virtual void unsubscribe(const String& destination, std::chrono::milliseconds timeout) = 0;

    /**
     * Get automatically generated unique client id
     */
    const String& getClientId() const;

    /**
     * Get connection host
     * @return connection host name
     */
    Host getHost() const;

    /**
     * Return client connection status
     * @return true if client is connected
     */
    virtual bool connected() const;

    /**
     * Check if client has any messages in received messages queue
     * @return number of messages available
     */
    size_t hasMessages() const;

    /**
     * MQ protocol type
     * @return
     */
    MQProtocolType protocolType() const;

    /**
     * Disable or enable MQ client.
     * @param state             If false, then client is disabled.
     */
    void enable(bool state);

    /**
     * Return true if client is enabled
     */
    bool enabled() const;
};

} // namespace sptk

#endif
