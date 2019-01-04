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
    std::mutex                  m_mutex;
    TCPSocket                   m_socket;
    Host                        m_server;
    String                      m_clientId;
    String                      m_username;
    String                      m_password;
    SMessageQueue               m_receivedMessages;
protected:
    void threadFunction() override;
public:
    SMQClient();
    void connect(const Host& server, const String& clientId, const String& username, const String& password);
    void disconnect();
    void subscribe(const String& destination);
    SMessage getMessage(std::chrono::milliseconds timeout);
    void sendMessage(const Message& message);
    size_t hasMessages() const;
};

}

#endif
