/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Buffer.cpp - description                               ║
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

#include <sptk5/Buffer.h>
#include <sptk5/SystemException.h>
#include <sptk5/filedefs.h>
#include <iomanip>

using namespace std;
using namespace sptk;

Buffer::Buffer(size_t sz)
: m_storage(sz + 1)
{
    m_buffer = &*m_storage.begin();
}

Buffer::Buffer(const void* data, size_t sz)
: m_storage(sz + 1)
{
    m_buffer = &*m_storage.begin();

    if (data != nullptr)
    {
        memcpy(m_buffer, data, sz);
        m_bytes = sz;
        m_buffer[sz] = 0;
    }
    else
        m_buffer[0] = 0;
}

Buffer::Buffer(const char* str)
{
    auto sz = (size_t) strlen(str);
    m_storage.resize(sz + 1);
    m_buffer = &*m_storage.begin();

    if (str != nullptr) {
        memcpy(m_buffer, str, sz);
        m_bytes = sz;
    }
    m_buffer[sz] = 0;
}

Buffer::Buffer(const string& str)
: m_storage(str.length() + 1)
{
    m_buffer = &*m_storage.begin();
    auto sz = str.length();

    if (!str.empty()) {
        memcpy(m_buffer, str.c_str(), sz);
        m_bytes = sz;
    }
    m_buffer[sz] = 0;
}

Buffer::Buffer(const String& str)
: m_storage(str.length() + 1)
{
    m_buffer = &*m_storage.begin();
    auto sz = str.length();

    if (!str.empty()) {
        memcpy(m_buffer, str.c_str(), sz);
        m_bytes = sz;
    }
    m_buffer[sz] = 0;
}

Buffer::Buffer(const Buffer& other)
: m_storage(other.m_storage), m_bytes(other.m_bytes)
{
    m_buffer = &*m_storage.begin();
}

Buffer::Buffer(Buffer&& other) noexcept
: m_storage(move(other.m_storage)), m_bytes(other.m_bytes)
{
    m_buffer = &*m_storage.begin();
    other.m_bytes = 0;
}

void Buffer::adjustSize(size_t sz)
{
    m_storage.resize(sz + 1);
    m_buffer = &*m_storage.begin();
    m_buffer[sz] = 0;
}

void Buffer::set(const char* data, size_t sz)
{
    checkSize(sz);
    memcpy(m_buffer, data, sz);
    m_bytes = sz;
}

void Buffer::append(char ch)
{
    checkSize(m_bytes + 1);
    m_buffer[m_bytes] = ch;
    m_bytes++;
}

void Buffer::append(uint8_t val)
{
    checkSize(m_bytes + 1);
    m_buffer[m_bytes] = val;
    m_bytes++;
}

void Buffer::append(uint16_t val)
{
    checkSize(m_bytes + 2);
    *(uint16_t*)(m_buffer + m_bytes) = val;
    m_bytes += 2;
}

void Buffer::append(const char* data, size_t sz)
{
    if (sz == 0)
        sz = (size_t) strlen(data);

    checkSize(m_bytes + sz + 1);
    memcpy(m_buffer + m_bytes, data, sz);
    m_bytes += sz;
    m_buffer[m_bytes] = 0;
}

void Buffer::fill(char c, size_t count)
{
    checkSize(count + 1);
    memset(m_buffer, c, count);
    m_bytes = count;
    m_buffer[m_bytes] = 0;
}

void Buffer::reset(size_t sz)
{
    checkSize(sz + 1);
    m_buffer = &*m_storage.begin();
    m_buffer[0] = 0;
    m_bytes = 0;
}

void Buffer::loadFromFile(const String& fileName)
{
    FILE* f = fopen(fileName.c_str(), "rb");

    if (f == nullptr)
        throw SystemException("Can't open file " + fileName + " for reading");

    struct stat st = {};
    if (fstat(fileno(f), &st) != 0) {
        fclose(f);
        throw Exception("Can't get file size for '" + fileName + "'");
    }

    auto size = (size_t) st.st_size;

    reset(size + 1);
    m_buffer[size] = 0;
    m_bytes = fread(m_buffer, 1, size, f);
    fclose(f);
}

void Buffer::saveToFile(const String& fileName) const
{
    FILE* f = fopen(fileName.c_str(), "wb");

    if (f == nullptr)
        throw SystemException("Can't open file " + fileName + " for writing");

    fwrite(m_buffer, bytes(), 1, f);
    fclose(f);
}

Buffer& Buffer::operator = (Buffer&& other) DOESNT_THROW
{
    m_storage = move(other.m_storage);
    m_buffer = &m_storage[0];

    m_bytes = other.m_bytes;
    other.m_bytes = 0;

    return *this;
}

Buffer& Buffer::operator = (const Buffer& other)
{
    if (&other == this)
        return *this;

    m_storage.assign(other.m_storage.begin(), other.m_storage.end());
    m_buffer = &m_storage[0];
    m_bytes = other.m_bytes;

    return *this;
}

Buffer& Buffer::operator = (const std::string& str)
{
    auto sz = (size_t) str.length();
    checkSize(sz);

    if (sz != 0)
        memcpy(m_buffer, str.c_str(), sz + 1);

    m_bytes = sz;
    return *this;
}

Buffer& Buffer::operator = (const char* str)
{
    auto sz = (size_t) strlen(str);
    checkSize(sz + 1);

    if (sz != 0)
        memcpy(m_buffer, str, sz + 1);

    m_bytes = sz;
    return *this;
}

ostream& sptk::operator<<(ostream& stream, const Buffer& buffer)
{
    char fillChar = stream.fill('0');
    auto old_settings = stream.flags();

    size_t offset = 0;

    while (offset < buffer.bytes()) {
        stream << hex << setw(8) << offset << "  ";

        size_t printed = 0;
        size_t rowOffset = offset;
        for (; rowOffset < buffer.bytes() && printed < 16; rowOffset++, printed++) {
            if (printed == 8)
                stream << " ";
            unsigned printChar = (uint8_t) buffer[rowOffset];
            stream << hex << setw(2) << printChar << " ";
        }

        while (printed < 16) {
            stream << "   ";
            printed++;
        }

        stream << " ";

        printed = 0;
        rowOffset = offset;
        for (; rowOffset < buffer.bytes() && printed < 16; rowOffset++, printed++) {
            if (printed == 8)
                stream << " ";
            auto testChar = (uint8_t) buffer[rowOffset];
            if (testChar >= 32)
                stream << buffer[rowOffset];
            else
                stream << ".";
        }

        stream << endl;
        offset += 16;
    }

    stream.fill(fillChar);
    stream.flags(old_settings);

    return stream;
}

#if USE_GTEST

static const char* testPhrase = "This is a test";
static const char* tempFileName = "/tmp/gtest_sptk5_buffer.tmp";

TEST(SPTK_Buffer, create)
{
    Buffer  buffer1(testPhrase);
    EXPECT_STREQ(testPhrase, buffer1.c_str());
    EXPECT_EQ(strlen(testPhrase), buffer1.bytes());
    EXPECT_TRUE(strlen(testPhrase) < buffer1.capacity());
}

Buffer* ptr;

TEST(SPTK_Buffer, assign)
{
    Buffer  buffer1(testPhrase);
    ptr = &buffer1;

    Buffer  buffer2;

    buffer2 = buffer1;

    EXPECT_STREQ(testPhrase, buffer2.c_str());
    EXPECT_EQ(strlen(testPhrase), buffer2.bytes());
    EXPECT_TRUE(strlen(testPhrase) < buffer2.capacity());
}

TEST(SPTK_Buffer, append)
{
    Buffer  buffer1;

    buffer1.append(testPhrase);

    EXPECT_STREQ(testPhrase, buffer1.c_str());
    EXPECT_EQ(strlen(testPhrase), buffer1.bytes());
    EXPECT_TRUE(strlen(testPhrase) < buffer1.capacity());
}

TEST(SPTK_Buffer, saveLoadFile)
{
    Buffer  buffer1(testPhrase);
    Buffer  buffer2;

    buffer1.saveToFile(tempFileName);
    buffer2.loadFromFile(tempFileName);

    EXPECT_STREQ(testPhrase, buffer2.c_str());
    EXPECT_EQ(strlen(testPhrase), buffer2.bytes());
    EXPECT_TRUE(strlen(testPhrase) < buffer2.capacity());
}

TEST(SPTK_Buffer, fill)
{
    Buffer  buffer1;

    buffer1.fill('#', 12);

    EXPECT_STREQ("############", buffer1.c_str());
    EXPECT_EQ(size_t(12), buffer1.bytes());
}

TEST(SPTK_Buffer, reset)
{
    Buffer  buffer1(testPhrase);

    buffer1.reset();

    EXPECT_STREQ("", buffer1.c_str());
    EXPECT_EQ(size_t(0), buffer1.bytes());
    EXPECT_TRUE(buffer1.capacity() > 0);
}

#endif
