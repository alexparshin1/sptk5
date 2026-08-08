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

#include <sptk5/cutils>
#include <sptk5/net/TCPSocket.h>

using namespace std;
using namespace sptk;

TCPSocket::TCPSocket(const SOCKET_ADDRESS_FAMILY domain, const int32_t type, const int32_t protocol)
    : Socket(domain, type, protocol)
{
}

void TCPSocket::openUnlocked(const Host&                 host, const OpenMode openMode, const bool _blockingMode,
                             const chrono::milliseconds& timeout, const char* clientBindAddress)
{
    if (!host.hostname().empty())
    {
        setHostUnlocked(host);
    }
    else if (getHostUnlocked().hostname().empty())
    {
        throw ConnectionException("Please, define the host name");
    }

    if (proxy() != nullptr)
    {
        const auto socketHandle = proxy()->connect(getHostUnlocked(), _blockingMode, timeout);
        attachUnlocked(socketHandle, false);
    }
    else
    {
        sockaddr_in addr = {};
        getHostUnlocked().getAddress(addr);
        openUnlocked(addr, openMode, _blockingMode, timeout, clientBindAddress);
    }
}

void TCPSocket::openUnlocked(const sockaddr_in&          address, const OpenMode openMode, const bool _blockingMode,
                             const chrono::milliseconds& timeoutMS, const char*  clientBindAddress)
{
    openAddressUnlocked(address, openMode, timeoutMS, true, clientBindAddress);

    if (!_blockingMode)
    {
        setBlockingModeUnlocked(false);
    }
}

bool TCPSocket::accept(SocketType&                 clientSocketFD, sockaddr_storage& clientInfo, socklen_t& clientInfoLength,
                       const chrono::milliseconds& timeout)
{
    clientSocketFD = INVALID_SOCKET;
    clientInfoLength = 0;

    if (!readyToRead(timeout))
    {
        return false;
    }

    const WriteLock lock(getMutex());

    if (!activeUnlocked())
    {
        return false;
    }

    // The pending connection can be aborted by the peer, or taken by another thread,
    // between the poll above and the accept() below. Non-blocking mode turns that
    // race into EAGAIN instead of an indefinitely blocked accept().
    setBlockingModeUnlocked(false);

    clientInfoLength = sizeof(clientInfo);
    clientSocketFD = ::accept(getSocketFdUnlocked(), bit_cast<sockaddr*>(&clientInfo), &clientInfoLength);
    if (clientSocketFD != INVALID_SOCKET)
    {
        return true;
    }

    clientInfoLength = 0;
    if (const auto error = getSocketError();
        error == EAGAIN || error == EINTR || error == ECONNABORTED)
    {
        // The pending connection disappeared before it could be accepted.
        return false;
    }

    throwSocketError("Error on accept(). ");
}

size_t TCPSocket::readUnlocked(uint8_t* destination, const size_t size, sockaddr*)
{
    for (;;)
    {
        const auto receivedBytes = recvUnlocked(destination, size);
        if (receivedBytes != RECV_RETRY)
        {
            return receivedBytes;
        }

        if (!activeUnlocked())
        {
            return 0;
        }

        if (!readyToReadUnlocked(500ms))
        {
            throw TimeoutException("Can't read from socket: timeout");
        }
    }
}

void TCPSocket::setProxy(shared_ptr<Proxy> proxy)
{
    m_proxy = std::move(proxy);
}