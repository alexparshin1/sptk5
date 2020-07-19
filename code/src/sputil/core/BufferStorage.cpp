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

BufferStorage::BufferStorage(const BufferStorage& other)
{
    allocate(other.m_bytes + 1);
    m_bytes = other.m_bytes;
    if (m_bytes > 0)
        memcpy(m_buffer, other.m_buffer, other.m_bytes + 1);
}

BufferStorage::BufferStorage(BufferStorage&& other) noexcept
: m_buffer(exchange(other.m_buffer, nullptr)),
  m_size(exchange(other.m_size, 0)),
  m_bytes(exchange(other.m_bytes,0))
{
}

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

BufferStorage& BufferStorage::operator=(const BufferStorage& other)
{
    if (&other != this) {
        if (m_size < other.m_size) {
            delete m_buffer;
            allocate(other.m_bytes + 1);
        }
        m_bytes = other.m_bytes;
        if (m_bytes > 0)
            memcpy(m_buffer, other.m_buffer, other.m_bytes + 1);
    }
    return *this;
}

BufferStorage& BufferStorage::operator=(BufferStorage&& other) noexcept
{
    if (&other != this) {
        m_buffer = exchange(other.m_buffer, nullptr);
        m_size = exchange(other.m_size, 0);
        m_bytes = exchange(other.m_bytes, 0);
    }
    return *this;
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
    checkSize(m_bytes + 2);
    m_buffer[m_bytes] = ch;
    ++m_bytes;
    m_buffer[m_bytes] = 0;
}

void BufferStorage::append(const char* data, size_t sz)
{
    if (sz == 0)
        sz = strlen(data);

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
    if (offset + length >= m_bytes)
        m_bytes = offset;

    if (length == 0)
        return; // Nothing to do

    size_t moveOffset = offset + length;
    size_t moveLength = m_bytes - moveOffset;

    if (offset + length > m_bytes)
        length = m_bytes - offset;

    if (length > 0)
        memmove(m_buffer + offset, m_buffer + offset + length, moveLength);

    m_bytes -= length;
    m_buffer[m_bytes] = 0;
}

#if USE_GTEST

static const String testString("0123456789ABCDEF");

TEST(SPTK_BufferStorage, constructors)
{
    BufferStorage testStorage1(testString.c_str(), testString.length());

    BufferStorage testStorage2(testStorage1);
    EXPECT_EQ(testStorage2.length(), size_t(16));
    EXPECT_STREQ(testStorage2.c_str(), testString.c_str());

    BufferStorage testStorage3(move(testStorage1));
    EXPECT_EQ(testStorage3.length(), size_t(16));
    EXPECT_STREQ(testStorage3.c_str(), testString.c_str());
    EXPECT_EQ(testStorage1.length(), size_t(0));
    EXPECT_STREQ(testStorage1.c_str(), nullptr);
}

TEST(SPTK_BufferStorage, assignments)
{
    BufferStorage testStorage1(testString.c_str(), testString.length());

    BufferStorage testStorage2;
    testStorage2 = testStorage1;
    EXPECT_EQ(testStorage2.length(), size_t(16));
    EXPECT_STREQ(testStorage2.c_str(), testString.c_str());

    BufferStorage testStorage3;
    testStorage3 = move(testStorage1);
    EXPECT_EQ(testStorage3.length(), size_t(16));
    EXPECT_STREQ(testStorage3.c_str(), testString.c_str());
    EXPECT_EQ(testStorage1.length(), size_t(0));
    EXPECT_STREQ(testStorage1.c_str(), nullptr);
}

TEST(SPTK_BufferStorage, append)
{
    BufferStorage testStorage;

    for (auto ch: testString)
        testStorage.append(ch);
    testStorage.append(testString.c_str(), testString.length());

    EXPECT_EQ(testStorage.length(), size_t(32));
    EXPECT_STREQ(testStorage.c_str(), "0123456789ABCDEF0123456789ABCDEF");
}

TEST(SPTK_BufferStorage, erase)
{
    BufferStorage testStorage(32);
    testStorage.fill(0, 32);
    testStorage.set("0123456789ABCDEF");
    EXPECT_EQ(testStorage.length(), size_t(16));
    EXPECT_STREQ(testStorage.c_str(), testString.c_str());
    testStorage.erase(0,4);
    EXPECT_STREQ(testStorage.c_str(), "456789ABCDEF");
}

TEST(SPTK_BufferStorage, reset)
{
    BufferStorage testStorage(32);
    testStorage.set(testString.c_str());

    testStorage.reset();
    EXPECT_EQ(testStorage.length(), size_t(0));

    testStorage.append(testString.c_str(), testString.length());

    EXPECT_EQ(testStorage.length(), testString.length());
    EXPECT_STREQ(testStorage.c_str(), testString.c_str());
}

#endif
