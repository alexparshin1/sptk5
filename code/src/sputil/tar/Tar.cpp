/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

Tar::Tar(const String& fileName)
{
    read(fileName);
}

void Tar::clear()
{
    m_fileName = "";
    m_files.clear();
}

const ArchiveFile& Tar::file(const String& fileName) const
{
    auto itor = m_files.find(fileName);
    if (itor == m_files.end())
    {
        throw Exception("File '" + fileName + "' isn't found");
    }
    return *itor->second;
}

void Tar::append(const SArchiveFile& file)
{
    // Note: Existing file is replaced, unlike regular tar
    m_files[file->fileName()] = file;
}

void Tar::remove(const String& fileName)
{
    // Note: Existing file is replaced, unlike regular tar
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

template<typename Field>
unsigned readOctalNumber(Field& field, const String& fieldName)
{
    constexpr int octal = 8;
    errno = 0;
    auto value = (unsigned) strtoul(data(field), nullptr, octal);
    if (errno != 0)
    {
        throw Exception("Invalid octal number for " + fieldName);
    }
    return value;
}

bool Tar::readNextFile(const Buffer& buffer, size_t& offset)
{
    const auto* header = (const TarHeader*) (buffer.data() + offset);
    if (header->magic[0] == 0)
    {
        // empty block at the end of file
        return false;
    }


    if (constexpr int magicLength = 5;
        memcmp(header->magic.data(), "ustar", magicLength) != 0)
    {
        throw Exception("Unsupported TAR format: Expecting ustar.");
    }
    offset += TAR_BLOCK_SIZE;

    auto type = (ArchiveFile::Type) header->typeflag;

    size_t contentLength = 0;
    if (type == ArchiveFile::Type::REGULAR_FILE || type == ArchiveFile::Type::REGULAR_FILE2)
    {
        contentLength = readOctalNumber(header->size, "size");
    }

    auto mode = (int) readOctalNumber(header->mode, "mode");
    auto uid = (int) readOctalNumber(header->uid, "uid");
    auto gid = (int) readOctalNumber(header->gid, "gid");

    const time_t mtime = readOctalNumber(header->mtime, "mtime");
    auto dateTime = DateTime::convertCTime(mtime);

    const Buffer content(buffer.data() + offset, contentLength);

    const std::filesystem::path fname(header->filename.data());
    const String uname(header->uname.data());
    const String gname(header->gname.data());
    const std::filesystem::path linkName(header->linkName.data());

    size_t blockCount = contentLength / TAR_BLOCK_SIZE;
    if (blockCount * TAR_BLOCK_SIZE < contentLength)
    {
        blockCount++;
    }

    const ArchiveFile::Ownership ownership {uid, gid, uname, gname};
    auto file = make_shared<ArchiveFile>(fname, content, mode, dateTime, type, ownership, linkName);

    m_files[fname.string()] = file;

    offset += blockCount * TAR_BLOCK_SIZE;

    return true;
}

void Tar::read(const char* tarFileName)
{
    Buffer tarData;
    tarData.loadFromFile(tarFileName);
    read(tarData);
}

void Tar::save(const String& tarFileName) const
{
    ofstream archive(tarFileName);
    for (const auto& [fileName, archiveFile]: m_files)
    {
        const auto& header = *(const TarHeader*) archiveFile->header();
        archive.write((const char*) &header, TAR_BLOCK_SIZE);
        if (!archiveFile->empty())
        {
            const size_t paddingLength = TAR_BLOCK_SIZE - archiveFile->size() % TAR_BLOCK_SIZE;
            const Buffer padding(paddingLength);
            archive.write(archiveFile->c_str(), (int) archiveFile->size());
            archive.write(padding.c_str(), (int) paddingLength);
        }
    }
    archive.close();
}
