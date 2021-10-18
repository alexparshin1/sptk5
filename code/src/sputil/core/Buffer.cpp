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

#include <filesystem>
#include <iomanip>
#include <sptk5/Buffer.h>
#include <sptk5/SystemException.h>
#include <sys/stat.h>

using namespace std;
using namespace sptk;

Buffer::Buffer(const String& str)
    : BufferStorage((const uint8_t*) str.c_str(), str.length())
{
}

void Buffer::loadFromFile(const filesystem::path& fileName)
{
    FILE* f = fopen(fileName.string().c_str(), "rb");

    if (f == nullptr)
    {
        throw SystemException("Can't open file " + fileName.string() + " for reading");
    }

    struct stat st = {};
    if (fstat(fileno(f), &st) != 0)
    {
        fclose(f);
        throw Exception("Can't get file size for '" + fileName.string() + "'");
    }

    auto size = (size_t) st.st_size;

    reset(size + 1);
    bytes(fread(data(), 1, size, f));
    fclose(f);
}

void Buffer::saveToFile(const fs::path& fileName) const
{
    FILE* f = fopen(fileName.string().c_str(), "wb");

    if (f == nullptr)
    {
        throw SystemException("Can't open file " + fileName.string() + " for writing");
    }

    fwrite(data(), bytes(), 1, f);
    fclose(f);
}

Buffer& Buffer::operator=(const String& other)
{
    set((const uint8_t*) other.c_str(), other.length());

    return *this;
}

Buffer& Buffer::operator=(const char* str)
{
    set((const uint8_t*) str, strlen(str));

    return *this;
}

bool Buffer::operator==(const Buffer& other) const
{
    if (bytes() != other.bytes())
    {
        return false;
    }
    return memcmp(data(), other.data(), bytes()) == 0;
}

bool Buffer::operator!=(const Buffer& other) const
{
    if (bytes() != other.bytes())
    {
        return true;
    }
    return memcmp(data(), other.data(), bytes()) != 0;
}

ostream& sptk::operator<<(ostream& stream, const Buffer& buffer)
{
    if ((stream.flags() & ios::hex) == 0)
    {
        stream << buffer.c_str();
        return stream;
    }

    char fillChar = stream.fill('0');
    auto old_settings = stream.flags();

    size_t offset = 0;

    constexpr int addressWidth {8};
    constexpr int bytesInHalfRow {8};
    constexpr int bytesInRow {16};

    while (offset < buffer.bytes())
    {
        stream << hex << setw(addressWidth) << offset << "  ";

        size_t printed = 0;
        size_t rowOffset = offset;
        for (; rowOffset < buffer.bytes() && printed < bytesInRow; ++rowOffset, ++printed)
        {
            if (printed == bytesInHalfRow)
            {
                stream << " ";
            }
            unsigned printChar = buffer[rowOffset];
            stream << hex << setw(2) << printChar << " ";
        }

        while (printed < bytesInRow)
        {
            stream << "   ";
            ++printed;
        }

        stream << " ";

        printed = 0;
        rowOffset = offset;
        for (; rowOffset < buffer.bytes() && printed < bytesInRow; ++rowOffset, ++printed)
        {
            if (printed == bytesInHalfRow)
            {
                stream << " ";
            }
            auto testChar = buffer[rowOffset];
            if (testChar >= ' ')
            {
                stream << buffer[rowOffset];
            }
            else
            {
                stream << ".";
            }
        }

        stream << endl;
        offset += bytesInRow;
    }

    stream.fill(fillChar);
    stream.flags(old_settings);

    return stream;
}

#ifdef USE_GTEST

static const String testPhrase("This is a test");
static const fs::path tempFileName("./gtest_sptk5_buffer.tmp");

TEST(SPTK_Buffer, create)
{
    Buffer buffer1(testPhrase);
    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer1.capacity());
}

TEST(SPTK_Buffer, copyCtor)
{
    auto buffer1 = make_shared<Buffer>(testPhrase);
    Buffer buffer2(*buffer1);
    buffer1.reset();

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());
}

TEST(SPTK_Buffer, move)
{
    Buffer buffer1(testPhrase);
    Buffer buffer2(move(buffer1));
    buffer1.reset();

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());

    buffer1 = "Test 1";
    EXPECT_STREQ("Test 1", buffer1.c_str());

    buffer2 = testPhrase;
    buffer1 = move(buffer2);

    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
}

TEST(SPTK_Buffer, assign)
{
    Buffer buffer1(testPhrase);
    Buffer buffer2;

    buffer2 = buffer1;

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());

    buffer1 = "Test 1";
    EXPECT_STREQ("Test 1", buffer1.c_str());

    buffer1 = String("Test 2");
    EXPECT_STREQ("Test 2", buffer1.c_str());
}

TEST(SPTK_Buffer, append)
{
    Buffer buffer1;

    buffer1.append(testPhrase);

    EXPECT_STREQ(testPhrase.c_str(), buffer1.c_str());
    EXPECT_EQ(testPhrase.length(), buffer1.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer1.capacity());
}

TEST(SPTK_Buffer, saveLoadFile)
{
    Buffer buffer1(testPhrase);
    Buffer buffer2;

    buffer1.saveToFile(tempFileName);
    buffer2.loadFromFile(tempFileName);

    EXPECT_STREQ(testPhrase.c_str(), buffer2.c_str());
    EXPECT_EQ(testPhrase.length(), buffer2.bytes());
    EXPECT_TRUE(testPhrase.length() <= buffer2.capacity());
}

TEST(SPTK_Buffer, fill)
{
    Buffer buffer1;

    constexpr int repeatCharCount = 12;
    buffer1.fill('#', repeatCharCount);

    EXPECT_STREQ("############", buffer1.c_str());
    EXPECT_EQ(size_t(12), buffer1.bytes());
}

TEST(SPTK_Buffer, reset)
{
    Buffer buffer1(testPhrase);

    buffer1.reset();

    EXPECT_STREQ("", buffer1.c_str());
    EXPECT_EQ(size_t(0), buffer1.bytes());
    EXPECT_TRUE(buffer1.capacity() > 0);
}

TEST(SPTK_Buffer, erase)
{
    Buffer buffer1(testPhrase);

    constexpr int removeCharCount = 5;
    buffer1.erase(4, removeCharCount);

    EXPECT_STREQ("This test", buffer1.c_str());
}

TEST(SPTK_Buffer, compare)
{
    Buffer buffer1(testPhrase);
    Buffer buffer2(testPhrase);
    Buffer buffer3("something else");

    EXPECT_TRUE(buffer1 == buffer2);
    EXPECT_FALSE(buffer1 != buffer2);

    EXPECT_FALSE(buffer1 == buffer3);
    EXPECT_TRUE(buffer1 != buffer3);
}

TEST(SPTK_Buffer, textDump)
{
    Buffer buffer(testPhrase);
    buffer.append(testPhrase);

    stringstream stream;
    stream << buffer;

    Strings output(stream.str(), "\n\r", Strings::SplitMode::ANYCHAR);
    EXPECT_STREQ(stream.str().c_str(), buffer.c_str());
}

TEST(SPTK_Buffer, hexDump)
{
    const Strings expected {
        "00000000  54 68 69 73 20 69 73 20  61 20 74 65 73 74 54 68  This is  a testTh",
        "00000010  69 73 20 69 73 20 61 20  74 65 73 74              is is a  test"};

    Buffer buffer(testPhrase);
    buffer.append(testPhrase);

    stringstream stream;
    stream << hex << buffer;

    Strings output(stream.str(), "\n\r", Strings::SplitMode::ANYCHAR);
    EXPECT_TRUE(output == expected);
}

#endif
