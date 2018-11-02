/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPSocket.cpp - description                            ║
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

#include <cstdio>
#include <cstdlib>
#include <sptk5/net/TCPSocket.h>
#include <thread>

using namespace std;
using namespace sptk;

#ifdef _WIN32
    static int   m_socketCount;
    static bool  m_inited(false);
#endif

TCPSocketReader::TCPSocketReader(BaseSocket& socket, size_t buffer_size)
    : Buffer(buffer_size), m_socket(socket)
{
    m_readOffset = 0;
    m_buffer[buffer_size-1] = 0;
}

void TCPSocketReader::open()
{
    m_readOffset = 0;
    m_bytes = 0;
}

void TCPSocketReader::close() noexcept
{
	try {
		reset(1024);
		m_readOffset = 0;
		m_bytes = 0;
	}
	catch (...) {}
}

int32_t TCPSocketReader::bufferedRead(char *destination, size_t sz, char delimiter, bool read_line, sockaddr_in* from)
{
    auto availableBytes = int(m_bytes - m_readOffset);
    auto bytesToRead = (int) sz;
    bool eol = false;

    if (availableBytes == 0) {
        m_readOffset = 0;
        int error;
        do {
            error = 0;
            if (from != nullptr) {
#ifdef _WIN32
                int flen = sizeof(sockaddr_in);
#else
                socklen_t flen = sizeof(sockaddr_in);
#endif
                m_bytes = (size_t) recvfrom(m_socket.handle(), m_buffer, m_capacity - 2, 0, (sockaddr*) from, &flen);
            } else {
                m_bytes = m_socket.recv(m_buffer, m_capacity - 2);
            }
            if ((int)m_bytes == -1) {
                error = errno;
                if (error == EAGAIN) {
                    if (!m_socket.readyToRead(chrono::seconds(1)))
                        throw TimeoutException("Can't read from socket: timeout");
                } else
                    THROW_SOCKET_ERROR("Can't read from socket");
            }
        } while (error == EAGAIN);

        availableBytes = int (m_bytes);
        m_buffer[m_bytes] = 0;
        if (m_bytes == 0)
            return 0;
    }

    char *readPosition = m_buffer + m_readOffset;
    if (availableBytes < bytesToRead)
        bytesToRead = availableBytes;

    char *cr = nullptr;
    if (read_line) {
        size_t len;
        if (delimiter == 0)
            len = strlen(readPosition);
        else {
            cr = strchr(readPosition, delimiter);
            if (cr != nullptr)
                len = cr - readPosition + 1;
            else {
                if (m_readOffset != 0) {
                    memmove(m_buffer, m_buffer + m_readOffset, (size_t) availableBytes);
                    m_readOffset = 0;
                    m_bytes = (size_t) availableBytes;
                } else {
                    checkSize(m_capacity + 128);
                }
                size_t bytes = m_socket.recv(m_buffer + availableBytes, m_capacity - availableBytes);
                m_bytes += bytes;
                return 0;
            }
        }
        if (len < sz) {
            eol = true;
            bytesToRead = (int) len;
            if (delimiter == 0)
                bytesToRead++;
            if (cr != nullptr)
                *cr = 0;
        }
    }

    // copy data to destination, advance the read offset
    memcpy(destination, readPosition, size_t(bytesToRead));

    if (read_line || bytesToRead < int(sz))
        destination[bytesToRead] = 0;

    m_readOffset += uint32_t(bytesToRead);
    if (eol) // Indicate, that we have a complete string
        return -bytesToRead;

    return bytesToRead;
}

size_t TCPSocketReader::read(char* destination, size_t sz, char delimiter, bool read_line, sockaddr_in* from)
{
    int total = 0;
    int eol = 0;

    if (m_socket.handle() <= 0)
        throw Exception("Can't read from closed socket", __FILE__, __LINE__);

    while (eol == 0) {
        int bytesToRead = int(sz) - total;
        if (bytesToRead <= 0)
            return sz;

        int bytes = bufferedRead(destination, size_t(bytesToRead), delimiter, read_line, from);

        if (bytes == 0) // No more data
            break;

        if (bytes < 0) { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += bytes;
        destination += bytes;
    }
    return size_t(total - eol);
}

size_t TCPSocketReader::availableBytes() const
{
    return m_bytes - m_readOffset;
}

size_t TCPSocketReader::readLine(Buffer& destinationBuffer, char delimiter)
{
    size_t total = 0;
    int eol = 0;

    if (m_socket.handle() <= 0)
        throw Exception("Can't read from closed socket", __FILE__, __LINE__);

    while (eol == 0) {
        auto bytesToRead = int(destinationBuffer.capacity() - total);
        if (bytesToRead <= 128) {
            destinationBuffer.checkSize(destinationBuffer.capacity() + 128);
            bytesToRead = int(destinationBuffer.capacity() - total - 1);
        }

        char *destination = destinationBuffer.data() + total;

        int bytes = bufferedRead(destination, size_t(bytesToRead), delimiter, true);

        if (bytes == 0) // No more data
            break;

        if (bytes < 0) { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += size_t(bytes);
    }
    destinationBuffer.data()[total] = 0;
    destinationBuffer.bytes(total);
    return destinationBuffer.bytes();
}

// Constructor
TCPSocket::TCPSocket(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
: BaseSocket(domain, type, protocol), m_reader(*this, 65536)
{
}

void TCPSocket::_open(const Host& host, CSocketOpenMode openMode, bool _blockingMode, std::chrono::milliseconds timeout)
{
    if (!host.hostname().empty())
        m_host = host;
    if (m_host.hostname().empty())
        throw Exception("Please, define the host name", __FILE__, __LINE__);

    sockaddr_in address = {};
    m_host.getAddress(address);

    _open(address, openMode, _blockingMode, timeout);
}

void TCPSocket::_open(const struct sockaddr_in& address, CSocketOpenMode openMode, bool _blockingMode,
                      chrono::milliseconds timeoutMS)
{
    open_addr(openMode, &address, timeoutMS);
    m_reader.open();

    if (!_blockingMode)
        blockingMode(false);
}

void TCPSocket::close() noexcept
{
    BaseSocket::close();
    m_reader.close();
}

void TCPSocket::accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo)
{
    socklen_t len = sizeof(clientInfo);
    clientSocketFD = ::accept(m_sockfd, (struct sockaddr *) & clientInfo, &len);
    if (clientSocketFD < 0)
        THROW_SOCKET_ERROR("Error on accept(). ");
}

size_t TCPSocket::socketBytes()
{
    if (m_reader.availableBytes() > 0)
        return m_reader.availableBytes();
    return BaseSocket::socketBytes();
}

bool TCPSocket::readyToRead(chrono::milliseconds timeout)
{
    return m_reader.availableBytes() > 0 || BaseSocket::readyToRead(timeout);
}

bool TCPSocket::readyToRead(DateTime timeout)
{
    return m_reader.availableBytes() > 0 || BaseSocket::readyToRead(timeout);
}

size_t TCPSocket::readLine(char *buffer, size_t size, char delimiter)
{
    return m_reader.read(buffer, size, delimiter, true);
}

size_t TCPSocket::readLine(Buffer& buffer, char delimiter)
{
    return m_reader.readLine(buffer, delimiter);
}

size_t TCPSocket::readLine(std::string& s, char delimiter)
{
    m_reader.readLine(m_stringBuffer, delimiter);
    if (m_stringBuffer.empty())
        s = "";
    else
        s.assign(m_stringBuffer.c_str(),m_stringBuffer.bytes() - 1);
    return m_stringBuffer.bytes();
}

size_t TCPSocket::read(char *buffer, size_t size, sockaddr_in* from)
{
    m_reader.read(buffer, size, 0, false, from);
    return size;
}

size_t TCPSocket::read(Buffer& buffer, size_t size, sockaddr_in* from)
{
    buffer.checkSize(size);
    size_t rc = m_reader.read(buffer.data(), size, 0, false, from);
    buffer.bytes(rc);
    return rc;
}

size_t TCPSocket::read(String& buffer, size_t size, sockaddr_in* from)
{
    buffer.resize(size);
    size_t rc = m_reader.read((char*)buffer.c_str(), size, 0, false, from);
    buffer.resize(rc);
    return rc;
}
