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

#include <sptk5/Tar.h>
#include <filesystem>
#include <fstream>

#ifdef USE_GTEST

#include <sptk5/md5.h>
#include <sptk5/Printer.h>

#endif

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
        throw Exception("File '" + fileName + "' isn't found", __FILE__, __LINE__);
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
    while (offset < tarData.length())
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
    auto value = (unsigned) strtoul(field.data(), nullptr, octal);
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


    if (constexpr int magicLength = 6;
        memcmp(header->magic.data(), "ustar ", magicLength) != 0)
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

    int mode = readOctalNumber(header->mode, "mode");
    int uid = readOctalNumber(header->uid, "uid");
    int gid = readOctalNumber(header->gid, "gid");

    time_t mtime = readOctalNumber(header->mtime, "mtime");
    auto dt = DateTime::convertCTime(mtime);

    const Buffer content(buffer.data() + offset, contentLength);

    fs::path fname(header->filename.data());
    String uname(header->uname.data());
    String gname(header->gname.data());
    fs::path linkName(header->linkname.data());

    size_t blockCount = contentLength / TAR_BLOCK_SIZE;
    if (blockCount * TAR_BLOCK_SIZE < contentLength)
    {
        blockCount++;
    }

    ArchiveFile::Ownership ownership {uid, gid, uname, gname};
    auto file = make_shared<ArchiveFile>(fname, content, mode, dt, type, ownership, linkName);

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
    for (const auto&[fileName, archiveFile]: m_files)
    {
        const auto& header = *(const TarHeader*) archiveFile->header();
        archive.write((const char*) &header, TAR_BLOCK_SIZE);
        if (archiveFile->length() > 0)
        {
            size_t paddingLength = TAR_BLOCK_SIZE - archiveFile->length() % TAR_BLOCK_SIZE;
            Buffer padding(paddingLength);
            archive.write(archiveFile->c_str(), archiveFile->length());
            archive.write(padding.c_str(), paddingLength);
        }
    }
    archive.close();
}

#ifdef USE_GTEST

static const String file1_md5 {"2934e1a7ae11b11b88c9b0e520efd978"};
static const String file2_md5 {"adb45e22bba7108bb4ad1b772ecf6b40"};
static const String gtestTempDirectory {"gtest_temp_directory3"};
static const String testTar1 {"gtest_temp1.tar"};
static const String testTar2 {"gtest_temp2.tar"};

class SPTK_Tar
    : public ::testing::Test
{
protected:
    void SetUp() override
    {

        filesystem::create_directories(gtestTempDirectory.c_str());

        constexpr int TestFileBytes = 1000;

        Buffer file1;
        for (int i = 0; i < TestFileBytes; ++i)
        {
            file1.append((const char*) &i, sizeof(i));
        }
        file1.saveToFile(gtestTempDirectory + "/file1.txt");

        Buffer file2;
        for (int i = 0; i < TestFileBytes; ++i)
        {
            file2.append("ABCDEFG HIJKLMN OPQRSTUV\n");
        }
        file2.saveToFile(gtestTempDirectory + "/file2.txt");

        ASSERT_EQ(0, system(("tar cf " + testTar1 + " " + gtestTempDirectory).c_str()));
    }

    void TearDown() override
    {
        filesystem::remove_all(gtestTempDirectory.c_str());
        unlink(testTar1.c_str());
        unlink(testTar2.c_str());
        unlink("test.lst");
    }
};

TEST_F(SPTK_Tar, relativePath)
{
    auto relPath = ArchiveFile::relativePath("/tmp/mydir/myfile.txt", "/tmp/mydir");
    EXPECT_STREQ(relPath.string().c_str(), "myfile.txt");

    relPath = ArchiveFile::relativePath("/tmp/mydir1/mydir2/myfile.txt", "/tmp/mydir1");
    EXPECT_STREQ(relPath.string().c_str(), "mydir2/myfile.txt");

    relPath = ArchiveFile::relativePath("/tmp/mydir1/myfile.txt", "/tmp/mydir");
    EXPECT_STREQ(relPath.string().c_str(), "/tmp/mydir1/myfile.txt");
}

TEST_F(SPTK_Tar, read)
{
    Tar tar;

    EXPECT_NO_THROW(tar.read(testTar1));

    const auto& outfile1 = tar.file(gtestTempDirectory + "/file1.txt");
    const auto& outfile2 = tar.file(gtestTempDirectory + "/file2.txt");
    EXPECT_STREQ(file1_md5.c_str(), md5(outfile1).c_str());
    EXPECT_STREQ(file2_md5.c_str(), md5(outfile2).c_str());
}

TEST_F(SPTK_Tar, write)
{
    Tar tar;

    EXPECT_NO_THROW(tar.read(testTar1));
    EXPECT_NO_THROW(tar.save(testTar2));

    ASSERT_EQ(0, system(("tar tf " + testTar1 + " > test.lst").c_str()));
}

#endif
