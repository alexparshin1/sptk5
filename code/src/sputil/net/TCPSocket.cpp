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

void TCPSocket::handleReadFromSocketErrorUnlocked(const int error)
{
    if (error == EAGAIN || error == EINTR || error == EINPROGRESS)
    {
        if (!readyToReadUnlocked(500ms))
        {
            throw TimeoutException("Can't read from socket: timeout");
        }
    }
    else
    {
        throw SystemException("Can't read from socket");
    }
}

TCPSocket::TCPSocket(const SOCKET_ADDRESS_FAMILY domain, const int32_t type, const int32_t protocol)
    : Socket(domain, type, protocol)
{
}

TCPSocket::~TCPSocket()
{
    close();
}

void TCPSocket::openUnlocked(const Host& host, const OpenMode openMode, const bool _blockingMode,
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

void TCPSocket::openUnlocked(const sockaddr_in& address, const OpenMode openMode, const bool _blockingMode,
                             const chrono::milliseconds& timeoutMS, const char* clientBindAddress)
{
    openAddressUnlocked(address, openMode, timeoutMS, true, clientBindAddress);

    if (!_blockingMode)
    {
        setBlockingModeUnlocked(false);
    }
}

bool TCPSocket::accept(SocketType& clientSocketFD, sockaddr_in& clientInfo, const chrono::milliseconds& timeout)
{
    socklen_t len = sizeof(clientInfo);
    if (readyToRead(timeout))
    {
        const ReadLock lock(getMutex());
        clientSocketFD = ::accept(fd(), bit_cast<sockaddr*>(&clientInfo), &len);
        if (clientSocketFD != INVALID_SOCKET)
        {
            return true;
        }
        throwSocketError("Error on accept(). ");
    }

    return false;
}

size_t TCPSocket::readUnlocked(uint8_t* destination, const size_t size, sockaddr*)
{
    int  receivedBytes;
    auto error = 0;
    do
    {
        receivedBytes = static_cast<int>(recvUnlocked(destination, size));

        if (receivedBytes == -1)
        {
            receivedBytes = 0;
            if (!activeUnlocked())
            {
                break;
            }
            error = getSocketError();
            handleReadFromSocketErrorUnlocked(error);
        }
    } while (error == EAGAIN || error == EINTR || error == EINPROGRESS);

    return receivedBytes;
}

void TCPSocket::setProxy(shared_ptr<Proxy> proxy)
{
    m_proxy = std::move(proxy);
}
