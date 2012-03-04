/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSocket.cpp  -  description
                             -------------------
    begin                : July 10 2002
    copyright            : (C) 2002-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef _WIN32
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#ifdef __APPLE__
typedef int socklen_t;
#endif

#else
typedef int socklen_t;
#endif

#include <sptk5/CSocket.h>
#include <sptk5/CException.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
int   CSocket::m_socketCount;
bool  CSocket::m_inited = false;
#endif

static void throwSocketError (std::string operation, int line) throw (CException)
{
#ifdef _WIN32
    string errorStr;
    switch (WSAGetLastError()) {
        case WSAEINTR: errorStr = "Interrupted function call";
            break;
        case WSAEACCES: errorStr = "Permission denied";
            break;
        case WSAEFAULT: errorStr = "Bad address";
            break;
        case WSAEINVAL: errorStr = "Invalid argument";
            break;
        case WSAEMFILE: errorStr = "Too many open files";
            break;
        case WSAEWOULDBLOCK: errorStr = "Resource temporarily unavailable";
            break;
        case WSAEINPROGRESS: errorStr = "Operation now in progress";
            break;
        case WSAEALREADY: errorStr = "Operation already in progress";
            break;
        case WSAENOTSOCK: errorStr = "Socket operation on nonsocket";
            break;
        case WSAEDESTADDRREQ: errorStr = "Destination address required";
            break;
        case WSAEMSGSIZE: errorStr = "Message too long";
            break;
        case WSAEPROTOTYPE: errorStr = "Protocol wrong type for socket";
            break;
        case WSAENOPROTOOPT: errorStr = "Bad protocol option";
            break;
        case WSAEPROTONOSUPPORT: errorStr = "Protocol not supported";
            break;
        case WSAESOCKTNOSUPPORT: errorStr = "Socket type not supported";
            break;
        case WSAEOPNOTSUPP: errorStr = "Operation not supported";
            break;
        case WSAEPFNOSUPPORT: errorStr = "Protocol family not supported";
            break;
        case WSAEAFNOSUPPORT: errorStr = "Address family not supported by protocol family";
            break;
        case WSAEADDRINUSE: errorStr = "Address already in use";
            break;
        case WSAEADDRNOTAVAIL: errorStr = "Cannot assign requested address";
            break;
        case WSAENETDOWN: errorStr = "Network is down";
            break;
        case WSAENETUNREACH: errorStr = "Network is unreachable";
            break;
        case WSAENETRESET: errorStr = "Network dropped connection on reset";
            break;
        case WSAECONNABORTED: errorStr = "Software caused connection abort";
            break;
        case WSAECONNRESET: errorStr = "Connection reset by peer";
            break;
        case WSAENOBUFS: errorStr = "No buffer space available";
            break;
        case WSAEISCONN: errorStr = "Socket is already connected";
            break;
        case WSAENOTCONN: errorStr = "Socket is not connected";
            break;
        case WSAESHUTDOWN: errorStr = "Cannot send after socket shutdown";
            break;
        case WSAETIMEDOUT: errorStr = "Connection timed out";
            break;
        case WSAECONNREFUSED: errorStr = "Connection refused";
            break;
        case WSAEHOSTDOWN: errorStr = "Host is down";
            break;
        case WSAEHOSTUNREACH: errorStr = "No route to host";
            break;
        case WSAEPROCLIM: errorStr = "Too many processes";
            break;
        case WSASYSNOTREADY: errorStr = "Network subsystem is unavailable";
            break;
        case WSAVERNOTSUPPORTED: errorStr = "Winsock.dll version out of range";
            break;
        case WSANOTINITIALISED: errorStr = "Successful WSAStartup not yet performed";
            break;
        case WSAEDISCON: errorStr = "Graceful shutdown in progress";
            break;
        case WSATYPE_NOT_FOUND: errorStr = "Class type not found";
            break;
        case WSAHOST_NOT_FOUND: errorStr = "Host not found";
            break;
        case WSATRY_AGAIN: errorStr = "Nonauthoritative host not found";
            break;
        case WSANO_RECOVERY: errorStr = "Nonrecoverable error during a database lookup";
            break;
        case WSANO_DATA: errorStr = "Valid name, no data record of requested type";
            break;
        case WSA_INVALID_HANDLE: errorStr = "Specified event object handle is invalid";
            break;
        case WSA_INVALID_PARAMETER: errorStr = "One or more parameters are invalid";
            break;
        case WSA_IO_INCOMPLETE: errorStr = "Overlapped I/O event object not in signaled state";
            break;
        case WSA_IO_PENDING: errorStr = "Overlapped operations will complete later";
            break;
        case WSA_NOT_ENOUGH_MEMORY: errorStr = "Insufficient memory available";
            break;
        case WSA_OPERATION_ABORTED: errorStr = "Overlapped operation aborted";
            break;
        case WSASYSCALLFAILURE: errorStr = "System call failure";
            break;
    }
#else
    char buffer[256];
    strerror_r (errno, buffer, sizeof (buffer));
    string errorStr (buffer);
#endif
    throw CException (operation + ": " + errorStr, __FILE__, line);
}

CSocketReader::CSocketReader (CSocket& socket, int buffer_size)
        : CBuffer (buffer_size), m_socket (socket)
{
    m_readOffset = 0;
    m_buffer[buffer_size-1] = 0;
}

void CSocketReader::open()
{
    m_readOffset = 0;
    m_bytes = 0;
}

int32_t CSocketReader::bufferedRead (char *dest, uint32_t sz, bool read_line, sockaddr_in* from)
{
    int availableBytes = int (m_bytes - m_readOffset);
    int bytesToRead = (int) sz;
    bool eol = 0;

    if (!availableBytes) {
        m_readOffset = 0;
        if (from) {
            socklen_t flen = sizeof (sockaddr_in);
            m_bytes = recvfrom (m_socket.handle(), m_buffer, int (m_size - 2), 0, (sockaddr*) from, &flen);
        } else {
            m_bytes = m_socket.recv (m_buffer, m_size - 2);
        }
        if (int (m_bytes) == -1)
            throwSocketError ("Can't read from socket", __LINE__);

        availableBytes = int (m_bytes);
        m_buffer[m_bytes] = 0;
        if (!m_bytes)
            return 0;
    }

    char *readPosition = m_buffer + m_readOffset;
    if (availableBytes < bytesToRead)
        bytesToRead = (int) availableBytes;

    if (read_line) {
        char *cr = (char *) strchr (readPosition, '\n');
        if (cr) {
            eol = 1;
            bytesToRead = int (cr - readPosition + 1);
            *cr = 0;
            if (bytesToRead) {
                cr--;
                if (*cr == '\r')
                    *cr = 0;
            }
        }
    }

    // copy data to dest, advance the read offset
    memcpy (dest, readPosition, bytesToRead);

    if (read_line || bytesToRead < int (sz))
        dest[bytesToRead] = 0;

    m_readOffset += bytesToRead;
    if (eol) // Indicate, that we have a complete string
        return -bytesToRead;

    return bytesToRead;
}

uint32_t CSocketReader::read (char *dest, uint32_t sz, bool read_line, sockaddr_in* from)
{
    int total = 0;
    int eol = 0;

    if (m_socket.handle() <= 0)
        throw CException ("Can't read from closed socket", __FILE__, __LINE__);

    while (!eol) {
        int bytesToRead = int (sz - total);
        if (bytesToRead <= 0)
            return sz;

        int bytes = bufferedRead (dest, bytesToRead, read_line, from);

        if (!bytes) // No more data
            break;

        if (bytes < 0) { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += bytes;
        dest += bytes;
    }
    return total - eol;
}

uint32_t CSocketReader::readLine (CBuffer& destBuffer)
{
    int total = 0;
    int eol = 0;

    if (m_socket.handle() <= 0)
        throw CException ("Can't read from closed socket", __FILE__, __LINE__);

    while (!eol) {
        int bytesToRead = int (destBuffer.size() - total);
        if (bytesToRead <= 128) {
            destBuffer.checkSize (destBuffer.size() + 128);
            bytesToRead = int (destBuffer.size() - total - 1);
        }

        char *dest = destBuffer.data() + total;

        int bytes = bufferedRead (dest, bytesToRead, true);

        if (!bytes) // No more data
            break;

        if (bytes < 0) { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += bytes;
    }
    destBuffer.data() [total] = 0;
    destBuffer.bytes (total);
    return destBuffer.bytes();
}

#ifdef _WIN32
void CSocket::init()
{
    if (m_inited)
        return;
    m_inited =  true;
    WSADATA  wsaData;
    WORD     wVersionRequested = MAKEWORD (2, 0);
    WSAStartup (wVersionRequested, &wsaData);
}

void CSocket::cleanup()
{
    m_inited =  false;
    WSACleanup();
}
#endif

// Constructor
CSocket::CSocket (int32_t domain, int32_t type, int32_t protocol)
        : m_reader (*this, 1024)
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

    FD_ZERO (&m_inputs);
    FD_ZERO (&m_outputs);
}

// Destructor
CSocket::~CSocket()
{
    close();
#ifdef _WIN32
    m_socketCount--;
    if (!m_socketCount)
        cleanup();
#endif
}

int32_t CSocket::recv (void* buffer, uint32_t len)
{
    return ::recv(m_sockfd, (char*) buffer, len, 0);
}

int32_t CSocket::send (const void* buffer, uint32_t len)
{
    return ::send(m_sockfd, (char*) buffer, len, 0);
}

int32_t CSocket::control (int flag, uint32_t *check)
{
#ifdef _WIN32
    return ioctlsocket (m_sockfd, flag, (u_long *) check);
#else
    return fcntl (m_sockfd, flag, *check);
#endif
}

void CSocket::host (string hostName)
{
    m_host = hostName;
}

void CSocket::port (int32_t portNumber)
{
    m_port = portNumber;
}

// Connect & disconnect
void CSocket::open_addr (CSocketOpenMode openMode, sockaddr_in* addr)
{
    if (active())
        close();

    // Create a new socket
    m_sockfd = socket (m_domain, m_type, m_protocol);
    if (m_sockfd == INVALID_SOCKET)
        throwSocketError ("Can't create socket", __LINE__);

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
        throw CException ("Can't open: " + currentOperation + "() failed.", __FILE__, __LINE__);
    }

    FD_SET (m_sockfd, &m_inputs);
    FD_SET (m_sockfd, &m_outputs);

    m_reader.open();
}

void CSocket::open (string hostName, int32_t portNumber, CSocketOpenMode openMode)
{
    if (hostName.length())
        m_host = hostName;
    if (!m_host.length())
        throw CException ("Please, define the host name", __FILE__, __LINE__);
    if (portNumber)
        m_port = portNumber;

    sockaddr_in addr;
    struct hostent *host_info;

    host_info = gethostbyname (m_host.c_str());
    if (!host_info)
        throw CException ("Can't connect. Host is unknown.", __FILE__, __LINE__);

    memset (&addr, 0, sizeof (addr));
    addr.sin_family = m_domain;
    memcpy (&addr.sin_addr, host_info->h_addr, host_info->h_length);
    addr.sin_port = htons (m_port);

    if (active())
        close();

    open_addr (openMode, &addr);
}

void CSocket::listen (int32_t portNumber)
{
    if (portNumber)
        m_port = portNumber;

    sockaddr_in addr;

    memset (&addr, 0, sizeof (addr));
    addr.sin_family = m_domain;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons (m_port);

    open_addr (SOM_BIND, &addr);
}

void CSocket::close()
{
    if (m_sockfd != INVALID_SOCKET) {
        m_reader.close();

        FD_CLR (m_sockfd, &m_inputs);
        FD_CLR (m_sockfd, &m_outputs);
#ifndef _WIN32
        ::close (m_sockfd);
#else
        closesocket (m_sockfd);
#endif
        m_sockfd = INVALID_SOCKET;
    }
}

void CSocket::accept (int& clientSocketFD, struct sockaddr_in& clientInfo)
{
    socklen_t len = sizeof (clientInfo);
    clientSocketFD = (int) ::accept (m_sockfd, (struct sockaddr *) & clientInfo, &len);
    if (clientSocketFD < 0)
        throwSocketError ("Error on accept(). ", __LINE__);
}

void CSocket::attach (SOCKET socketHandle)
{
    close();
    m_sockfd = socketHandle;

    FD_SET (m_sockfd, &m_inputs);
    FD_SET (m_sockfd, &m_outputs);

    m_reader.open();
}

char CSocket::getChar()
{
    char ch;
#ifdef _WIN32
    int bytes = ::recv (m_sockfd, &ch, 1, 0);
#else
    int bytes = ::read (m_sockfd, &ch, 1);
#endif
    if (!bytes)
        return 0;
    return ch;
}

uint32_t CSocket::readLine (char *buffer, uint32_t size)
{
    return m_reader.read (buffer, size, true);
}

uint32_t CSocket::readLine (CBuffer& buffer)
{
    return m_reader.readLine (buffer);
}

uint32_t CSocket::readLine (std::string& s)
{
    m_reader.readLine (m_stringBuffer);
    s = m_stringBuffer.data();
    return m_stringBuffer.size() - 1;
}

uint32_t CSocket::read (char *buffer, uint32_t size, sockaddr_in* from)
{
    m_reader.read (buffer, size, false, from);
    return size;
}

uint32_t CSocket::read (CBuffer& buffer, sockaddr_in* from)
{
    uint32_t rc = m_reader.read (buffer.data(), buffer.size(), false, from);
    buffer.bytes (rc);
    return rc;
}

uint32_t CSocket::write (const char *buffer, uint32_t size, const sockaddr_in* peer)
{
    int         bytes;
    const char *p = buffer;
    uint32_t      total = size;
    while (size > 0) {
        if (peer)
            bytes = sendto (m_sockfd, p, int (size), 0, (sockaddr *) peer, sizeof (sockaddr_in));
        else
            bytes = send (p, int (size));
        if (bytes < 0)
            throwSocketError ("Can't write to socket", __LINE__);
        size -= bytes;
        p += bytes;
    }
    return total;
}

uint32_t CSocket::write (const CBuffer& buffer)
{
    char*  ptr = buffer.data();
    uint32_t bytes = buffer.bytes();
    while (bytes > 0) {
        uint32_t rc = write (ptr, bytes);
        bytes -= rc;
        ptr += rc;
    }
    return buffer.bytes();
}

bool CSocket::readyToRead (uint32_t wait_msec)
{
    struct timeval timeout;
    timeout.tv_sec = int32_t (wait_msec) / 1000;
    timeout.tv_usec = int32_t (wait_msec) % 1000 * 1000;

    //FD_ZERO(&inputs);
    FD_SET (m_sockfd, &m_inputs);

    select (FD_SETSIZE, &m_inputs, NULL, NULL, &timeout);
    if (select (FD_SETSIZE, &m_inputs, NULL, NULL, &timeout) < 0)
        throwSocketError ("Can't read from socket", __LINE__);

    return (FD_ISSET (m_sockfd , &m_inputs) > 0);
}

bool CSocket::readyToWrite()
{
    return true;
}

#ifdef _WIN32
# define VALUE_TYPE(val) (char*)(val)
#else
# define VALUE_TYPE(val) (void*)(val)
#endif

void CSocket::setOption (int level, int option, int value) throw (CException)
{
    socklen_t len = sizeof (int);
    if (setsockopt (m_sockfd, level, option, VALUE_TYPE (&value), len))
        throwSocketError ("Can't set socket option", __LINE__);
}

void CSocket::getOption (int level, int option, int& value) throw (CException)
{
    socklen_t len = sizeof (int);
    if (getsockopt (m_sockfd, level, option, VALUE_TYPE (&value), &len))
        throwSocketError ("Can't get socket option", __LINE__);
}

CSocket& CSocket::operator << (const std::string& s)
{
    write (s.c_str(), (int) s.length());
    return *this;
}

CSocket& CSocket::operator >> (std::string& s)
{
    readLine (s);
    return *this;
}

