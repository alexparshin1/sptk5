/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
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

void Channel::open(const SocketType sourceFD, const String& interfaceAddress, const Host& destination)
{
    scoped_lock lock(m_mutex);

    m_source->attach(sourceFD, false);

    m_destination->bind(interfaceAddress.c_str(), 0);
    m_destination->open(destination, Socket::OpenMode::CONNECT, false, chrono::seconds(60));

    m_sourceEvents.add(m_source, shared_from_this());
    m_destinationEvents.add(m_destination, shared_from_this());
}

void Channel::close()
{
    scoped_lock lock(m_mutex);

    if (m_source->active())
    {
        m_sourceEvents.remove(m_source);
        m_source->close();
    }

    if (m_destination->active())
    {
        m_destinationEvents.remove(m_destination);
        m_destination->close();
    }
}

int Channel::copyData(const TCPSocket& source, const TCPSocket& destination)
{
    scoped_lock lock(m_mutex);

    constexpr size_t fragmentSize = 1024;
    Buffer           buffer(fragmentSize);
    uint32_t         totalBytes = 0;
    auto             readBytes = static_cast<int>(fragmentSize);

    while (static_cast<size_t>(readBytes) == fragmentSize)
    {

#ifdef _WIN32
        readBytes = _read(static_cast<int>(source.fd()), buffer.data(), fragmentSize);
        if (readBytes < 0)
            throw SystemException("Can't read from socket");

        if (_write(static_cast<int>(destination.fd()), buffer.data(), readBytes) < 0)
            throw SystemException("Can't write to socket");
#else
        readBytes = static_cast<int>(::read(source.fd(), buffer.data(), fragmentSize));
        if (readBytes < 0)
        {
            throw SystemException("Can't read from socket");
        }

        if (::write(destination.fd(), buffer.data(), static_cast<size_t>(readBytes)) < 0)
        {
            throw SystemException("Can't write to socket");
        }
#endif
        totalBytes += readBytes;
    }

    return totalBytes;
}
