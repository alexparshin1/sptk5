/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQProtocol.cpp - description                          ║
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

#include <sptk5/mq/protocols/SMQProtocol.h>

using namespace std;
using namespace sptk;

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
        if (valueStart != nullptr)
            *valueEnd = 0;
        headers[ptr] = valueStart;
        ptr = valueEnd + 1;
    }
}

void SMQProtocol::ack(Message::Type sourceMessageType, const String& messageId)
{
/*
    auto ackMessage = make_shared<Message>(Message::ACK);
    ackMessage->headers()["id"] = messageId;
    switch (sourceMessageType) {
        case Message::CONNECT:
            sendMessage("", ackMessage);
            break;
        default:
            break;
    }
*/
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
    if (!headers.empty())
        parseHeaders(headers, outputMessage->headers());

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
    for (auto itor: message->headers()) {
        headers.append(itor.first);
        headers.append(": ", 2);
        headers.append(itor.second);
        headers.append('\n');
    }
    output.append((uint32_t) headers.bytes());
    output.append(headers.c_str(), headers.bytes());

    if ((message->type() & (Message::MESSAGE|Message::SUBSCRIBE)) != 0) {
        if (message->destination().empty()) {
            throw Exception("Message destination is empty or not defined");
        }

        if (message->type() == Message::MESSAGE) {
            output.append((uint32_t) message->bytes());
            output.append(message->c_str(), message->bytes());
        }
    }

    socket().write(output);

    return true;
}
