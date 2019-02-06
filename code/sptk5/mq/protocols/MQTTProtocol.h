/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MQTTProtocol.h - description                           ║
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

#ifndef SPTK_MQTTPROTOCOL_H
#define SPTK_MQTTPROTOCOL_H

#include <sptk5/mq/protocols/MQProtocol.h>
#include "MQTTFrame.h"

namespace sptk {

class MQTTProtocol : public MQProtocol
{

public:
    MQTTProtocol(TCPSocket& socket) : MQProtocol(socket) {}

    static Message::Type mqMessageType(MQTTFrameType nativeMessageType);
    static MQTTFrameType nativeMessageType(Message::Type mqMessageType);

    void ack(Message::Type sourceMessageType, const String& messageId) override;
    bool readMessage(SMessage& message) override;
    bool sendMessage(const String& destination, SMessage& message) override;
};

}

#endif //SPTK_MQTTPROTOCOL_H
