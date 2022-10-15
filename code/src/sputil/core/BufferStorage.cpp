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

#include <sptk5/Buffer.h>

using namespace std;
using namespace sptk;

void BufferStorage::adjustSize(size_t size)
{
    constexpr size_t sizeGranularity {32};
    size = (size / sizeGranularity + 1) * sizeGranularity;
    reallocate(size);
    m_buffer[size] = 0;
}

void BufferStorage::_set(const uint8_t* data, size_t size)
{
    checkSize(size + 1);
    if (data != nullptr && size > 0)
    {
        memcpy(m_buffer.data(), data, size);
        m_bytes = size;
    }
    else
    {
        m_bytes = 0;
    }
    m_buffer[size] = 0;
}

void BufferStorage::append(char chr)
{
    checkSize(m_bytes + 2);
    m_buffer[m_bytes] = chr;
    ++m_bytes;
    m_buffer[m_bytes] = 0;
}

void BufferStorage::append(const char* data, size_t size)
{
    if (size == 0)
    {
        size = strlen(data);
    }

    checkSize(m_bytes + size + 1);
    if (data != nullptr)
    {
        memcpy(m_buffer.data() + m_bytes, data, size);
        m_bytes += size;
        m_buffer[m_bytes] = 0;
    }
}

void BufferStorage::append(const uint8_t* data, size_t size)
{
    if (size == 0)
    {
        return;
    }

    checkSize(m_bytes + size + 1);
    if (data != nullptr)
    {
        memcpy(m_buffer.data() + m_bytes, data, size);
        m_bytes += size;
        m_buffer[m_bytes] = 0;
    }
}

void BufferStorage::reset(size_t size)
{
    checkSize(size + 1);
    m_buffer[0] = 0;
    m_bytes = 0;
}

void BufferStorage::fill(char chr, size_t count)
{
    checkSize(count + 1);
    memset(m_buffer.data(), chr, count);
    m_bytes = count;
    m_buffer[m_bytes] = 0;
}

void BufferStorage::erase(size_t offset, size_t length)
{
    if (offset + length >= m_bytes)
    {
        m_bytes = offset;
    }

    if (length == 0)
    {
        return;
    } // Nothing to do

    size_t moveOffset = offset + length;
    size_t moveLength = m_bytes - moveOffset;

    if (offset + length > m_bytes)
    {
        length = m_bytes - offset;
    }

    if (length > 0)
    {
        memmove(m_buffer.data() + offset, m_buffer.data() + offset + length, moveLength);
    }

    m_bytes -= length;
    m_buffer[m_bytes] = 0;
}
