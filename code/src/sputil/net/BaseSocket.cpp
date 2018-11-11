/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseSocket.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <cerrno>
#include <sptk5/SystemException.h>
#include <sptk5/net/BaseSocket.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
static int   m_socketCount;
static bool  m_inited(false);
#else

#include <sys/poll.h>

#endif

void BaseSocket::throwSocketError(const String& operation, const char* file, int line)
{
    string errorStr;
#ifdef _WIN32
    LPCTSTR lpMsgBuf = nullptr;
    const DWORD dw = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, nullptr );
    if (lpMsgBuf)
        errorStr = lpMsgBuf;
#else
    errorStr = strerror(errno);
#endif
    throw Exception(operation + ": " + errorStr, file, line);
}

#ifdef _WIN32
void BaseSocket::init() noexcept
{
    if (m_inited)
        return;
    m_inited =  true;
	WSADATA  wsaData = {};
    const WORD     wVersionRequested = MAKEWORD (2, 0);
    WSAStartup (wVersionRequested, &wsaData);
}

void BaseSocket::cleanup() noexcept
{
    m_inited =  false;
    WSACleanup();
}
#endif

// Constructor
BaseSocket::BaseSocket(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
{
#ifdef _WIN32
    init();
    m_socketCount++;
#endif
    m_sockfd = INVALID_SOCKET;
    m_domain = domain;
    m_type = type;
    m_protocol = protocol;
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
/*
void BaseSocket::getHostAddress(const string& hostname, sockaddr_in& addr)
{
    memset(&addr, 0, sizeof(addr));

#ifdef _WIN32
    struct hostent* host_info = gethostbyname(hostname.c_str());
    memcpy(&addr.sin_addr, host_info->h_addr, size_t(host_info->h_length));
#else
    struct addrinfo hints = {};
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;          // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Socket type
    //hints.ai_flags = AI_PASSIVE;      // For wildcard IP address
    hints.ai_protocol = 0;

    struct addrinfo* result;
    int rc = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (rc != 0)
        throw Exception(gai_strerror(rc));

    memcpy(&addr, (struct sockaddr_in*) result->ai_addr, result->ai_addrlen);

    freeaddrinfo(result);
#endif
}
*/
void BaseSocket::blockingMode(bool blocking)
{
#ifdef _WIN32
    uint32_t arg = blocking ? 0 : 1;
    control(FIONBIO, &arg);
    u_long arg2 = arg;
    const int rc = ioctlsocket(m_sockfd, FIONBIO, &arg2);
#else
    int flags = fcntl(m_sockfd, F_GETFL);
    if ((flags & O_NONBLOCK) == O_NONBLOCK)
        flags -= O_NONBLOCK;
    if (!blocking)
        flags |= O_NONBLOCK;
    int rc = fcntl(m_sockfd, F_SETFL, flags);
#endif
    if (rc != 0)
        THROW_SOCKET_ERROR("Can't set socket blocking mode");
}

size_t BaseSocket::socketBytes()
{
    uint32_t bytes = 0;
#ifdef _WIN32
    const int32_t rc = ioctlsocket(m_sockfd, FIONREAD, (u_long*) &bytes);
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

int32_t BaseSocket::control(int flag, const uint32_t* check)
{
#ifdef _WIN32
    return ioctlsocket (m_sockfd, flag, (u_long *) check);
#else
    return fcntl(m_sockfd, flag, *check);
#endif
}

void BaseSocket::host(const Host& host)
{
    m_host = host;
}

// Connect & disconnect
void BaseSocket::open_addr(CSocketOpenMode openMode, const sockaddr_in* addr, std::chrono::milliseconds timeout)
{
    auto timeoutMS = (int) timeout.count();

    if (active())
        close();

    // Create a new socket
    m_sockfd = socket(m_domain, m_type, m_protocol);
    if (m_sockfd == INVALID_SOCKET)
        THROW_SOCKET_ERROR("Can't create socket");

    int rc = 0;
    string currentOperation;

    switch (openMode) {
        case SOM_CONNECT:
            currentOperation = "connect";
            if (timeoutMS != 0) {
                blockingMode(false);
                rc = connect(m_sockfd, (sockaddr*) addr, sizeof(sockaddr_in));
                switch (rc) {
                    case ENETUNREACH:
                        throw Exception("Network unreachable");
                    case ECONNREFUSED:
                        throw Exception("Connection refused");
                    default:
                        break;
                }
                try {
                    if (!readyToWrite(timeout))
                        throw Exception("Connection timeout");
                }
                catch (const exception& e) {
                    close();
                    throw;
                }
                rc = 0;
                blockingMode(true);
            } else
                rc = connect(m_sockfd, (sockaddr*) addr, sizeof(sockaddr_in));
            break;
        case SOM_BIND:
            if (m_type != SOCK_DGRAM) {
#ifndef _WIN32
                setOption(SOL_SOCKET, SO_REUSEPORT, 1);
#else
                setOption(SOL_SOCKET, SO_REUSEADDR, 1);
#endif
            }
            currentOperation = "bind";
            rc = ::bind(m_sockfd, (sockaddr*) addr, sizeof(sockaddr_in));
            if (rc == 0 && m_type != SOCK_DGRAM) {
                rc = ::listen(m_sockfd, SOMAXCONN);
                currentOperation = "listen";
            }
            break;
        case SOM_CREATE:
            break;
    }

    if (rc != 0) {
        stringstream error;
        error << "Can't " << currentOperation << " to " << m_host.toString(false) << ". " << SystemException::osError()
              << ".";
        close();
        throw Exception(error.str());
    }
}

void BaseSocket::_open(const Host&, CSocketOpenMode, bool, std::chrono::milliseconds)
{}

void BaseSocket::bind(const char* address, uint32_t portNumber)
{
    if (m_sockfd == INVALID_SOCKET) {
        // Create a new socket
        m_sockfd = socket(m_domain, m_type, m_protocol);
        if (m_sockfd == INVALID_SOCKET)
            THROW_SOCKET_ERROR("Can't create socket");
    }

    sockaddr_in addr = {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = (SOCKET_ADDRESS_FAMILY) m_domain;

    if (address == nullptr)
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        addr.sin_addr.s_addr = inet_addr(address);

    addr.sin_port = htons(uint16_t(portNumber));

    if (::bind(m_sockfd, (sockaddr*) &addr, sizeof(addr)) != 0)
        THROW_SOCKET_ERROR("Can't bind socket to port " + int2string(portNumber));
}

void BaseSocket::listen(uint16_t portNumber)
{
    if (portNumber != 0)
        m_host.port(portNumber);

    sockaddr_in addr = {};

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = (SOCKET_ADDRESS_FAMILY) m_domain;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_host.port());

    open_addr(SOM_BIND, &addr);
}

void BaseSocket::close() noexcept
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

void BaseSocket::attach(SOCKET socketHandle)
{
    close();
    m_sockfd = socketHandle;
}

size_t BaseSocket::read(char* buffer, size_t size, sockaddr_in* from)
{
    int bytes;
    if (from != nullptr) {
        socklen_t flen = sizeof(sockaddr_in);
        bytes = (int) ::recvfrom(m_sockfd, buffer, (int32_t) size, 0, (sockaddr*) from, &flen);
    } else
        bytes = (int) ::recv(m_sockfd, buffer, (int32_t) size, 0);

    if (bytes == -1)
        THROW_SOCKET_ERROR("Can't read from socket");

    return (size_t) bytes;
}

size_t BaseSocket::read(Buffer& buffer, size_t size, sockaddr_in* from)
{
    buffer.checkSize(size);
    size_t bytes = read(buffer.data(), size, from);
    if (bytes != size)
        buffer.bytes(bytes);
    return bytes;
}

size_t BaseSocket::read(String& buffer, size_t size, sockaddr_in* from)
{
    buffer.resize(size);
    size_t bytes = read((char*)buffer.c_str(), size, from);
    if (bytes != size)
        buffer.resize(bytes);
    return bytes;
}

size_t BaseSocket::write(const char* buffer, size_t size, const sockaddr_in* peer)
{
    int bytes;
    const char* p = buffer;

    if ((int)size == -1)
        size = strlen(buffer);

    const size_t total = size;
    auto remaining = (int) size;
    while (remaining > 0) {
        if (peer != nullptr)
            bytes = (int) sendto(m_sockfd, p, (int32_t) size, 0, (sockaddr*) peer, sizeof(sockaddr_in));
        else
            bytes = (int) send(p, (int32_t) size);
        if (bytes == -1)
            THROW_SOCKET_ERROR("Can't write to socket");
        remaining -= bytes;
        p += bytes;
    }
    return total;
}

size_t BaseSocket::write(const Buffer& buffer, const sockaddr_in* peer)
{
    return write(buffer.data(), buffer.bytes(), peer);
}

size_t BaseSocket::write(const String& buffer, const sockaddr_in* peer)
{
    return write(buffer.c_str(), buffer.length(), peer);
}

#if (__FreeBSD__ | __OpenBSD__)
#define CONNCLOSED (POLLHUP)
#else
#define CONNCLOSED (POLLRDHUP|POLLHUP)
#endif

bool BaseSocket::readyToRead(chrono::milliseconds timeout)
{
    const auto timeoutMS = (int) timeout.count();
#ifdef _WIN32
    struct timeval time;
    time.tv_sec = int32_t (timeoutMS) / 1000;
    time.tv_usec = int32_t (timeoutMS) % 1000 * 1000;

    fd_set  inputs, errors;
    FD_ZERO(&inputs);
    FD_ZERO(&errors);
    FD_SET(m_sockfd, &inputs);
    FD_SET(m_sockfd, &errors);

    const int rc = select(FD_SETSIZE, &inputs, nullptr, &errors, &time);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (FD_ISSET(m_sockfd, &errors))
        throw ConnectionException("Connection closed");
#else
    struct pollfd pfd = {};

    pfd.fd = m_sockfd;
    pfd.events = POLLIN;
    int rc = poll(&pfd, 1, timeoutMS);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (rc == 1 && (pfd.revents & CONNCLOSED) != 0)
        throw ConnectionException("Connection closed");
#endif
    return rc != 0;
}

bool BaseSocket::readyToRead(DateTime timeout)
{
    const auto timeoutMS = (int) chrono::duration_cast<chrono::milliseconds>(timeout - DateTime::Now()).count();
#ifdef _WIN32
    struct timeval time;
    time.tv_sec = int32_t (timeoutMS) / 1000;
    time.tv_usec = int32_t (timeoutMS) % 1000 * 1000;

    fd_set  inputs, errors;
    FD_ZERO(&inputs);
    FD_ZERO(&errors);
    FD_SET(m_sockfd, &inputs);
    FD_SET(m_sockfd, &errors);

    const int rc = select(FD_SETSIZE, &inputs, nullptr, &errors, &time);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (FD_ISSET(m_sockfd, &errors))
        throw ConnectionException("Connection closed");
#else
    struct pollfd pfd = {};

    pfd.fd = m_sockfd;
    pfd.events = POLLIN;
    int rc = poll(&pfd, 1, timeoutMS);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (rc == 1 && (pfd.revents & CONNCLOSED) != 0)
        throw ConnectionException("Connection closed");
#endif
    return rc != 0;
}

bool BaseSocket::readyToWrite(std::chrono::milliseconds timeout)
{
    const auto timeoutMS = (int) timeout.count();
#ifdef _WIN32
    struct timeval time;
    time.tv_sec = int32_t (timeoutMS) / 1000;
    time.tv_usec = int32_t (timeoutMS) % 1000 * 1000;

    fd_set  inputs, errors;
    FD_ZERO(&inputs);
    FD_ZERO(&errors);
    FD_SET(m_sockfd, &inputs);
    FD_SET(m_sockfd, &errors);

    const int rc = select(FD_SETSIZE, nullptr, &inputs, &errors, &time);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (FD_ISSET(m_sockfd, &errors))
        THROW_SOCKET_ERROR("Socket closed");
#else
    struct pollfd pfd = {};
    pfd.fd = m_sockfd;
    pfd.events = POLLOUT;
    int rc = poll(&pfd, 1, timeoutMS);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (rc == 1 && (pfd.revents & CONNCLOSED) != 0)
        throw Exception("Connection closed");
#endif
    return rc != 0;
}

bool BaseSocket::readyToWrite(DateTime timeout)
{
	const auto timeoutMS = (int) chrono::duration_cast<chrono::milliseconds>(timeout - DateTime::Now()).count();
#ifdef _WIN32
    struct timeval time;
    time.tv_sec = int32_t (timeoutMS) / 1000;
    time.tv_usec = int32_t (timeoutMS) % 1000 * 1000;

    fd_set  inputs, errors;
    FD_ZERO(&inputs);
    FD_ZERO(&errors);
    FD_SET(m_sockfd, &inputs);
    FD_SET(m_sockfd, &errors);

    const int rc = select(FD_SETSIZE, nullptr, &inputs, &errors, &time);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (FD_ISSET(m_sockfd, &errors))
        THROW_SOCKET_ERROR("Socket closed");
#else
    struct pollfd pfd = {};
    pfd.fd = m_sockfd;
    pfd.events = POLLOUT;
    int rc = poll(&pfd, 1, timeoutMS);
    if (rc < 0)
        THROW_SOCKET_ERROR("Can't read from socket");
    if (rc == 1 && (pfd.revents & CONNCLOSED) != 0)
        throw Exception("Connection closed");
#endif
    return rc != 0;
}

#ifdef _WIN32
# define VALUE_TYPE(val) (char*)(val)
#else
# define VALUE_TYPE(val) (void*)(val)
#endif

void BaseSocket::setOption(int level, int option, int value)
{
    const socklen_t len = sizeof(int);
    if (setsockopt(m_sockfd, level, option, VALUE_TYPE (&value), len) != 0)
        THROW_SOCKET_ERROR("Can't set socket option");
}

void BaseSocket::getOption(int level, int option, int& value)
{
    socklen_t len = sizeof(int);
    if (getsockopt(m_sockfd, level, option, VALUE_TYPE (&value), &len) != 0)
        THROW_SOCKET_ERROR("Can't get socket option");
}
