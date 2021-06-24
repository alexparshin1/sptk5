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

#include "libtar.h"
#include <sptk5/Tar.h>
#include <filesystem>
#include <fstream>

#if USE_GTEST

#include <sptk5/md5.h>
#include <sptk5/Printer.h>

#endif

using namespace std;
using namespace sptk;

Tar::Tar(const Buffer& tarData)
{
    read(tarData);
}

void Tar::throwError(const String& fileName)
{
    const char* ptr = strerror(errno);
    if (fileName.empty())
    {
        throw Exception(ptr);
    }
    throw Exception(fileName + ": " + string(ptr));
}

void Tar::clear()
{
    m_fileName = "";
    m_files.clear();
}

const Buffer& Tar::file(const String& fileName) const
{
    auto itor = m_files.find(fileName);
    if (itor == m_files.end())
    {
        throw Exception("File '" + fileName + "' isn't found", __FILE__, __LINE__);
    }
    return *itor->second;
}

void Tar::append(const SArchiveFile& file)
{
    // Note: Existing file is replaced, unlike regular tar
    m_files[file->fileName()] = file;
}

void Tar::read(const Buffer& tarData)
{
    m_files.clear();

    size_t offset = 0;
    while (offset < tarData.length())
    {
        if (!readNextFile(tarData, offset))
        {
            break;
        }
    }
}

template<typename Field>
unsigned readOctalNumber(Field& field)
{
    unsigned value;
    sscanf(field.data(), "%o", &value);
    return value;
}

bool Tar::readNextFile(const Buffer& buffer, size_t& offset)
{
    const auto* header = (const TarHeader*) (buffer.data() + offset);
    if (header->magic.data()[0] == 0)
    {
        // empty block at the end of file
        return false;
    }

    if (memcmp(header->magic.data(), "ustar ", 6) != 0)
    {
        throw Exception("Unsupported TAR format: Expecting ustar.");
    }
    offset += T_BLOCKSIZE;

    ArchiveFile::Type type = (ArchiveFile::Type) header->typeflag;

    size_t contentLength = 0;
    if (type == ArchiveFile::Type::REGULAR_FILE || type == ArchiveFile::Type::REGULAR_FILE2)
    {
        contentLength = readOctalNumber(header->size);
    }

    int mode = readOctalNumber(header->mode);
    int uid = readOctalNumber(header->uid);
    int gid = readOctalNumber(header->gid);

    time_t mtime = readOctalNumber(header->mtime);
    auto dt = DateTime::convertCTime(mtime);

    const uint8_t* content = buffer.data() + offset;

    String fname = header->name.data();
    String uname = header->uname.data();
    String gname = header->gname.data();
    String linkName = header->linkname.data();

    size_t blockCount = contentLength / T_BLOCKSIZE;
    if (blockCount * T_BLOCKSIZE < contentLength)
    {
        blockCount++;
    }

    auto file = make_shared<ArchiveFile>(fname, content, contentLength, mode, uid, gid, dt, type,
                                         uname, gname, linkName);

    m_files[fname] = file;

    offset += blockCount * T_BLOCKSIZE;

    return true;
}

void Tar::read(const char* tarFileName)
{
    Buffer tarData;
    tarData.loadFromFile(tarFileName);
    read(tarData);
}

void Tar::saveToFile(const String& tarFileName)
{
    ofstream archive(tarFileName);
    for (const auto&[fileName, fileData]: m_files)
    {
        auto archiveFile = dynamic_pointer_cast<ArchiveFile>(fileData);
        if (archiveFile)
        {
            const auto& header = *(const TarHeader*) archiveFile->header();
            archive.write((const char*) &header, T_BLOCKSIZE);
            if (archiveFile->length() > 0)
            {
                size_t paddingLength = T_BLOCKSIZE - archiveFile->length() % T_BLOCKSIZE;
                Buffer padding(paddingLength);
                archive.write(archiveFile->c_str(), archiveFile->length());
                archive.write(padding.c_str(), paddingLength);
            }
        }
    }
    archive.close();
}

#if USE_GTEST

static const string gtestTempDirectory("gtest_temp_directory3");
static const string file1_md5("2934e1a7ae11b11b88c9b0e520efd978");
static const string file2_md5("adb45e22bba7108bb4ad1b772ecf6b40");

TEST(SPTK_Tar, read)
{
    Tar tar;

    filesystem::create_directories(gtestTempDirectory.c_str());

    constexpr int TestFileBytes = 1000;

    Buffer file1;
    for (int i = 0; i < TestFileBytes; ++i)
    {
        file1.append((const char*) &i, sizeof(i));
    }
    file1.saveToFile(gtestTempDirectory + "/file1.txt");
    EXPECT_STREQ(file1_md5.c_str(), md5(file1).c_str());

    Buffer file2;
    for (int i = 0; i < TestFileBytes; ++i)
    {
        file2.append("ABCDEFG HIJKLMN OPQRSTUV\n");
    }
    file2.saveToFile(gtestTempDirectory + "/file2.txt");
    EXPECT_STREQ(file2_md5.c_str(), md5(file2).c_str());

    ASSERT_EQ(0, system(("tar cf gtest_temp.tar " + gtestTempDirectory).c_str()));
    EXPECT_NO_THROW(tar.read("gtest_temp.tar"));

    Buffer outfile1;
    Buffer outfile2;

    EXPECT_NO_THROW(outfile1 = tar.file(gtestTempDirectory + "/file1.txt"));
    EXPECT_NO_THROW(outfile2 = tar.file(gtestTempDirectory + "/file2.txt"));
    EXPECT_STREQ(file1_md5.c_str(), md5(outfile1).c_str());
    EXPECT_STREQ(file2_md5.c_str(), md5(outfile2).c_str());

#ifdef _WIN32
    EXPECT_EQ(0, system(("rmdir /s /q " + gtestTempDirectory).c_str()));
#else
    EXPECT_EQ(0, system(("rm -rf " + gtestTempDirectory).c_str()));
#endif
}

TEST(SPTK_Tar, load)
{
    Tar tar;
    tar.read("/tmp/1.bin");
    auto files = tar.fileList();
    COUT(files.join("\n") << endl)
}

TEST(SPTK_Tar, write)
{
    Tar tar;
    auto archiveFile = make_shared<ArchiveFile>("/tmp/3.lnk", "/tmp");
    tar.append(archiveFile);
    archiveFile = make_shared<ArchiveFile>("/tmp/1.txt", "/tmp");
    tar.append(archiveFile);
    /*
    archiveFile = make_shared<ArchiveFile>("/tmp/2.txt", "/tmp");
    tar.append(archiveFile);
    */
    tar.saveToFile("/tmp/2.bin");
}

#endif
