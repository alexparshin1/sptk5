/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQProtocol.cpp - description                          ║
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

#include <smq/protocols/SMQProtocol.h>

using namespace std;
using namespace sptk;
using namespace smq;

static void parseHeaders(Buffer& buffer, Message::Headers& headers)
{
    char* pstart = buffer.data();
    char* pend = buffer.data() + buffer.bytes();
    for (char* ptr = pstart; ptr < pend; ) {
        char* nameEnd = strstr(ptr, ": ");
        if (nameEnd == nullptr)
            break;
        *nameEnd = 0;
        char* valueStart = nameEnd + 2;
        char* valueEnd = strchr(valueStart, '\n');
        if (valueEnd != nullptr)
            *valueEnd = 0;
        headers[ptr] = valueStart;
        if (valueEnd == nullptr)
            break;
        ptr = valueEnd + 1;
    }
}

void SMQProtocol::ack(Message::Type sourceMessageType, const String& messageId)
{
    return;

    Message::Type ackType;
    switch (sourceMessageType) {
        case Message::CONNECT:
            ackType = Message::CONNECT_ACK;
            break;
        case Message::SUBSCRIBE:
            ackType = Message::SUBSCRIBE_ACK;
            break;
        case Message::UNSUBSCRIBE:
            ackType = Message::SUBSCRIBE_ACK;
            break;
        case Message::MESSAGE:
            ackType = Message::PUBLISH_ACK;
            break;
        case Message::PING:
            ackType = Message::PING_ACK;
            break;
        default:
            return;
    }
    auto ackMessage = make_shared<Message>(ackType);
    ackMessage->headers()["id"] = messageId;
    sendMessage("", ackMessage);
}

bool SMQProtocol::readMessage(SMessage& outputMessage)
{
    char    data[16];
    Buffer  headers;
    Buffer  message;
    uint8_t messageType;

    // Read message signature
    size_t bytes = read(data, 4);
    if (bytes == 0)
        throw ConnectionException("Connection closed");

    if (strncmp(data, "MSG:", 4) != 0)
        throw Exception("Invalid message magic byte");

    // Read message type
    read((char*)&messageType, sizeof(messageType));
    if (messageType > Message::MESSAGE)
        throw Exception("Invalid message type");

    switch (messageType) {
        case Message::CONNECT:
        case Message::SUBSCRIBE:
            read(headers);
            break;

        case Message::MESSAGE:
            read(headers);
            read(message);
            break;

        default:
            message.bytes(0);
            break;
    }

    outputMessage = make_shared<Message>((Message::Type) messageType, move(message));
    if (!headers.empty()) {
        parseHeaders(headers, outputMessage->headers());
        auto itor = outputMessage->headers().find("destination");
        if (itor != outputMessage->headers().end()) {
            outputMessage->destination(itor->second);
            outputMessage->headers().erase(itor);
        }
    }

    return true;
}

bool SMQProtocol::sendMessage(const String& destination, SMessage& message)
{
    Buffer output("MSG:", 4);

    if (!socket().active())
        throw Exception("Not connected");

    // Append message type
    output.append(message->type());
    message->destination(destination);

    Buffer headers;

    headers.append("destination: ", 13);
    headers.append(destination);
    headers.append('\n');

    for (auto& itor: message->headers()) {
        headers.append(itor.first);
        headers.append(": ", 2);
        headers.append(itor.second);
        headers.append('\n');
    }

    output.append((uint32_t) headers.bytes());
    output.append(headers.c_str(), headers.bytes());

    if (message->type() == Message::MESSAGE || message->type() == Message::SUBSCRIBE) {
        if (message->destination().empty())
            throw Exception("Message destination is empty or not defined");

        if (message->type() == Message::MESSAGE) {
            output.append((uint32_t) message->bytes());
            output.append(message->c_str(), message->bytes());
        }
    }

    socket().write(output);

    return true;
}
