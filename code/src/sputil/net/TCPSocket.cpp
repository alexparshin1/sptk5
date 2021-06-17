/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

#ifdef _WIN32
    static int   m_socketCount;
    static bool  m_inited(false);
#endif

TCPSocketReader::TCPSocketReader(BaseSocket& socket, size_t buffer_size)
: Buffer(buffer_size),
  m_socket(socket)
{}

void TCPSocketReader::open()
{
    m_readOffset = 0;
    bytes(0);
}

void TCPSocketReader::close() noexcept
{
	try {
		reset(1024);
		m_readOffset = 0;
		bytes(0);
	}
	catch (const Exception& e) {
	    CERR(e.what() << endl)
	}
}

void TCPSocketReader::handleReadFromSocketError(int error)
{
    if (error == EAGAIN) {
        if (!m_socket.readyToRead(chrono::seconds(1)))
            throw TimeoutException("Can't read from socket: timeout");
    } else
        throw SystemException("Can't read from socket");
}

int32_t TCPSocketReader::readFromSocket(sockaddr_in* from)
{
    m_readOffset = 0;
    int error;
    do {
        error = 0;
        int receivedBytes;
        if (from != nullptr) {
#ifdef _WIN32
            int flen = sizeof(sockaddr_in);
#else
            socklen_t flen = sizeof(sockaddr_in);
#endif
            receivedBytes = (int) recvfrom(m_socket.fd(), data(), (int) capacity() - 2, 0, (sockaddr*) from, &flen);
        } else
            receivedBytes = (int) m_socket.recv((uint8_t*) data(), capacity() - 2);

        if (receivedBytes == -1) {
            bytes(0);
            error = errno;
            handleReadFromSocketError(error);
        } else
            bytes((size_t)receivedBytes);
    } while (error == EAGAIN);

    data()[bytes()] = 0;

    return (int32_t) bytes();
}

void TCPSocketReader::readMoreFromSocket(int availableBytes)
{
    if (m_readOffset != 0) {
        memmove(data(), data() + m_readOffset, (size_t) availableBytes);
        m_readOffset = 0;
        bytes((size_t) availableBytes);
    } else
        checkSize(capacity() + 128);
    size_t receivedBytes = m_socket.recv((uint8_t*) data() + availableBytes, capacity() - availableBytes);
    bytes(bytes() + receivedBytes);
}

int32_t TCPSocketReader::bufferedRead(char *destination, size_t sz, char delimiter, bool read_line, sockaddr_in* from)
{
    auto availableBytes = int(bytes() - m_readOffset);
    auto bytesToRead = (int) sz;
    bool eol = false;

    if (availableBytes == 0) {
        availableBytes = readFromSocket(from);
        if (empty())
            return 0;
    }

    char *readPosition = data() + m_readOffset;
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
                readMoreFromSocket(availableBytes);
                return 0;
            }
        }
        if (len < sz) {
            eol = true;
            bytesToRead = (int) len;
            if (delimiter == 0)
                ++bytesToRead;
            if (cr != nullptr)
                *cr = 0;
        }
    }

    // copy data to destination, advance the read offset
    memcpy(destination, readPosition, size_t(bytesToRead));

    if (read_line || bytesToRead < int(sz))
        destination[bytesToRead] = 0;

    m_readOffset += uint32_t(bytesToRead);
    return eol ? -bytesToRead : bytesToRead;
}

size_t TCPSocketReader::read(char* destination, size_t sz, char delimiter, bool read_line, sockaddr_in* from)
{
    int total = 0;
    int eol = 0;

    if (m_socket.fd() <= 0)
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
    return bytes() - m_readOffset;
}

size_t TCPSocketReader::readLine(Buffer& destinationBuffer, char delimiter)
{
    size_t total = 0;
    int eol = 0;

    if (m_socket.fd() <= 0)
        throw Exception("Can't read from closed socket", __FILE__, __LINE__);

    while (eol == 0) {
        auto bytesToRead = int(destinationBuffer.capacity() - total - 1);
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
: BaseSocket(domain, type, protocol),
  m_reader(*this, 16384)
{
}

TCPSocket::~TCPSocket()
{
    TCPSocket::close();
}

void TCPSocket::_open(const Host& _host, OpenMode openMode, bool _blockingMode, std::chrono::milliseconds timeout)
{
    if (!_host.hostname().empty())
        host(_host);
    if (host().hostname().empty())
        throw Exception("Please, define the host name", __FILE__, __LINE__);

    if (proxy() != nullptr) {
        SOCKET fd = proxy()->connect(host(), _blockingMode, timeout);
        attach(fd, false);
    } else {
        sockaddr_in addr = {};
        host().getAddress(addr);

        _open(addr, openMode, _blockingMode, timeout);
    }
}

void TCPSocket::_open(const struct sockaddr_in& address, OpenMode openMode, bool _blockingMode,
                      chrono::milliseconds timeoutMS)
{
    open_addr(openMode, &address, timeoutMS);
    m_reader.open();

    if (!_blockingMode)
        blockingMode(false);
}

void TCPSocket::close() noexcept
{
    m_reader.close();
    BaseSocket::close();
}

void TCPSocket::accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo)
{
    socklen_t len = sizeof(clientInfo);
    clientSocketFD = ::accept(fd(), (struct sockaddr *) & clientInfo, &len);
    if (clientSocketFD == INVALID_SOCKET)
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

size_t TCPSocket::readLine(char *buffer, size_t size, char delimiter)
{
    return m_reader.read(buffer, size, delimiter, true);
}

size_t TCPSocket::readLine(Buffer& buffer, char delimiter)
{
    return m_reader.readLine(buffer, delimiter);
}

size_t TCPSocket::readLine(String& s, char delimiter)
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
    return m_reader.read(buffer, size, 0, false, from);
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
    size_t rc = m_reader.read(&buffer[0], size, 0, false, from);
    buffer.resize(rc);
    return rc;
}

void TCPSocket::setProxy(shared_ptr<Proxy> proxy)
{
    m_proxy = move(proxy);
}
