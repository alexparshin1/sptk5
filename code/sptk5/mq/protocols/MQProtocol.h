/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MQProtocol.h - description                             ║
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

#ifndef __MQ_PROTOCOLS_H__
#define __MQ_PROTOCOLS_H__

#include <sptk5/cnet>
#include <sptk5/mq/Message.h>

namespace sptk {

/**
 * MQ Protocols
 */
enum MQProtocolType
{
    MP_SMQ,
    MP_MQTT,
    MP_STOMP,
    MP_AMQP
};

class MQProtocol
{
    TCPSocket&  m_socket;

public:

    typedef std::map<String, String> Parameters;

protected:

    MQProtocol(TCPSocket& socket) : m_socket(socket) {}

    TCPSocket& socket() const;

public:

    template<class T> size_t read(T& data)
    {
        return m_socket.read((char*)&data, sizeof(data));
    }
    size_t read(String& str);
    size_t read(Buffer& data);
    size_t read(char* data, size_t dataSize);

    template<class T> size_t write(const T& data)
    {
        return m_socket.write((const char*)&data, sizeof(data));
    }
    size_t write(String& str);
    size_t write(Buffer& data);

    virtual void ack(Message::Type sourceMessageType, const String& messageId) = 0;
    virtual bool readMessage(SMessage& message) = 0;
    virtual bool sendMessage(const String& destination, SMessage& message) = 0;

    static std::shared_ptr<MQProtocol> factory(MQProtocolType protocolType, TCPSocket& socket);
};

} // namespace sptk

#endif
