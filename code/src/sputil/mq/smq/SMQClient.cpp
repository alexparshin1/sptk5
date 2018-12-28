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

using namespace std;
using namespace sptk;

SMQClient::SMQClient()
: Thread("SMQClient")
{}

void SMQClient::connect(const Host& host)
{
    m_socket.open(host);
}

void SMQClient::disconnect()
{
    m_socket.close();
}

void SMQClient::threadFunction()
{

}

void SMQClient::sendMessage(const String& destination, const Message& message)
{
    Buffer output;

    if (!m_socket.active())
        throw Exception("Not connected");

    // Append message type
    output.append((uint8_t)message.type());

    // Append destination
    output.append((uint32_t)destination.size());
    output.append(destination);

    output.append((uint32_t)message.bytes());
    output.append(message.c_str(), message.bytes());

    const char* magic = "MSG:";
    m_socket.write(magic, strlen(magic));
    m_socket.write(output);
}

void SMQClient::subscribe(const String& destination)
{
    sendMessage(destination, Message(Message::SUBSCRIBE));
}
