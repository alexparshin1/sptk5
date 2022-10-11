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

void Buffer::loadFromFile(const fs::path& fileName)
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
