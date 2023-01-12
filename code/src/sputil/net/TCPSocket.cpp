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

#include <cstdio>
#include <cstdlib>
#include <sptk5/cutils>
#include <sptk5/net/TCPSocket.h>
#include <thread>

using namespace std;
using namespace sptk;

#ifdef _WIN32
static int m_socketCount;
static bool m_inited(false);
#endif

void TCPSocket::handleReadFromSocketError(int error)
{
    if (error == EAGAIN)
    {
        if (!readyToRead(chrono::seconds(1)))
        {
            throw TimeoutException("Can't read from socket: timeout");
        }
    }
    else
    {
        throw SystemException("Can't read from socket");
    }
}

TCPSocket::TCPSocket(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
    : BaseSocket(domain, type, protocol)
{
}

TCPSocket::~TCPSocket()
{
    TCPSocket::close();
}

void TCPSocket::_open(const Host& _host, OpenMode openMode, bool _blockingMode, std::chrono::milliseconds timeout)
{
    if (!_host.hostname().empty())
    {
        host(_host);
    }

    if (host().hostname().empty())
    {
        throw Exception("Please, define the host name", __FILE__, __LINE__);
    }

    if (proxy() != nullptr)
    {
        SOCKET fd = proxy()->connect(host(), _blockingMode, timeout);
        attach(fd, false);
    }
    else
    {
        sockaddr_in addr = {};
        host().getAddress(addr);

        _open(addr, openMode, _blockingMode, timeout);
    }
}

void TCPSocket::_open(const struct sockaddr_in& address, OpenMode openMode, bool _blockingMode,
                      chrono::milliseconds timeoutMS)
{
    open_addr(openMode, &address, timeoutMS);

    if (!_blockingMode)
    {
        blockingMode(false);
    }
}

bool TCPSocket::accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo, std::chrono::milliseconds timeout)
{
    socklen_t len = sizeof(clientInfo);
    if (!blockingMode())
    {
        clientSocketFD = ::accept(fd(), (struct sockaddr*) &clientInfo, &len);
        if (clientSocketFD > 0)
        {
            return true;
        }
        if (clientSocketFD == INVALID_SOCKET)
        {
            THROW_SOCKET_ERROR("Error on accept(). ");
        }
    }

    if (readyToRead(timeout))
    {
        clientSocketFD = ::accept(fd(), (struct sockaddr*) &clientInfo, &len);
        if (clientSocketFD > 0)
        {
            return true;
        }
        if (clientSocketFD == INVALID_SOCKET)
            THROW_SOCKET_ERROR("Error on accept(). ");
    }

    return false;
}

size_t TCPSocket::socketBytes()
{
    return BaseSocket::socketBytes();
}

bool TCPSocket::readyToRead(chrono::milliseconds timeout)
{
    return BaseSocket::readyToRead(timeout);
}

size_t TCPSocket::read(uint8_t* destination, size_t size, sockaddr_in* from)
{
    int receivedBytes;
    int error {0};
    do
    {
        error = 0;
        if (from != nullptr)
        {
#ifdef _WIN32
            int flen = sizeof(sockaddr_in);
#else
            socklen_t flen = sizeof(sockaddr_in);
#endif
            receivedBytes = (int) recvfrom(fd(), (char*) destination, (int) size, 0, (sockaddr*) from,
                                           &flen);
        }
        else
        {
            receivedBytes = (int) recv(destination, size);
        }

        if (receivedBytes == -1)
        {
            receivedBytes = 0;
            error = errno;
            handleReadFromSocketError(error);
        }
    } while (error == EAGAIN);

    return (int32_t) receivedBytes;
}

size_t TCPSocket::read(Buffer& buffer, size_t size, sockaddr_in* from)
{
    buffer.checkSize(size);
    size_t bytes = read(buffer.data(), size, from);
    buffer.bytes(bytes);
    return bytes;
}

size_t TCPSocket::read(String& buffer, size_t size, sockaddr_in* from)
{
    buffer.resize(size);
    size_t bytes = read((uint8_t*) buffer.data(), size, from);
    buffer.resize(bytes);
    return bytes;
}

void TCPSocket::setProxy(shared_ptr<Proxy> proxy)
{
    m_proxy = move(proxy);
}
