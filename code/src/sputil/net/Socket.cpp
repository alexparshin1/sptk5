/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-03                                             ║
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
Socket::Socket(const SOCKET_ADDRESS_FAMILY domain, const int32_t type, const int32_t protocol)
    : SocketVirtualMethods(domain, type, protocol)
{
#ifdef _WIN32
    init();
#endif
}

Socket::~Socket()
{
    close();
}

size_t Socket::read(Buffer& buffer, const size_t size, sockaddr* from)
{
    // Allocate before locking - it touches the buffer, not the socket.
    buffer.reserve(size);
    const WriteLock lock(m_mutex);
    const auto      bytes = readUnlocked(buffer.data(), size, from);
    buffer.bytes(bytes);

    return bytes;
}

size_t Socket::read(String& buffer, const size_t size, sockaddr* from)
{
    // Allocate before locking - it touches the buffer, not the socket.
    buffer.resize(size);
    const WriteLock lock(m_mutex);
    const auto      bytes = readUnlocked(bit_cast<uint8_t*>(buffer.data()), size, from);
    buffer.resize(bytes);

    return bytes;
}

size_t Socket::write(const Buffer& buffer, const sockaddr* peer)
{
    return write(buffer.data(), buffer.bytes(), peer);
}

size_t Socket::write(const String& buffer, const sockaddr* peer)
{
    return write(bit_cast<const uint8_t*>(buffer.c_str()), buffer.length(), peer);
}
