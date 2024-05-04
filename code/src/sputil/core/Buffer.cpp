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

#include <iomanip>
#include <sptk5/Buffer.h>
#include <sptk5/SystemException.h>
#include <sys/stat.h>

using namespace std;
using namespace sptk;

Buffer::Buffer(string_view str)
    : BufferStorage(bit_cast<const uint8_t*>(str.data()), str.length())
{
}

void Buffer::loadFromFile(const std::filesystem::path& fileName)
{
    FILE* file = fopen(fileName.string().c_str(), "rb");

    if (file == nullptr)
    {
        throw SystemException("Can't open file " + fileName.string() + " for reading");
    }

    struct stat fileStat = {};
    if (fstat(fileno(file), &fileStat) != 0)
    {
        fclose(file);
        throw Exception("Can't get file size for '" + fileName.string() + "'");
    }

    const auto size = static_cast<size_t>(fileStat.st_size);

    reset(size + 1);
    bytes(fread(data(), 1, size, file));
    const auto result = fclose(file);
    if (result != 0)
    {
        throw SystemException("Can't close file " + fileName.string());
    }
}

void Buffer::saveToFile(const std::filesystem::path& fileName) const
{
    FILE* file = fopen(fileName.string().c_str(), "wb");

    if (file == nullptr)
    {
        throw SystemException("Can't open file " + fileName.string() + " for writing");
    }

    const auto rc1 = fwrite(data(), bytes(), 1, file);
    const auto rc2 = fclose(file);
    if (rc1 != 1 || rc2 != 0)
    {
        throw SystemException("Can't close file " + fileName.string());
    }
}

Buffer& Buffer::operator=(const String& other)
{
    set(bit_cast<const uint8_t*>(other.c_str()), other.length());

    return *this;
}

Buffer& Buffer::operator=(const char* str)
{
    set(bit_cast<const uint8_t*>(str), strlen(str));

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

ostream& sptk::operator<<(ostream& stream, const Buffer& buffer)
{
    if ((stream.flags() & ios::hex) == 0)
    {
        stream << buffer.c_str();
        return stream;
    }

    const char fillChar = stream.fill('0');
    const auto old_settings = stream.flags();

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
            const unsigned printChar = buffer[rowOffset];
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
            const auto testChar = buffer[rowOffset];
            if (testChar >= ' ')
            {
                stream << buffer[rowOffset];
            }
            else
            {
                stream << ".";
            }
        }

        stream << '\n';
        offset += bytesInRow;
    }

    stream.fill(fillChar);
    stream.flags(old_settings);

    return stream;
}
