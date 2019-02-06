/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MQTTProtocol.cpp - description                         ║
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

#include <sptk5/mq/protocols/MQTTProtocol.h>
#include <sptk5/mq/protocols/MQTTFrame.h>

using namespace std;
using namespace sptk;
using namespace chrono;

void MQTTProtocol::ack(Message::Type sourceMessageType, const String& messageId)
{
    MQTTFrameType ackType = FT_UNDEFINED;
    switch (sourceMessageType) {
        case Message::CONNECT:
            ackType = FT_CONNACK;
            break;
        default:
            return;
    }

    uint16_t shortMessageId = messageId.empty() ? 0 : (uint16_t) string2int(messageId);
    MQTTFrame ackFrame(ackType, shortMessageId, QOS_0);
    ackFrame.setACK(shortMessageId, ackType);
    socket().send(ackFrame.c_str(), ackFrame.bytes());
}

bool MQTTProtocol::readMessage(SMessage& message)
{
    MQTTFrame frame(FT_UNDEFINED, 0, QOS_0);

    Message::Headers headers;
    try {
        if (frame.read(socket(), headers, seconds(10))) {
            message = make_shared<Message>(mqMessageType(frame.type()));
            message->headers() = move(headers);
            message->set(frame);
            return true;
        }
    }
    catch (...) {
        if (socket().active())
            throw;
    }

    return false;
}

bool MQTTProtocol::sendMessage(const String& destination, SMessage& message)
{
    return false;
}

Message::Type MQTTProtocol::mqMessageType(MQTTFrameType nativeMessageType)
{
    switch (nativeMessageType) {
        case FT_CONNECT:        return Message::CONNECT;
        case FT_PUBLISH:        return Message::MESSAGE;
        case FT_SUBSCRIBE:      return Message::SUBSCRIBE;
        case FT_UNSUBSCRIBE:    return Message::UNSUBSCRIBE;
        case FT_DISCONNECT:     return Message::DISCONNECT;
        case FT_PINGREQ:
        case FT_PINGRESP:       return Message::PING;
        case FT_UNSUBACK:
        case FT_SUBACK:
        case FT_PUBACK:
        case FT_CONNACK:        return Message::ACK;
        default:                return Message::UNDEFINED;
    }
}

MQTTFrameType MQTTProtocol::nativeMessageType(Message::Type mqMessageType)
{
    switch (mqMessageType) {
        case Message::CONNECT:      return FT_CONNECT;
        case Message::DISCONNECT:   return FT_DISCONNECT;
        case Message::SUBSCRIBE:    return FT_SUBSCRIBE;
        case Message::UNSUBSCRIBE:  return FT_UNSUBSCRIBE;
        case Message::PING:         return FT_PINGREQ;
        case Message::MESSAGE:      return FT_PUBLISH;
        case Message::ACK:          return FT_PUBACK;
        default:                    return FT_UNDEFINED;
    }
}

