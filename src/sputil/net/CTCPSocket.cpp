/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTCPSocket.cpp  -  description
                             -------------------
    begin                : July 10 2002
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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

#include <sptk5/net/CTCPSocket.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
    static int   m_socketCount;
    static bool  m_inited(false);
#endif

CTCPSocketReader::CTCPSocketReader(CBaseSocket& socket, size_t buffer_size)
    : CBuffer(buffer_size), m_socket(socket)
{
    m_readOffset = 0;
    m_buffer[buffer_size-1] = 0;
}

void CTCPSocketReader::open()
{
    m_readOffset = 0;
    m_bytes = 0;
}

int32_t CTCPSocketReader::bufferedRead(char *dest, size_t sz, char delimiter, bool read_line, sockaddr_in* from)
{
    int availableBytes = int(m_bytes - m_readOffset);
    int bytesToRead = (int) sz;
    bool eol = 0;

    if (!availableBytes) {
        m_readOffset = 0;
        if (from) {
            socklen_t flen = sizeof(sockaddr_in);
            m_bytes = recvfrom(m_socket.handle(), m_buffer, int32_t(m_size - 2), 0, (sockaddr*) from, &flen);
        } else {
            m_bytes = m_socket.recv(m_buffer, m_size - 2);
        }
        if (int(m_bytes) == -1)
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
        if (delimiter == 0) {
            size_t len = strlen(readPosition);
            eol = m_readOffset + len < m_bytes;
            bytesToRead = (int) len;
            if (eol)
                bytesToRead++;
        } else {
            char *cr = (char *) strchr(readPosition, delimiter);
            if (cr) {
                eol = 1;
                bytesToRead = int(cr - readPosition + 1);
                *cr = 0;
            }
        }
    }

    // copy data to dest, advance the read offset
    memcpy(dest, readPosition, size_t(bytesToRead));

    if (read_line || bytesToRead < int(sz))
        dest[bytesToRead] = 0;

    m_readOffset += uint32_t(bytesToRead);
    if (eol) // Indicate, that we have a complete string
        return -bytesToRead;

    return bytesToRead;
}

size_t CTCPSocketReader::read(char *dest, size_t sz, char delimiter, bool read_line, sockaddr_in* from)
{
    int total = 0;
    int eol = 0;

    if (m_socket.handle() <= 0)
        throw CException("Can't read from closed socket", __FILE__, __LINE__);

    while (!eol) {
        int bytesToRead = int(int(sz) - total);
        if (bytesToRead <= 0)
            return sz;

        int bytes = bufferedRead(dest, size_t(bytesToRead), delimiter, read_line, from);

        if (!bytes) // No more data
            break;

        if (bytes < 0) { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += bytes;
        dest += bytes;
    }
    return size_t(total - eol);
}

size_t CTCPSocketReader::readLine(CBuffer& destBuffer, char delimiter)
{
    size_t total = 0;
    int eol = 0;

    if (m_socket.handle() <= 0)
        throw CException("Can't read from closed socket", __FILE__, __LINE__);

    while (!eol) {
        int bytesToRead = int(destBuffer.size() - total);
        if (bytesToRead <= 128) {
            destBuffer.checkSize(destBuffer.size() + 128);
            bytesToRead = int(destBuffer.size() - total - 1);
        }

        char *dest = destBuffer.data() + total;

        int bytes = bufferedRead(dest, size_t(bytesToRead), delimiter, true);

        if (!bytes) // No more data
            break;

        if (bytes < 0) { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += size_t(bytes);
    }
    destBuffer.data()[total] = 0;
    destBuffer.bytes(total);
    return destBuffer.bytes();
}

// Constructor
CTCPSocket::CTCPSocket(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
        : CBaseSocket(domain, type, protocol), m_reader(*this, 65536)
{
}

// Destructor
CTCPSocket::~CTCPSocket()
{
}

void CTCPSocket::open(string hostName, uint32_t portNumber, CSocketOpenMode openMode) THROWS_EXCEPTIONS
{
    if (hostName.length())
        m_host = hostName;
    if (!m_host.length())
        throw CException("Please, define the host name", __FILE__, __LINE__);
    if (portNumber)
        m_port = portNumber;

    sockaddr_in addr;
    struct hostent *host_info;

    host_info = gethostbyname(m_host.c_str());
    if (!host_info)
        throw CException("Can't connect. Host is unknown.", __FILE__, __LINE__);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = (SOCKET_ADDRESS_FAMILY) m_domain;
    memcpy(&addr.sin_addr, host_info->h_addr, size_t(host_info->h_length));
    addr.sin_port = htons(uint16_t(m_port));

    if (active())
        close();

    open_addr(openMode, &addr);
    m_reader.open();
}

void CTCPSocket::accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo)
{
    socklen_t len = sizeof(clientInfo);
    clientSocketFD = (int) ::accept(m_sockfd, (struct sockaddr *) & clientInfo, &len);
    if (clientSocketFD < 0)
        THROW_SOCKET_ERROR("Error on accept(). ");
}

char CTCPSocket::getChar()
{
    char ch;
#ifdef _WIN32
    int bytes = ::recv(m_sockfd, &ch, 1, 0);
#else
    ssize_t bytes = ::read(m_sockfd, &ch, 1);
#endif
    if (!bytes)
        return 0;
    return ch;
}

size_t CTCPSocket::readLine(char *buffer, size_t size, char delimiter)
{
    return m_reader.read(buffer, size, delimiter, true);
}

size_t CTCPSocket::readLine(CBuffer& buffer, char delimiter)
{
    return m_reader.readLine(buffer, delimiter);
}

size_t CTCPSocket::readLine(std::string& s, char delimiter)
{
    m_reader.readLine(m_stringBuffer, delimiter);
    s = m_stringBuffer.data();
    return m_stringBuffer.size() - 1;
}

size_t CTCPSocket::read(char *buffer, size_t size, sockaddr_in* from) THROWS_EXCEPTIONS
{
    m_reader.read(buffer, size, 0, false, from);
    return size;
}

size_t CTCPSocket::read(CBuffer& buffer, size_t size, sockaddr_in* from) THROWS_EXCEPTIONS
{
    buffer.checkSize(size);
    size_t rc = m_reader.read(buffer.data(), size, 0, false, from);
    buffer.bytes(rc);
    return rc;
}

size_t CTCPSocket::read(string& buffer, size_t size, sockaddr_in* from) THROWS_EXCEPTIONS
{
    buffer.resize(size);
    size_t rc = m_reader.read((char*)buffer.c_str(), size, 0, false, from);
    buffer.resize(rc);
    return rc;
}
