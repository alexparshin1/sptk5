/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include <sptk5/SystemException.h>
#include <sptk5/filedefs.h>

using namespace std;
using namespace sptk;

BufferStorage::BufferStorage(size_t sz)
{
    allocate(sz + 1);
}

BufferStorage::BufferStorage(const void* data, size_t sz)
{
    allocate(sz + 1);
    if (data != nullptr) {
        memcpy(m_buffer, data, sz);
        m_bytes = sz;
    }
}

void BufferStorage::adjustSize(size_t sz)
{
    sz = (sz / 128 + 1) * 128;
    reallocate(sz);
    m_buffer[sz] = 0;
}

void BufferStorage::set(const char* data, size_t sz)
{
    checkSize(sz);
    if (data != nullptr) {
        memcpy(this->data(), data, sz + 1);
        m_bytes = sz;
    } else
        m_bytes = 0;
}

void BufferStorage::append(char ch)
{
    checkSize(m_bytes + 1);
    m_buffer[m_bytes] = ch;
    ++m_bytes;
}

void BufferStorage::append(const char* data, size_t sz)
{
    if (sz == 0)
        sz = (size_t) strlen(data);

    checkSize(m_bytes + sz + 1);
    if (data != nullptr) {
        memcpy(m_buffer + m_bytes, data, sz);
        m_bytes += sz;
        m_buffer[m_bytes] = 0;
    }
}

void BufferStorage::reset(size_t sz)
{
    checkSize(sz + 1);
    m_buffer[0] = 0;
    m_bytes = 0;
}

void BufferStorage::fill(char c, size_t count)
{
    checkSize(count + 1);
    memset(m_buffer, c, count);
    m_bytes = count;
    m_buffer[m_bytes] = 0;
}

void BufferStorage::erase(size_t offset, size_t length)
{
    if (offset >= m_bytes || length == 0)
        return; // Nothing to do
    if (offset + length > m_bytes)
        length = m_bytes - offset;
    if (length > 0)
        memmove(m_buffer + offset, m_buffer + offset + length, length);
    m_bytes -= length;
    m_buffer[m_bytes] = 0;
}
