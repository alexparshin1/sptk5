/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

SocketReader::SocketReader(TCPSocket& socket, size_t bufferSize)
    : m_socket(socket)
    , m_buffer(bufferSize)
{
}

void SocketReader::clear()
{
    scoped_lock const lock(m_mutex);
    m_readOffset = 0;
    m_buffer.bytes(0);
}

void SocketReader::close()
{
    scoped_lock const lock(m_mutex);
    try
    {
        m_readOffset = 0;
        m_buffer.bytes(0);
        m_socket.close();
    }
    catch (const Exception& e)
    {
        CERR(e.what());
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

size_t SocketReader::readFromSocket()
{
    m_readOffset = 0;
    int error;
    do
    {
        error = 0;
        const auto receivedBytes = static_cast<int>(m_socket.read(m_buffer.data(), m_buffer.capacity()));
        if (receivedBytes == -1)
        {
            m_buffer.bytes(0);
            if (m_socket.active())
            {
                error = errno;
                handleReadFromSocketError(error);
            }
        }
        else
        {
            m_buffer.bytes(static_cast<size_t>(receivedBytes));
        }
    } while (error == EAGAIN);

    m_buffer.data()[m_buffer.bytes()] = 0;

    return m_buffer.bytes();
}

static constexpr size_t readBytesLWM {128};

void SocketReader::readMoreFromSocket(size_t availableBytes)
{
    if (m_readOffset != 0)
    {
        memmove(m_buffer.data(), m_buffer.data() + m_readOffset, (size_t) availableBytes);
        m_readOffset = 0;
        m_buffer.bytes(availableBytes);
    }
    else
    {
        m_buffer.checkSize(m_buffer.capacity() + readBytesLWM);
    }

    const size_t receivedBytes = m_socket.read(m_buffer.data() + availableBytes, m_buffer.capacity() - availableBytes);
    m_buffer.bytes(m_buffer.bytes() + receivedBytes);
}

size_t SocketReader::bufferedRead(uint8_t* destination, size_t size)
{
    auto availableBytes = m_buffer.bytes() - m_readOffset;
    auto bytesToRead = size;

    if (availableBytes == 0)
    {
        if (!m_socket.active())
        {
            return 0;
        }

        availableBytes = readFromSocket();
        if (m_buffer.empty())
        {
            return 0;
        }
    }

    if (availableBytes < bytesToRead)
    {
        bytesToRead = availableBytes;
    }

    // copy data to destination, advance the read offset
    if (destination)
    {
        memcpy(destination, m_buffer.data() + m_readOffset, static_cast<size_t>(bytesToRead));
    }

    m_readOffset += bytesToRead;

    return bytesToRead;
}

int32_t SocketReader::bufferedReadLine(uint8_t* destination, size_t size, char delimiter)
{
    auto availableBytes = m_buffer.bytes() - m_readOffset;
    auto bytesToRead = size;
    bool eol = false;

    if (availableBytes == 0)
    {
        if (!m_socket.active())
        {
            return 0;
        }

        availableBytes = readFromSocket();
        if (m_buffer.empty())
        {
            return 0;
        }
    }

    char* readPosition = bit_cast<char*>(m_buffer.data()) + m_readOffset;
    if (availableBytes < bytesToRead)
    {
        bytesToRead = availableBytes;
    }

    char* carriageReturn = nullptr;
    size_t len;
    if (delimiter == 0)
    {
        len = strlen(readPosition);
    }
    else
    {
        carriageReturn = strchr(readPosition, delimiter);
        if (carriageReturn != nullptr)
        {
            len = carriageReturn - readPosition + 1;
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
        bytesToRead = static_cast<int>(len);
        if (delimiter == 0)
        {
            ++bytesToRead;
        }
        if (carriageReturn != nullptr)
        {
            *carriageReturn = 0;
        }
    }

    // copy data to destination, advance the read offset
    if (destination)
    {
        memcpy(destination, readPosition, static_cast<size_t>(bytesToRead));
        destination[bytesToRead] = 0;
    }

    m_readOffset += static_cast<uint32_t>(bytesToRead);

    return eol ? -static_cast<int>(bytesToRead) : static_cast<int>(bytesToRead);
}

size_t SocketReader::read(uint8_t* destination, size_t size)
{
    size_t totalReceived = 0;

    while (totalReceived < size)
    {
        const auto bytesToRead = size - totalReceived;
        const auto bytes = bufferedRead(destination, static_cast<size_t>(bytesToRead));

        if (bytes == 0)
        { // No more data
            break;
        }

        totalReceived += bytes;
        destination += bytes;
    }

    return totalReceived;
}

size_t SocketReader::readLine(uint8_t* destination, size_t size, char delimiter)
{
    int total = 0;
    int eol = 0;

    while (eol == 0)
    {
        const int bytesToRead = static_cast<int>(size) - total;
        if (bytesToRead <= 0)
        {
            return size;
        }

        int bytes = bufferedReadLine(destination, static_cast<size_t>(bytesToRead), delimiter);

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
    return static_cast<size_t>(total - eol);
}

size_t SocketReader::availableBytes() const
{
    scoped_lock const lock(m_mutex);
    auto              available = m_buffer.bytes() - m_readOffset;
    if (available == 0)
    {
        available = m_socket.socketBytes();
    }
    return available;
}

bool SocketReader::canRead(size_t bytesToRead) const
{
    scoped_lock const lock(m_mutex);

    const auto available = m_buffer.bytes() - m_readOffset;
    if (available >= bytesToRead)
    {
        return true;
    }

    return available + m_socket.socketBytes() >= bytesToRead;
}

bool SocketReader::readyToRead(std::chrono::milliseconds timeout) const
{
    scoped_lock const lock(m_mutex);

    if (const auto availableBytes = m_buffer.bytes() - m_readOffset;
        availableBytes > 0)
    {
        return true;
    }

    return m_socket.readyToRead(timeout);
}

size_t SocketReader::read(Buffer& destinationBuffer, size_t size)
{
    scoped_lock const lock(m_mutex);
    destinationBuffer.checkSize(size);
    const auto bytes = read(destinationBuffer.data(), size);
    destinationBuffer.bytes(bytes);
    return bytes;
}

size_t SocketReader::readLine(Buffer& destinationBuffer, char delimiter)
{
    scoped_lock const lock(m_mutex);
    size_t total = 0;
    int               eol = 0;

    if (!m_socket.active())
    {
        throw Exception("Can't read from closed socket");
    }

    while (eol == 0)
    {
        auto bytesToRead = static_cast<int>(destinationBuffer.capacity() - total - 1);
        if (bytesToRead <= static_cast<int>(readBytesLWM))
        {
            destinationBuffer.checkSize(destinationBuffer.capacity() + readBytesLWM);
            bytesToRead = static_cast<int>(destinationBuffer.capacity() - total - 1);
        }

        auto* destination = destinationBuffer.data() + total;

        int bytes = bufferedReadLine(destination, static_cast<size_t>(bytesToRead), delimiter);
        if (bytes == 0)
        { // No more data
            break;
        }

        if (bytes < 0)
        { // Received the complete string
            eol = 1;
            bytes = -bytes;
        }

        total += static_cast<size_t>(bytes);
    }
    destinationBuffer.data()[total] = 0;
    destinationBuffer.bytes(total);
    return destinationBuffer.bytes();
}

size_t SocketReader::readLine(String& destinationBuffer, char delimiter)
{
    Buffer buffer;
    const auto bytes = readLine(buffer, delimiter);
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
