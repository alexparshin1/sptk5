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

#include <smq/clients/SMQClient.h>
#include <smq/clients/SMQClient.h>


using namespace std;
using namespace sptk;
using namespace chrono;
using namespace smq;

SMQClient::SMQClient(MQProtocolType protocolType, const String& clientId)
: TCPMQClient(protocolType, clientId)
{
}

void SMQClient::connect(const Host& server, const String& username, const String& password, bool encrypted, milliseconds timeout)
{
    UniqueLock(m_mutex);

    createConnection(server, encrypted, timeout);

    m_server = server;
    m_username = username;
    m_password = password;

    auto connectMessage = make_shared<Message>(Message::CONNECT);
    (*connectMessage)["client_id"] = getClientId();
    (*connectMessage)["username"] = username;
    (*connectMessage)["password"] = password;

    if (m_lastWillMessage) {
        (*connectMessage)["last_will_destination"] = m_lastWillMessage->destination();
        (*connectMessage)["last_will_message"] = (String) m_lastWillMessage->message();
    }

    send("", connectMessage, timeout);
}

void SMQClient::disconnect(bool)
{
    destroyConnection();
}

void SMQClient::send(const String& destination, SMessage& message, std::chrono::milliseconds)
{
    protocol().sendMessage(destination, message);
}

void SMQClient::subscribe(const String& destination, std::chrono::milliseconds timeout)
{
    auto subscribeMessage = make_shared<Message>(Message::SUBSCRIBE);
    send(destination, subscribeMessage, timeout);
}

void SMQClient::unsubscribe(const String& destination, std::chrono::milliseconds timeout)
{
    auto unsubscribeMessage = make_shared<Message>(Message::UNSUBSCRIBE);
    send(destination, unsubscribeMessage, timeout);
}

void SMQClient::socketEvent(SocketEventType eventType)
{
    if (eventType == ET_CONNECTION_CLOSED) {
        destroyConnection();
        return;
    }

    SMessage msg;
    try {
        while (connected() && socket().socketBytes() > 0) {
            if (protocol().readMessage(msg) && msg->type() == Message::MESSAGE) {
                acceptMessage(msg);
            }
        }
    }
    catch (const Exception& e) {
        CERR("ERROR: " << e.what() << endl);
        destroyConnection();
    }
}

void SMQClient::setLastWillMessage(std::unique_ptr<MQLastWillMessage>& lastWillMessage)
{
    UniqueLock(m_mutex);
    m_lastWillMessage = move(lastWillMessage);
}
