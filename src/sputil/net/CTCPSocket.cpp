/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTCPSocket.cpp  -  description
                             -------------------
    begin                : July 10 2002
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#include <sptk5/net/CTCPSocket.h>
#include <sptk5/CException.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
    static int   m_socketCount;
    static bool  m_inited(false);
#endif

CTCPSocketReader::CTCPSocketReader (CBaseSocket& socket, int buffer_size)
    : CBuffer (buffer_size), m_socket (socket)
{
    m_readOffset = 0;
    m_buffer[buffer_size-1] = 0;
}

void CTCPSocketReader::open()
{
    m_readOffset = 0;
    m_bytes = 0;
}

int32_t CTCPSocketReader::bufferedRead (char *dest, uint32_t sz, bool read_line, sockaddr_in* from)
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
            THROW_SOCKET_ERROR("Can't read from socket");

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

uint32_t CTCPSocketReader::read (char *dest, uint32_t sz, bool read_line, sockaddr_in* from)
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

uint32_t CTCPSocketReader::readLine (CBuffer& destBuffer)
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

// Constructor
CTCPSocket::CTCPSocket (SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
        : CBaseSocket(domain, type, protocol), m_reader (*this, 1024)
{
}

// Destructor
CTCPSocket::~CTCPSocket()
{
}

void CTCPSocket::open (string hostName, uint32_t portNumber, CSocketOpenMode openMode)
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
    m_reader.open();
}

void CTCPSocket::listen (uint32_t portNumber)
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

void CTCPSocket::accept (SOCKET& clientSocketFD, struct sockaddr_in& clientInfo)
{
    socklen_t len = sizeof (clientInfo);
    clientSocketFD = (int) ::accept (m_sockfd, (struct sockaddr *) & clientInfo, &len);
    if (clientSocketFD < 0)
        THROW_SOCKET_ERROR("Error on accept(). ");
}

char CTCPSocket::getChar()
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

uint32_t CTCPSocket::readLine (char *buffer, uint32_t size)
{
    return m_reader.read (buffer, size, true);
}

uint32_t CTCPSocket::readLine (CBuffer& buffer)
{
    return m_reader.readLine (buffer);
}

uint32_t CTCPSocket::readLine (std::string& s)
{
    m_reader.readLine (m_stringBuffer);
    s = m_stringBuffer.data();
    return m_stringBuffer.size() - 1;
}

uint32_t CTCPSocket::read (char *buffer, uint32_t size, sockaddr_in* from)
{
    m_reader.read (buffer, size, false, from);
    return size;
}

uint32_t CTCPSocket::read (CBuffer& buffer, uint32_t size, sockaddr_in* from)
{
    buffer.checkSize(size);
    uint32_t rc = m_reader.read (buffer.data(), size, false, from);
    buffer.bytes (rc);
    return rc;
}

uint32_t CTCPSocket::read (string& buffer, uint32_t size, sockaddr_in* from)
{
    buffer.resize(size);
    uint32_t rc = m_reader.read ((char*)buffer.c_str(), size, false, from);
    buffer.resize(rc);
    return rc;
}
