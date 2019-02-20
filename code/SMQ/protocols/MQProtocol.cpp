/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MQProtocol.cpp - description                           ║
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

#include <SMQ/protocols/SMQProtocol.h>
#include <SMQ/protocols/MQTTProtocol.h>


using namespace std;
using namespace sptk;
using namespace chrono;

size_t MQProtocol::read(String& str)
{
    uint8_t dataSize;
    read(dataSize);
    if (dataSize == 0)
        throw Exception("Invalid string size");
    return m_socket.read(str, dataSize) + sizeof(uint8_t);
}

size_t MQProtocol::read(Buffer& data)
{
    uint32_t dataSize;
    read(dataSize);
    if (dataSize > 0) {
        data.checkSize(dataSize);
        m_socket.read(data.data(), dataSize);
    }
    data.bytes(dataSize);
    return dataSize + sizeof(dataSize);
}

size_t MQProtocol::read(char* data, size_t dataSize)
{
    return m_socket.read(data, dataSize);
}

size_t MQProtocol::write(String& str)
{
    return m_socket.write(str.c_str(), str.length());
}

size_t MQProtocol::write(Buffer& data)
{
    return m_socket.write(data.c_str(), data.bytes());
}

std::shared_ptr<MQProtocol> MQProtocol::factory(MQProtocolType protocolType, TCPSocket& socket)
{
    switch (protocolType) {
        case MP_SMQ:    return make_shared<SMQProtocol>(socket);
        case MP_MQTT:   return make_shared<MQTTProtocol>(socket);
        default:        throw Exception("Protocol is not yet supported");
    }
}

TCPSocket& MQProtocol::socket() const
{
    return m_socket;
}
