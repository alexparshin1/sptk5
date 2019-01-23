/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQClient.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
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

#ifndef __SMQ_CLIENT_H__
#define __SMQ_CLIENT_H__

#include <sptk5/cthreads>
#include <sptk5/cnet>
#include <sptk5/mq/SMQMessage.h>

namespace sptk {

class SMQClient : public Thread
{
    std::mutex                  m_mutex;            ///< Mutex that protects internal data
    TCPSocket                   m_socket;           ///< TCP or SSL connection socket
    Host                        m_server;           ///< SMQ server host
    String                      m_clientId;         ///< Unique SMQ client id
    String                      m_username;         ///< Connection user name
    String                      m_password;         ///< Connection password
    SMessageQueue               m_receivedMessages; ///< Received messages queue

protected:
    /**
     * Thread function
     *
     * Retrieves incoming messages
     */
    void threadFunction() override;

    /**
     * Preview received messages before placing them in the received messages queue.
     * Message may be modified, or ignored.
     * @param message           Received message
     * @return true if message should be placed to the received messages queue.
     */
    virtual bool previewMessage(Message& message) { return true; }

public:

    /**
     * Constructor
     */
    SMQClient();

    /**
     * Check if client is connected to server
     * @return true if client is connected to server
     */
    bool connected() const { return m_socket.active(); }

    /**
     * Connect to SMQ server
     * @param server            SMQ server host
     * @param clientId          Unique SMQ client id
     * @param username          SMQ server username
     * @param password          SMQ server password
     */
    void connect(const Host& server, const String& clientId, const String& username, const String& password);

    /**
     * Disconnect from SMQ server
     */
    void disconnect();

    /**
     * Subscribe to a queue
     * @param destination       Destination queue name
     */
    void subscribe(const String& destination);

    /**
     * Get a single message from received messages queue
     * @param timeout           Timeout
     * @return shared pointer to message, or empty pointer if timeout
     */
    SMessage getMessage(std::chrono::milliseconds timeout);

    /**
     * Send a message
     *
     * Destination queue is defined in message 'destination' header
     * @param message           Message to send
     */
    void sendMessage(const Message& message);

    /**
     * Check if client has any messages in received messages queue
     * @return number of messages available
     */
    size_t hasMessages() const;
    /**
     * Get client id
     * @return
     */
    const String& clientId() const { return m_clientId; }
};

}

#endif
