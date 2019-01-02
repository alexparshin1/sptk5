/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.h - description                              ║
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

#ifndef __SMQ_SERVER_H__
#define __SMQ_SERVER_H__

#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerConnection.h>
#include <src/sputil/mq/Message.h>
#include <sptk5/net/SocketEvents.h>

namespace sptk {

class SMQServer : public TCPServer
{
    std::mutex                                          m_mutex;
    String                                              m_username;
    String                                              m_password;
protected:

    typedef std::shared_ptr<Message>                    SMessage;
    typedef SynchronizedQueue<SMessage>                 SMessageQueue;

    std::map<String, std::shared_ptr<SMessageQueue>>    m_queues;
    SocketEvents                                        m_socketEvents;

    static void socketEventCallback(void *userData, SocketEventType eventType);

public:
    class Connection : public TCPServerConnection
    {
        std::mutex                      m_mutex;
        std::shared_ptr<SMessageQueue>  m_subscribedQueue;

        std::shared_ptr<SMessageQueue>  subscribedQueue();

        template<class T> void read(T& data)
        {
            socket().read((char*)&data, sizeof(data));
        }

        void read(String& str);
        void read(Buffer& str);

        void readConnect();
        void readMessage(String& destination, Buffer& message);
    public:
        Connection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*);
        ~Connection() override;
        void run() override;
        void terminate() override;
        void readRawMessage(String& destination, Buffer& message, uint8_t& messageType);
        void subscribeTo(const String& destination);
    };

    ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override;
    bool authenticate(const String& username, const String& password);

public:
    SMQServer(const String& username, const String& password, LogEngine& logEngine);
    std::shared_ptr<SMessageQueue> getClientQueue(const String& destination);
    void distributeMessage(const String& destination, SMessage message);

    static void sendMessage(TCPSocket& socket, const Message& message);
};

}

#endif
