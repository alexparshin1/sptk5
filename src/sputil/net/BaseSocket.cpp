/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseSocket.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sptk5/net/BaseSocket.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
    static int   m_socketCount;
    static bool  m_inited(false);
#endif

void BaseSocket::throwSocketError (std::string operation, const char* file, int line) THROWS_EXCEPTIONS
{
    string errorStr;
#ifdef _WIN32
    LPCTSTR lpMsgBuf = 0;
    DWORD dw = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
    if (lpMsgBuf)
        errorStr = lpMsgBuf;
#else
    errorStr = strerror(errno);
#endif
    throw Exception (operation + ": " + errorStr, file, line);
}

#ifdef _WIN32
void BaseSocket::init()
{
    if (m_inited)
        return;
    m_inited =  true;
    WSADATA  wsaData;
    WORD     wVersionRequested = MAKEWORD (2, 0);
    WSAStartup (wVersionRequested, &wsaData);
}

void BaseSocket::cleanup()
{
    m_inited =  false;
    WSACleanup();
}
#endif

// Constructor
BaseSocket::BaseSocket (SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
{
#ifdef _WIN32
    init();
    m_socketCount++;
#endif
    m_sockfd   = INVALID_SOCKET;
    m_domain   = domain;
    m_type     = type;
    m_protocol = protocol;
    m_port     = 0;
}

// Destructor
BaseSocket::~BaseSocket()
{
    close();
#ifdef _WIN32
    m_socketCount--;
    if (!m_socketCount)
        cleanup();
#endif
}

void BaseSocket::blockingMode(bool blocking) THROWS_EXCEPTIONS
{
#ifdef _WIN32
    uint32_t arg = blocking ? 0 : 1;
    control(FIONBIO, &arg);
    u_long arg2 = arg;
    int rc = ioctlsocket(m_sockfd, FIONBIO, &arg2);
#else
    int flags = fcntl(m_sockfd, F_GETFL);
    if (flags & O_NONBLOCK)
        flags -= O_NONBLOCK;
    if (!blocking)
        flags |= O_NONBLOCK;
    int rc = fcntl(m_sockfd, F_SETFL, flags);
#endif
    if (rc)
        THROW_SOCKET_ERROR("Can't set socket blocking mode");
}

uint32_t BaseSocket::socketBytes()
{
    uint32_t bytes = 0;
#ifdef _WIN32
    int32_t rc = ioctlsocket(m_sockfd, FIONREAD, (u_long*) &bytes);
#else
    int32_t rc = ioctl(m_sockfd, FIONREAD, &bytes);
#endif
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't get socket bytes");

    return bytes;
}

size_t BaseSocket::recv(void* buffer, size_t len)
{
    return (size_t) ::recv(m_sockfd, (char*) buffer, (int32_t) len, 0);
}

size_t BaseSocket::send(const void* buffer, size_t len)
{
    return (size_t) ::send(m_sockfd, (char*) buffer, (int32_t) len, 0);
}

int32_t BaseSocket::control (int flag, uint32_t *check)
{
#ifdef _WIN32
    return ioctlsocket (m_sockfd, flag, (u_long *) check);
#else
    return fcntl (m_sockfd, flag, *check);
#endif
}

void BaseSocket::host (string hostName)
{
    m_host = hostName;
}

void BaseSocket::port (int32_t portNumber)
{
    m_port = (uint32_t) portNumber;
}

// Connect & disconnect
void BaseSocket::open_addr (CSocketOpenMode openMode, sockaddr_in* addr)
{
    if (active())
        close();

    // Create a new socket
    m_sockfd = socket (m_domain, m_type, m_protocol);
    if (m_sockfd == INVALID_SOCKET)
        THROW_SOCKET_ERROR("Can't create socket");

    int rc = 0;
    string currentOperation;

    switch (openMode) {
        case SOM_CONNECT:
            rc = connect (m_sockfd, (sockaddr *) addr, sizeof (sockaddr_in));
            currentOperation = "connect";
            break;
        case SOM_BIND:
            rc = bind (m_sockfd, (sockaddr *) addr, sizeof (sockaddr_in));
            currentOperation = "bind";
            if (!rc && m_type != SOCK_DGRAM) {
                rc = ::listen (m_sockfd, SOMAXCONN);
                currentOperation = "listen";
            }
            break;
        case SOM_CREATE:
            break;
    }

    if (rc) {
        close();
        throw Exception ("Can't open: " + currentOperation + "() failed.", __FILE__, __LINE__);
    }
}

void BaseSocket::listen (uint32_t portNumber)
{
    if (portNumber)
        m_port = portNumber;

    sockaddr_in addr;

    memset (&addr, 0, sizeof (addr));
    addr.sin_family = (SOCKET_ADDRESS_FAMILY)m_domain;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons(uint16_t(m_port));

    open_addr (SOM_BIND, &addr);
}

void BaseSocket::close()
{
    if (m_sockfd != INVALID_SOCKET) {
#ifndef _WIN32
        shutdown(m_sockfd, SHUT_RDWR);
        ::close(m_sockfd);
#else
        closesocket (m_sockfd);
#endif
        m_sockfd = INVALID_SOCKET;
    }
}

void BaseSocket::attach (SOCKET socketHandle)
{
    close();
    m_sockfd = socketHandle;
}

size_t BaseSocket::read(char *buffer,size_t size,sockaddr_in* from) THROWS_EXCEPTIONS
{
    int bytes;
    if (from) {
        socklen_t flen = sizeof (sockaddr_in);
        bytes = ::recvfrom(m_sockfd, buffer, (int32_t) size, 0, (sockaddr*) from, &flen);
    } else
        bytes = ::recv(m_sockfd, (char*) buffer, (int32_t) size, 0);

    if (bytes == -1)
        THROW_SOCKET_ERROR("Can't read from socket");
    
    return (size_t) bytes;
}

size_t BaseSocket::read(CBuffer& buffer,size_t size,sockaddr_in* from) THROWS_EXCEPTIONS
{
    buffer.checkSize(size);
    size_t bytes = read(buffer.data(),size,from);
    if (bytes != size)
        buffer.bytes(bytes);
    return bytes;
}

size_t BaseSocket::read(std::string& buffer,size_t size,sockaddr_in* from) THROWS_EXCEPTIONS
{
    buffer.resize(size);
    size_t bytes = read((char*)buffer.data(),size,from);
    if (bytes != size)
        buffer.resize(bytes);
    return bytes;
}

size_t BaseSocket::write(const char *buffer, size_t size, const sockaddr_in* peer) THROWS_EXCEPTIONS
{
    int         bytes;
    const char* p = buffer;

    if (int(size) == -1)
        size = strlen(buffer);

    size_t  total = size;
    int remaining = (int) size;
    while (remaining > 0) {
        if (peer)
            bytes = sendto(m_sockfd, p, (int32_t) size, 0, (sockaddr *) peer, sizeof(sockaddr_in));
        else
            bytes = (int) send(p, (int32_t) size);
        if (bytes == -1)
            THROW_SOCKET_ERROR("Can't write to socket");
        remaining -= bytes;
        p += bytes;
    }
    return total;
}

size_t BaseSocket::write (const CBuffer& buffer, const sockaddr_in* peer) THROWS_EXCEPTIONS
{
    return write(buffer.data(), buffer.bytes(), peer);
}

size_t BaseSocket::write (const std::string& buffer, const sockaddr_in* peer) THROWS_EXCEPTIONS
{
    return write(buffer.c_str(), buffer.length(), peer);
}

bool BaseSocket::readyToRead (size_t wait_msec)
{
    struct timeval timeout;
    timeout.tv_sec = int32_t (wait_msec) / 1000;
    timeout.tv_usec = int32_t (wait_msec) % 1000 * 1000;

    fd_set  inputs, errors;
    FD_ZERO(&inputs);
    FD_ZERO(&errors);
    FD_SET(m_sockfd, &inputs);
    FD_SET(m_sockfd, &errors);

    int rc = select(FD_SETSIZE, &inputs, NULL, &errors, &timeout);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (FD_ISSET(m_sockfd, &errors))
        THROW_SOCKET_ERROR("Socket closed");
    return FD_ISSET(m_sockfd, &inputs) != 0;
}

bool BaseSocket::readyToWrite()
{
    return true;
}

#ifdef _WIN32
# define VALUE_TYPE(val) (char*)(val)
#else
# define VALUE_TYPE(val) (void*)(val)
#endif

void BaseSocket::setOption (int level, int option, int value) THROWS_EXCEPTIONS
{
    socklen_t len = sizeof (int);
    if (setsockopt (m_sockfd, level, option, VALUE_TYPE (&value), len))
        THROW_SOCKET_ERROR("Can't set socket option");
}

void BaseSocket::getOption (int level, int option, int& value) THROWS_EXCEPTIONS
{
    socklen_t len = sizeof (int);
    if (getsockopt (m_sockfd, level, option, VALUE_TYPE (&value), &len))
        THROW_SOCKET_ERROR("Can't get socket option");
}
