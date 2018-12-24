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
protected:

    typedef SynchronizedQueue< std::shared_ptr<Message> > MessageQueue;

    std::map<String, MessageQueue>  m_queues;
    SocketEvents                    m_socketEvents;

    static void socketEventCallback(void *userData, SocketEventType eventType);

public:
    class Connection : public TCPServerConnection
    {
        void readRawMessage(String& destination, Buffer& message);
    public:
        Connection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*);
        void terminate() override;
        void run() override;
    };

    ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override;

public:
    SMQServer();
    void distributeMessage(const String& destination, Message&& message);
};

}

#endif
