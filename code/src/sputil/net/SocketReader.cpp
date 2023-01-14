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

#include <sptk5/cutils>
#include <sptk5/net/SocketReader.h>

using namespace std;
using namespace sptk;

SocketReader::SocketReader(TCPSocket& socket, size_t buffer_size)
    : Buffer(buffer_size)
    , m_socket(socket)
{
}

void SocketReader::reset()
{
    scoped_lock lock(m_mutex);
    m_readOffset = 0;
    bytes(0);
}

void SocketReader::close()
{
    scoped_lock lock(m_mutex);
    try
    {
        m_readOffset = 0;
        bytes(0);
        m_socket.close();
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

int32_t SocketReader::readFromSocket()
{
    m_readOffset = 0;
    int error {0};
    do
    {
        error = 0;
        auto receivedBytes = (int) m_socket.recv(data(), capacity() - 2);

        if (receivedBytes == -1)
        {
            bytes(0);
            if (m_socket.active())
            {
                error = errno;
                handleReadFromSocketError(error);
            }
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

int32_t SocketReader::bufferedRead(uint8_t* destination, size_t size, char delimiter, bool read_line)
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

        availableBytes = readFromSocket();
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

size_t SocketReader::read(uint8_t* destination, size_t sz, char delimiter, bool read_line)
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

        int bytes = bufferedRead(destination, size_t(bytesToRead), delimiter, read_line);

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
    scoped_lock lock(m_mutex);
    auto available = bytes() - m_readOffset;
    if (available == 0 && readyToRead(chrono::milliseconds(0)))
    {
        available = m_socket.socketBytes();
    }
    return available;
}

bool SocketReader::readyToRead(std::chrono::milliseconds timeout) const
{
    scoped_lock lock(m_mutex);

    auto availableBytes = bytes() - m_readOffset;
    if (availableBytes > 0)
    {
        return true;
    }

    return m_socket.readyToRead(timeout);
}

size_t SocketReader::read(Buffer& destinationBuffer, size_t size)
{
    scoped_lock lock(m_mutex);
    destinationBuffer.checkSize(size);
    auto bytes = read(destinationBuffer.data(), size, '\0', false);
    destinationBuffer.bytes(bytes);
    return bytes;
}

size_t SocketReader::read(uint8_t* destinationBuffer, size_t size)
{
    scoped_lock lock(m_mutex);
    return read(destinationBuffer, size, '\0', false);
}

size_t SocketReader::readLine(Buffer& destinationBuffer, char delimiter)
{
    scoped_lock lock(m_mutex);
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

size_t SocketReader::readLine(String& destinationBuffer, char delimiter)
{
    Buffer buffer;
    auto bytes = readLine(buffer, delimiter);
    if (bytes > 0)
    {
        destinationBuffer.assign(buffer.c_str(), bytes - 1);
    }
    else
    {
        destinationBuffer.clear();
    }
    return bytes;
}

TCPSocket& SocketReader::socket()
{
    return m_socket;
}
