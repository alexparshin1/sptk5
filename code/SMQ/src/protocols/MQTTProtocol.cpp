/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MQTTProtocol.cpp - description                         ║
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

#include <smq/protocols/MQTTProtocol.h>

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

    auto shortMessageId = uint16_t(messageId.empty() ? 0 : string2int(messageId));
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
    catch (const Exception&) {
        if (socket().active())
            throw;
    }

    return false;
}

bool MQTTProtocol::sendMessage(const String& destination, SMessage& message)
{
    MQTTFrame frame;
    switch (message->type()) {
        case Message::CONNECT:
            frame.setCONNECT(
                    60,
                    (*message)["username"],
                    (*message)["password"],
                    (*message)["client_id"],
                    (*message)["last_will_destination"],
                    (*message)["last_will_message"],
                    MQTT_PROTOCOL_V311
                    );
            break;
        case Message::SUBSCRIBE:
            frame.setSUBSCRIBE(destination, QOS_0);
            break;
        case Message::MESSAGE:
            frame.setPUBLISH(destination, *message, QOS_0);
            break;
        default:
            throw Exception("Message type not handled!");
    }
    socket().send(frame.c_str(), frame.bytes());
    return true;
}

Message::Type MQTTProtocol::mqMessageType(MQTTFrameType nativeMessageType)
{
    switch (nativeMessageType) {
        case FT_CONNECT:        return Message::CONNECT;
        case FT_PUBLISH:        return Message::MESSAGE;
        case FT_SUBSCRIBE:      return Message::SUBSCRIBE;
        case FT_UNSUBSCRIBE:    return Message::UNSUBSCRIBE;
        case FT_DISCONNECT:     return Message::DISCONNECT;
        case FT_PINGREQ:        return Message::PING;
        case FT_PINGRESP:       return Message::PING_ACK;
        case FT_UNSUBACK:       return Message::UNSUBSCRIBE_ACK;
        case FT_SUBACK:         return Message::SUBSCRIBE_ACK;
        case FT_PUBACK:         return Message::PUBLISH_ACK;
        case FT_CONNACK:        return Message::CONNECT_ACK;
        default:                return Message::UNDEFINED;
    }
}

MQTTFrameType MQTTProtocol::nativeMessageType(Message::Type mqMessageType)
{
    switch (mqMessageType) {
        case Message::CONNECT:          return FT_CONNECT;
        case Message::DISCONNECT:       return FT_DISCONNECT;
        case Message::SUBSCRIBE:        return FT_SUBSCRIBE;
        case Message::UNSUBSCRIBE:      return FT_UNSUBSCRIBE;
        case Message::PING:             return FT_PINGREQ;
        case Message::MESSAGE:          return FT_PUBLISH;
        case Message::CONNECT_ACK:      return FT_CONNACK;
        case Message::SUBSCRIBE_ACK:    return FT_CONNACK;
        case Message::UNSUBSCRIBE_ACK:  return FT_UNSUBACK;
        case Message::PUBLISH_ACK:      return FT_PUBACK;
        case Message::PING_ACK:         return FT_PINGRESP;
        default:                        return FT_UNDEFINED;
    }
}

