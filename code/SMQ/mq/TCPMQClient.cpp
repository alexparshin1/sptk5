/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPMQClient.cpp - description                          ║
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

#include "SMQ/mq/TCPMQClient.h"

using namespace std;
using namespace sptk;
using namespace chrono;

SharedSocketEvents TCPMQClient::smqSocketEvents;

SharedSocketEvents& TCPMQClient::initSocketEvents()
{
    static mutex              amutex;

    lock_guard<mutex> lock(amutex);

    if (!smqSocketEvents)
        smqSocketEvents = make_shared<SocketEvents>("MQ Client", smqSocketEventCallback);

    return smqSocketEvents;
}

TCPMQClient::TCPMQClient(MQProtocolType protocolType, const String& clientId)
: BaseMQClient(protocolType, clientId)
{
    UniqueLock(m_mutex);
    initSocketEvents();
}

TCPMQClient::~TCPMQClient()
{
    destroyConnection();
}

void TCPMQClient::createConnection(const Host& server, bool encrypted, std::chrono::milliseconds timeout)
{
    UniqueLock(m_mutex);
    if (m_socket && m_socket->active())
        return;
    if (encrypted) {
        m_socket = make_shared<SSLSocket>();
        //loadKeys(keyFile, certificateFile, password, caFile, verifyMode, verifyDepth);
    } else {
        m_socket = make_shared<TCPSocket>();
    }
    m_socket->open(server, TCPSocket::SOM_CONNECT, true, timeout);
    smqSocketEvents->add(*m_socket, this);

    m_protocol = MQProtocol::factory(protocolType(), *m_socket);
}

void TCPMQClient::destroyConnection()
{
    UniqueLock(m_mutex);
    if (m_socket) {
        smqSocketEvents->remove(*m_socket);
        m_socket->close();
        m_socket.reset();
    }
}

void TCPMQClient::smqSocketEventCallback(void* userData, SocketEventType eventType)
{
    auto* client = (TCPMQClient*) userData;
    client->socketEvent(eventType);
}

void TCPMQClient::loadSslKeys(const SSLKeys& keys)
{
    if (m_socket) {
        SSLSocket* sslSocket = dynamic_cast<SSLSocket*>(m_socket.get());
        if (sslSocket != nullptr)
            sslSocket->loadKeys(keys);
    }
}

MQProtocol& TCPMQClient::protocol()
{
    return *m_protocol;
}
