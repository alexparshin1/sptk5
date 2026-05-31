/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

SocketReader::SocketReader(shared_ptr<TCPSocket> socket, const size_t bufferSize)
    : m_socket(std::move(socket))
    , m_buffer(bufferSize)
    , m_lineBuffer(128)
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
        m_socket->close();
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

void SocketReader::handleReadFromSocketError(const int error) const
{
    if (error == EAGAIN)
    {
        if (!m_socket->readyToRead(chrono::seconds(1)))
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
        if (const auto receivedBytes = static_cast<int>(m_socket->read(m_buffer.data(), m_buffer.capacity()));
            receivedBytes == -1)
        {
            m_buffer.bytes(0);
            if (m_socket->active())
            {
                error = getSocketError();
                handleReadFromSocketError(error);
            }
        }
        else
        {
            m_buffer.bytes(static_cast<size_t>(receivedBytes));
        }
    } while (error == EAGAIN);

    return m_buffer.bytes();
}

// Minimum growth step when the read buffer needs to be enlarged. Growth is
// geometric (doubling) but never smaller than this floor.
static constexpr size_t readBytesLWM {128};

void SocketReader::readMoreFromSocket(const size_t availableBytes)
{
    if (m_readOffset != 0)
    {
        const auto* currentReadPosition = m_buffer.data() + m_readOffset;
        if (m_buffer.data() + availableBytes < currentReadPosition)
        {
            memcpy(m_buffer.data(), currentReadPosition, availableBytes);
        }
        else
        {
            memmove(m_buffer.data(), currentReadPosition, availableBytes);
        }
        m_readOffset = 0;
        m_buffer.bytes(availableBytes);
    }
    else
    {
        const auto cap = m_buffer.capacity();
        m_buffer.checkSize(std::max(cap * 2, cap + readBytesLWM));
    }

    const auto receivedBytes = m_socket->read(m_buffer.data() + availableBytes, m_buffer.capacity() - availableBytes);
    m_buffer.bytes(m_buffer.bytes() + receivedBytes);
}

size_t SocketReader::bufferedRead(uint8_t* destination, const size_t size)
{
    auto availableBytes = m_buffer.bytes() - m_readOffset;
    auto bytesToRead = size;

    if (availableBytes == 0)
    {
        if (!m_socket->active())
        {
            return 0;
        }

        availableBytes = readFromSocket();
        if (availableBytes == 0)
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
        memcpy(destination, m_buffer.data() + m_readOffset, bytesToRead);
    }

    m_readOffset += bytesToRead;

    return bytesToRead;
}

size_t SocketReader::bufferedReadLine(uint8_t* destination, const size_t size, const char delimiter, bool& endOfLine)
{
    auto availableBytes = m_buffer.size() - m_readOffset;
    auto bytesToRead = size;

    endOfLine = false;

    if (availableBytes == 0)
    {
        if (!m_socket->active())
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

    char*  carriageReturn = nullptr;
    size_t len;
    if (delimiter == 0)
    {
        readPosition[availableBytes] = 0;
        const auto* eol = static_cast<const char*>(memchr(readPosition, 0, availableBytes));
        len = eol == nullptr ? availableBytes : eol - readPosition - 1;
    }
    else
    {
        carriageReturn = static_cast<char*>(memchr(readPosition, delimiter, availableBytes));
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
        endOfLine = true;
        bytesToRead = len;
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
        memcpy(destination, readPosition, bytesToRead);
        //destination[bytesToRead] = 0;
    }

    m_readOffset += bytesToRead;

    return bytesToRead;
}

size_t SocketReader::read(uint8_t* destination, const size_t size)
{
    scoped_lock const lock(m_mutex);

    if (!m_socket->active())
    {
        throw Exception("Can't read from closed socket");
    }

    size_t totalReceived = 0;

    while (totalReceived < size)
    {
        const auto bytesToRead = size - totalReceived;
        const auto bytes = bufferedRead(destination, bytesToRead);

        if (bytes == 0)
        { // No more data
            break;
        }

        totalReceived += bytes;
        destination += bytes;
    }

    return totalReceived;
}


size_t SocketReader::availableBytes() const
{
    scoped_lock const lock(m_mutex);
    auto              available = m_buffer.size() - m_readOffset;
    if (available == 0)
    {
        available = m_socket->socketBytes();
    }
    return available;
}

[[maybe_unused]] bool SocketReader::canRead(const size_t bytesToRead) const
{
    scoped_lock const lock(m_mutex);

    const auto available = m_buffer.size() - m_readOffset;
    if (available >= bytesToRead)
    {
        return true;
    }

    return available + m_socket->socketBytes() >= bytesToRead;
}

bool SocketReader::readyToRead(const chrono::milliseconds& timeout) const
{
    scoped_lock const lock(m_mutex);

    if (const auto availableBytes = m_buffer.size() - m_readOffset;
        availableBytes > 0)
    {
        return true;
    }

    return m_socket->readyToRead(timeout);
}

size_t SocketReader::read(Buffer& destinationBuffer, const size_t size)
{
    destinationBuffer.checkSize(size);
    const auto bytes = read(destinationBuffer.data(), size);
    destinationBuffer.bytes(bytes);
    return bytes;
}

size_t SocketReader::readLine(uint8_t* destinationBuffer, const size_t size, const char delimiter)
{
    scoped_lock const lock(m_mutex);

    size_t total = 0;
    auto   endOfLine = false;

    while (!endOfLine)
    {
        const auto bytesToRead = static_cast<int>(size - total);
        if (bytesToRead <= 0)
        {
            return size;
        }

        endOfLine = false;
        const auto bytes = bufferedReadLine(destinationBuffer, static_cast<size_t>(bytesToRead), delimiter, endOfLine);
        if (bytes == 0)
        {
            // No more data
            break;
        }

        total += bytes;
        destinationBuffer += bytes;
    }

    return total - (endOfLine ? 1 : 0);
}

size_t SocketReader::readLine(Buffer& destinationBuffer, const char delimiter)
{
    scoped_lock const lock(m_mutex);
    size_t            total = 0;
    auto              endOfLine = false;
    constexpr size_t  maxBufferSize = 128 * 1024;

    while (!endOfLine)
    {
        auto bytesToRead = static_cast<int>(destinationBuffer.capacity() - total - 1);
        if (bytesToRead <= static_cast<int>(readBytesLWM))
        {
            if (const auto desiredCapacity = std::min(maxBufferSize, destinationBuffer.capacity() * 2 + readBytesLWM);
                desiredCapacity > destinationBuffer.capacity())
            {
                destinationBuffer.checkSize(desiredCapacity);
                bytesToRead = static_cast<int>(destinationBuffer.capacity() - total - 1);
            }
        }

        auto* destination = destinationBuffer.data() + total;

        const auto bytes = bufferedReadLine(destination, static_cast<size_t>(bytesToRead), delimiter, endOfLine);
        if (bytes == 0)
        { // No more data
            break;
        }

        total += bytes;
    }
    destinationBuffer.data()[total] = 0;
    destinationBuffer.bytes(total);
    return destinationBuffer.bytes();
}

size_t SocketReader::readLine(String& destinationBuffer, const char delimiter)
{
    const auto bytes = readLine(m_lineBuffer, delimiter);
    if (bytes > 0)
    {
        destinationBuffer.assign(m_lineBuffer.c_str(), bytes - 1);
    }
    else
    {
        destinationBuffer.clear();
    }
    return bytes;
}

shared_ptr<TCPSocket> SocketReader::socket() const
{
    return m_socket;
}
