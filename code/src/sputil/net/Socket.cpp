/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/net/Socket.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
namespace {
atomic_bool winsockInitialized(false);
}
#endif

void SocketVirtualMethods::openUnlocked(const Host&, OpenMode, bool, std::chrono::milliseconds, const char*)
{
    // Implement in derived class
}

#ifdef _WIN32
void Socket::init() noexcept
{
    if (winsockInitialized)
        return;
    winsockInitialized = true;
    WSADATA        wsaData = {};
    constexpr WORD wVersionRequested = MAKEWORD(2, 0);
    WSAStartup(wVersionRequested, &wsaData);
}

void Socket::cleanup() noexcept
{
    winsockInitialized = false;
    WSACleanup();
}
#endif

// Constructor
Socket::Socket(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
    : SocketVirtualMethods(domain, type, protocol)
{
#ifdef _WIN32
    init();
#endif
}

Socket::~Socket()
{
    Socket::close();
}

size_t Socket::read(Buffer& buffer, size_t size, sockaddr_in* from)
{
    buffer.checkSize(size);
    const size_t bytes = readUnlocked(buffer.data(), size, from);
    buffer.bytes(bytes);

    return bytes;
}

size_t Socket::read(String& buffer, size_t size, sockaddr_in* from)
{
    buffer.resize(size);
    const size_t bytes = readUnlocked(bit_cast<uint8_t*>(buffer.data()), size, from);
    buffer.resize(bytes);

    return bytes;
}

size_t Socket::write(const Buffer& buffer, const sockaddr_in* peer)
{
    return write(buffer.data(), buffer.bytes(), peer);
}

size_t Socket::write(const String& buffer, const sockaddr_in* peer)
{
    return write(bit_cast<const uint8_t*>(buffer.c_str()), buffer.length(), peer);
}
