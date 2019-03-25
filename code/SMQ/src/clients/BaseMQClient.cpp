/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseMQClient.cpp - description                         ║
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

using namespace std;
using namespace sptk;
using namespace chrono;

BaseMQClient::BaseMQClient(MQProtocolType protocolType, const String& clientId)
: m_clientId(clientId), m_protocolType(protocolType)
{}

const String& BaseMQClient::getClientId() const
{
    SharedLock(m_mutex);
    return m_clientId;
}

const Host& BaseMQClient::getHost() const
{
    SharedLock(m_mutex);
    return m_host;
}

bool BaseMQClient::connected() const
{
    SharedLock(m_mutex);
    return m_connected;
}

SMessage BaseMQClient::getMessage(std::chrono::milliseconds timeout)
{
    SMessage message;
    m_incomingMessages.pop(message, timeout);
    return message;
}

size_t BaseMQClient::hasMessages() const
{
    return m_incomingMessages.size();
}

void BaseMQClient::acceptMessage(SMessage& message)
{
    try {
        if (previewMessage(message) && message)
            m_incomingMessages.push(message);
    }
    catch (const Exception& e) {
        CERR("Can't accept message: " + String(e.what()));
    }
}

MQProtocolType BaseMQClient::protocolType() const
{
    SharedLock(m_mutex);
    return m_protocolType;
}

void BaseMQClient::enable(bool state)
{
    UniqueLock(m_mutex);
    m_enabled = state;
}

bool BaseMQClient::enabled() const
{
    SharedLock(m_mutex);
    return m_enabled;
}
