/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/Buffer.h>

using namespace std;
using namespace sptk;

void BufferStorage::throwNotEnoughMemory()
{
    throw Exception("Not enough memory");
}

void BufferStorage::swapInternal(BufferStorage& other)
{
    swap(m_buffer, other.m_buffer);
    swap(m_allocated, other.m_allocated);
    swap(m_size, other.m_size);
}

void BufferStorage::_set(const uint8_t* data, const size_t size)
{
    reserve(size + 1);
    if (data != nullptr && size > 0)
    {
        memcpy(m_buffer, data, size);
        m_size = size;
    }
    else
    {
        m_size = 0;
    }
    m_buffer[m_size] = 0;
}

void BufferStorage::append(const char chr)
{
    reserve(m_size + 1);
    m_buffer[m_size] = chr;
    m_buffer[++m_size] = 0;
}

void BufferStorage::reset(const size_t size)
{
    if (size > m_allocated)
    {
        reserve(size);
    }
    m_buffer[0] = 0;
    m_size = 0;
}

void BufferStorage::fill(const char chr, const size_t count)
{
    reserve(count);
    memset(m_buffer, chr, count);
    m_size = count;
    m_buffer[m_size] = 0;
}

void BufferStorage::erase(const size_t offset, size_t length)
{
    if (offset >= m_allocated)
    {
        // Attempt to erase after the allocated memory.
        return;
    }

    if (offset + length >= m_size)
    {
        m_size = offset;
        return;
    }

    if (length == 0)
    {
        return;
    }

    const auto moveOffset = offset + length;
    const auto moveLength = m_size - moveOffset;

    if (offset + length > m_size)
    {
        length = m_size - offset;
    }

    if (length > 0)
    {
        memmove(m_buffer + offset, m_buffer + offset + length, moveLength);
        m_size -= length;
        m_buffer[m_size] = 0;
    }
}

