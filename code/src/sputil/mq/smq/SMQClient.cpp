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

#include "SMQClient.h"
#include "SMQMessage.h"
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

SMQClient::SMQClient()
: Thread("SMQClient")
{}

void SMQClient::connect(const Host& server, const String& clientId, const String& username, const String& password)
{
    lock_guard<mutex> lock(m_mutex);

    if (m_socket.active()) {
        if (m_server == server && m_clientId == clientId && m_username == username && m_password == password)
            return;
        m_socket.close();
    }

    m_server = server;
    m_clientId = clientId;
    m_username = username;
    m_password = password;

    m_socket.open(server);
    Message connectMessage(Message::CONNECT);
    connectMessage.append((uint8_t)clientId.length());
    connectMessage.append(clientId.c_str(), clientId.length());
    connectMessage.append((uint8_t)username.length());
    connectMessage.append(username.c_str(), username.length());
    connectMessage.append((uint8_t)password.length());
    connectMessage.append(password.c_str(), password.length());
    sendMessage(connectMessage);

    if (!running())
        run();
}

void SMQClient::disconnect()
{
    terminate();
    join();
    m_socket.close();
}

void SMQClient::threadFunction()
{
    String errorMessagePrefix;
    while (!terminated()) {
        try {
            if (!m_socket.active() && !m_clientId.empty()) {
                try {
                    errorMessagePrefix = "SMQClient " + m_clientId + ": ";
                    connect(m_server, m_clientId, m_username, m_password);
                }
                catch (const Exception& e) {
                    CERR(errorMessagePrefix << e.what() << endl);
                    sleep_for(chrono::seconds(10));
                    continue;
                }
            }

            if (m_socket.readyToRead(chrono::seconds(1))) {
                auto message = SMQMessage::readRawMessage(m_socket);
                m_receivedMessages.push(message);
            }
        }
        catch (const Exception& e) {
            CERR(errorMessagePrefix << e.what() << endl);
            m_socket.close();
        }
    }
}

void SMQClient::sendMessage(const Message& message)
{
    Buffer output;

    if (!m_socket.active())
        throw Exception("Not connected");

    // Append message type
    output.append((uint8_t)message.type());

    if (message.type() == Message::MESSAGE || message.type() == Message::SUBSCRIBE) {
        if (message.destination().empty())
            throw Exception("Empty destination");
        // Append destination
        output.append((uint8_t) message.destination().size());
        output.append(message.destination());
    }

    output.append((uint32_t)message.bytes());
    output.append(message.c_str(), message.bytes());

    const char* magic = "MSG:";
    m_socket.write(magic, strlen(magic));
    m_socket.write(output);
}

void SMQClient::subscribe(const String& destination)
{
    Message subscribeMessage(Message::SUBSCRIBE);
    subscribeMessage.destination(destination);
    sendMessage(subscribeMessage);
}

size_t SMQClient::hasMessages() const
{
    return m_receivedMessages.size();
}

SMessage SMQClient::getMessage(std::chrono::milliseconds timeout)
{
    SMessage message;
    m_receivedMessages.pop(message, timeout);
    return sptk::SMessage();
}
