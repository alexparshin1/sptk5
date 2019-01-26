/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.cpp - description                            ║
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

#include <sptk5/mq/SMQServer.h>
#include <sptk5/cutils>
#include <sptk5/mq/SMQConnection.h>


using namespace std;
using namespace sptk;

SMQConnection::SMQConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*)
: TCPServerConnection(server, connectionSocket)
{
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server);
    if (smqServer != nullptr)
        smqServer->watchSocket(socket(), this);
}

SMQConnection::~SMQConnection()
{
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
    if (smqServer != nullptr)
        smqServer->forgetSocket(socket());
}

void SMQConnection::terminate()
{
    socket().close();
    TCPServerConnection::terminate();
}

void SMQConnection::subscribeTo(const String& destination)
{
    UniqueLock(m_mutex);

    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
    m_subscribedQueue = smqServer->getClientQueue(destination);

    smqServer->log(LP_INFO, "(" + m_clientId + ") Subscribed to " + destination);
}

shared_ptr<SMessageQueue> SMQConnection::subscribedQueue()
{
    SharedLock(m_mutex);
    return m_subscribedQueue;
}

void SMQConnection::run()
{
    while (!terminated()) {
        try {
            shared_ptr<SMessageQueue> queue = subscribedQueue();
            SMessage message;
            if (queue) {
                if (queue->pop(message, chrono::milliseconds(1000))) {
                    SMQMessage::sendMessage(socket(), *message);
                }
            }
            else
                this_thread::sleep_for(chrono::milliseconds(10));
        }
        catch (const Exception& e) {
            CERR(e.what() << endl);
        }
    }
    socket().close();
}

String SMQConnection::getClientId() const
{
    SharedLock(m_mutex);
    return m_clientId;
}

void SMQConnection::setClientId(String& id)
{
    UniqueLock(m_mutex);
    m_clientId = id;
}
