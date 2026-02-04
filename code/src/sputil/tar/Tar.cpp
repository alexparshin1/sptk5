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

#include <fstream>
#include <sptk5/Tar.h>

using namespace std;
using namespace sptk;

Tar::Tar(const Buffer& tarData)
{
    read(tarData);
}

Tar::Tar(const filesystem::path& fileName)
{
    read(fileName);
}

void Tar::clear()
{
    m_fileName = "";
    m_files.clear();
}

const ArchiveFile& Tar::file(const filesystem::path& fileName) const
{
    const auto itor = m_files.find(fileName);
    if (itor == m_files.end())
    {
        throw Exception("File '" + fileName.string() + "' isn't found");
    }
    return *itor->second;
}

void Tar::append(const SArchiveFile& file)
{
    // Note: Existing file is replaced, unlike regular tar
    filesystem::path filename = file->fileName().c_str();
    m_files[filename] = file;
}

void Tar::remove(const filesystem::path& fileName)
{
    m_files.erase(fileName);
}

void Tar::read(const Buffer& tarData)
{
    m_files.clear();

    size_t offset = 0;
    while (offset < tarData.size())
    {
        if (!readNextFile(tarData, offset))
        {
            break;
        }
    }
}

namespace {
template<typename T>
size_t readOctalNumber(const T& field, const String& fieldName)
{
    constexpr int octal = 8;

    size_t result = 0;
    string fieldData(field.data(), field.size());
    auto   [tail, ec] = from_chars(fieldData.data(), fieldData.data() + sizeof(field), result, octal);

    if (ec == errc())
    {
        return result;
    }

    if (ec == errc::result_out_of_range)
    {
        throw Exception(format("The value for {} in the file header '{}' is larger than size_t.", fieldName.c_str(), fieldData));
    }

    throw Exception(format("The value for {} in the file header '{}' is not a number.", fieldName.c_str(), fieldData));
}

template<size_t N>
string tarFieldToString(const array<char, N>& f)
{
    const auto end = find(f.begin(), f.end(), '\0');
    return string(f.begin(), end);
}
}

bool Tar::readNextFile(const Buffer& buffer, size_t& offset)
{
    if (offset + TAR_BLOCK_SIZE > buffer.size())
    {
        // No more data to read
        return false;
    }

    const auto* header = reinterpret_cast<const TarHeader*>(buffer.data() + offset);
    if (header->magic[0] == 0)
    {
        if (buffer.size() - offset < TAR_BLOCK_SIZE + TAR_BLOCK_SIZE)
        {
            throw Exception("Invalid padding in the TAR file (too short).");
        }

        constexpr array<char, 1024> padding{};
        if (memcmp(padding.data(), header, 1024) != 0)
        {
            throw Exception("Invalid padding in the TAR file (not zero-filled).");
        }

        // Two empty blocks at the end of the file:
        return false;
    }

    if (constexpr int magicLength = 5;
        memcmp(header->magic.data(), "ustar", magicLength) != 0)
    {
        throw Exception("Unsupported TAR format: Expecting ustar.");
    }
    offset += TAR_BLOCK_SIZE;

    auto type = static_cast<ArchiveFile::Type>(header->typeflag);

    size_t contentLength = 0;
    if (type == ArchiveFile::Type::REGULAR_FILE || type == ArchiveFile::Type::REGULAR_FILE2)
    {
        contentLength = readOctalNumber(header->size, "size");
    }

    if (offset + contentLength > buffer.size())
    {
        // Truncated data?
        throw Exception("Truncated TAR file data.");
    }

    auto       mode = static_cast<int>(readOctalNumber(header->mode, "mode"));
    const auto uid = static_cast<int>(readOctalNumber(header->uid, "uid"));
    const auto gid = static_cast<int>(readOctalNumber(header->gid, "gid"));

    const time_t mtime = static_cast<time_t>(readOctalNumber(header->mtime, "mtime"));
    auto         dateTime = DateTime::convertCTime(mtime);

    const Buffer content(buffer.data() + offset, contentLength);

    const filesystem::path fname(tarFieldToString(header->filename));
    const String           uname(tarFieldToString(header->uname));
    const String           gname(tarFieldToString(header->gname));
    const filesystem::path linkName(tarFieldToString(header->linkName));

    size_t blockCount = contentLength / TAR_BLOCK_SIZE;
    if (blockCount * TAR_BLOCK_SIZE < contentLength)
    {
        blockCount++;
    }

    const ArchiveFile::Ownership ownership{.uid = uid, .gid = gid, .uname = uname, .gname = gname};
    const auto                   file = make_shared<ArchiveFile>(fname, content, mode, dateTime, type, ownership, linkName);

    m_files[fname] = file;

    offset += blockCount * TAR_BLOCK_SIZE;

    return true;
}

void Tar::read(const filesystem::path& tarFileName)
{
    Buffer tarData;
    tarData.loadFromFile(tarFileName);
    read(tarData);
}

void Tar::save(const filesystem::path& tarFileName) const
{
    ofstream archive(tarFileName, ios::binary | ios::trunc);
    for (const auto& archiveFile: m_files | views::values)
    {
        const auto& header = *reinterpret_cast<const TarHeader*>(archiveFile->header());
        archive.write(reinterpret_cast<const char*>(&header), TAR_BLOCK_SIZE);
        if (!archiveFile->empty())
        {
            const size_t paddingLength = (TAR_BLOCK_SIZE - archiveFile->size() % TAR_BLOCK_SIZE) % TAR_BLOCK_SIZE;
            const Buffer padding(paddingLength);
            archive.write(archiveFile->c_str(), static_cast<streamsize>(archiveFile->size()));
            archive.write(padding.c_str(), static_cast<streamsize>(paddingLength));
        }
    }

    // Standard tar ends with two 512-byte zero blocks.
    constexpr array<char, 1024> padding{};
    archive.write(padding.data(), padding.size());
    archive.close();
}