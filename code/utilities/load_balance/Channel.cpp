/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "Channel.h"

#ifdef _WIN32
#include <io.h>
#endif

using namespace std;
using namespace sptk;

void Channel::open(SocketType sourceFD, const String& interfaceAddress, const Host& destination)
{
    scoped_lock lock(m_mutex);

    m_source.attach(sourceFD, false);

    m_destination.bind(interfaceAddress.c_str(), 0);
    m_destination.open(destination, Socket::OpenMode::CONNECT, false, chrono::seconds(60));

    m_sourceEvents.add(m_source, (uint8_t*) this, false);
    m_destinationEvents.add(m_destination, (uint8_t*) this, false);
}

void Channel::close()
{
    scoped_lock lock(m_mutex);

    if (m_source.active())
    {
        m_sourceEvents.remove(m_source);
        m_source.close();
    }

    if (m_destination.active())
    {
        m_destinationEvents.remove(m_destination);
        m_destination.close();
    }
}

int Channel::copyData(const TCPSocket& source, const TCPSocket& destination)
{
    scoped_lock lock(m_mutex);

    Buffer buffer(1024);
    uint32_t totalBytes = 0;
    size_t fragmentSize = sizeof(buffer);
    auto readBytes = (int) fragmentSize;

    while ((size_t) readBytes == fragmentSize)
    {

#ifdef _WIN32
        readBytes = _read((int) source.fd(), buffer.data(), (unsigned) fragmentSize);
        if (readBytes < 0)
            throw SystemException("Can't read from socket");

        if (_write((int) destination.fd(), buffer.data(), readBytes) < 0)
            throw SystemException("Can't write to socket");
#else
        readBytes = (int) ::read(source.fd(), buffer.data(), fragmentSize);
        if (readBytes < 0)
        {
            throw SystemException("Can't read from socket");
        }

        if (::write(destination.fd(), buffer.data(), (size_t) readBytes) < 0)
        {
            throw SystemException("Can't write to socket");
        }
#endif
        totalBytes += readBytes;
    }

    return totalBytes;
}
