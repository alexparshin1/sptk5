/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>
#include <sptk5/net/SocketReader.h>

using namespace std;
using namespace sptk;

SocketReader::SocketReader(BaseSocket& socket, size_t buffer_size)
    : Buffer(buffer_size)
    , m_socket(socket)
{
}

void SocketReader::open()
{
    m_readOffset = 0;
    bytes(0);
}

void SocketReader::close() noexcept
{
    try
    {
        m_readOffset = 0;
        bytes(0);
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }
}

void SocketReader::handleReadFromSocketError(int error)
{
    if (error == EAGAIN)
    {
        if (!m_socket.readyToRead(chrono::seconds(1)))
        {
            throw TimeoutException("Can't read from socket: timeout");
        }
    }
    else
    {
        throw SystemException("Can't read from socket");
    }
}

int32_t SocketReader::readFromSocket(sockaddr_in* from)
{
    if (!m_socket.active())
    {
        return 0;
    }

    m_readOffset = 0;
    int error {0};
    do
    {
        error = 0;
        int receivedBytes;
        if (from != nullptr)
        {
#ifdef _WIN32
            int flen = sizeof(sockaddr_in);
#else
            socklen_t flen = sizeof(sockaddr_in);
#endif
            receivedBytes = (int) recvfrom(m_socket.fd(), (char*) data(), (int) capacity() - 2, 0, (sockaddr*) from,
                                           &flen);
        }
        else
        {
            receivedBytes = (int) m_socket.recv(data(), capacity() - 2);
        }

        if (receivedBytes == -1)
        {
            bytes(0);
            error = errno;
            handleReadFromSocketError(error);
        }
        else
        {
            bytes((size_t) receivedBytes);
        }
    } while (error == EAGAIN);

    data()[bytes()] = 0;

    return (int32_t) bytes();
}

static constexpr size_t readBytesLWM {128};

void SocketReader::readMoreFromSocket(int availableBytes)
{
    if (m_readOffset != 0)
    {
        memmove(data(), data() + m_readOffset, (size_t) availableBytes);
        m_readOffset = 0;
        bytes((size_t) availableBytes);
    }
    else
    {
        checkSize(capacity() + readBytesLWM);
    }
    size_t receivedBytes = m_socket.recv(data() + availableBytes, capacity() - availableBytes);
    bytes(bytes() + receivedBytes);
}

int32_t SocketReader::bufferedRead(uint8_t* destination, size_t size, char delimiter, bool read_line,
                                      sockaddr_in* from)
{
    auto availableBytes = int(bytes() - m_readOffset);
    auto bytesToRead = (int) size;
    bool eol = false;

    if (availableBytes == 0)
    {
        if (!m_socket.active())
        {
            return 0;
        }

        availableBytes = readFromSocket(from);
        if (empty())
        {
            return 0;
        }
    }

    char* readPosition = (char*) data() + m_readOffset;
    if (availableBytes < bytesToRead)
    {
        bytesToRead = availableBytes;
    }

    char* cr = nullptr;
    if (read_line)
    {
        size_t len;
        if (delimiter == 0)
        {
            len = strlen(readPosition);
        }
        else
        {
            cr = strchr(readPosition, delimiter);
            if (cr != nullptr)
            {
                len = cr - readPosition + 1;
            }
            else
            {
                readMoreFromSocket(availableBytes);
                return 0;
            }
        }
        if (len < size)
        {
            eol = true;
            bytesToRead = (int) len;
            if (delimiter == 0)
            {
                ++bytesToRead;
            }
            if (cr != nullptr)
            {
                *cr = 0;
            }
        }
    }

    // copy data to destination, advance the read offset
    memcpy(destination, readPosition, size_t(bytesToRead));

    if (read_line || bytesToRead < int(size))
    {
        destination[bytesToRead] = 0;
    }

    m_readOffset += uint32_t(bytesToRead);
    return eol ? -bytesToRead : bytesToRead;
}

size_t SocketReader::read(uint8_t* destination, size_t sz, char delimiter, bool read_line, sockaddr_in* from)
{
    int total = 0;
    int eol = 0;

    while (eol == 0)
    {
        int bytesToRead = int(sz) - total;
        if (bytesToRead <= 0)
        {
            return sz;
        }

        int bytes = bufferedRead(destination, size_t(bytesToRead), delimiter, read_line, from);

        if (bytes == 0)
        { // No more data
            break;
        }

        if (bytes < 0)
        { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += bytes;
        destination += bytes;
    }
    return size_t(total - eol);
}

size_t SocketReader::availableBytes() const
{
    return bytes() - m_readOffset;
}

size_t SocketReader::readLine(Buffer& destinationBuffer, char delimiter)
{
    size_t total = 0;
    int eol = 0;

    if (!m_socket.active())
    {
        throw Exception("Can't read from closed socket", __FILE__, __LINE__);
    }

    while (eol == 0)
    {
        auto bytesToRead = int(destinationBuffer.capacity() - total - 1);
        if (bytesToRead <= (int) readBytesLWM)
        {
            destinationBuffer.checkSize(destinationBuffer.capacity() + readBytesLWM);
            bytesToRead = int(destinationBuffer.capacity() - total - 1);
        }

        auto* destination = destinationBuffer.data() + total;

        int bytes = bufferedRead(destination, size_t(bytesToRead), delimiter, true);
        if (bytes == 0)
        { // No more data
            break;
        }

        if (bytes < 0)
        { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += size_t(bytes);
    }
    destinationBuffer.data()[total] = 0;
    destinationBuffer.bytes(total);
    return destinationBuffer.bytes();
}
