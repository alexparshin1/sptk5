/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTar.cpp  -  description
                             -------------------
    begin                : Fri Sep 1 2006
    copyright            : © 1999-2021 Alexey Parshin.    All rights reserved.

    This module creation was sponsored by Total Knowledge (http://www.total-knowledge.com).
    Author thanks the developers of CPPSERV project (http://www.total-knowledge.com/progs/cppserv)
    for defining the requirements for this class.
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include "libtar.h"
#include <sptk5/Tar.h>

#ifdef _WIN32
#include <io.h>
#endif

#if USE_GTEST

#include <sptk5/md5.h>
#include <filesystem>
#include <fstream>

#endif

using namespace std;
using namespace sptk;

using CMemOpenCallback = int (*)(const char*, int, ...);
using CMemCloseCallback = int (*)(int);
using CMemReadCallback = int (*)(int, uint8_t*, size_t);
using CMemWriteCallback = int (*)(int, const uint8_t*, size_t);

#ifdef _MSC_VER
#define lseek _lseek
#endif

static const tartype_t memtype
    {
        (CMemOpenCallback) Tar::mem_open,
        (CMemCloseCallback) Tar::mem_close,
        (CMemReadCallback) Tar::mem_read,
        (CMemWriteCallback) Tar::mem_write
    };

int            Tar::lastTarHandle;
TarHandleMap   Tar::tarHandleMap;

MemoryTarHandle* Tar::tarMemoryHandle(int handle)
{
    auto itor = tarHandleMap.find(handle);
    if (itor == tarHandleMap.end())
    {
        return nullptr;
    }
    return itor->second.get();
}

int Tar::mem_open(const char*, int, const uint8_t*)
{
    ++lastTarHandle;
    tarHandleMap[lastTarHandle] = make_shared<MemoryTarHandle>();
    return lastTarHandle;
}

int Tar::mem_close(int handle)
{
    auto itor = tarHandleMap.find(handle);
    if (itor == tarHandleMap.end())
    {
        return -1;
    }
    tarHandleMap.erase(itor);
    return 0;
}

int Tar::mem_read(int x, uint8_t* buf, size_t len)
{
    MemoryTarHandle* memHandle = tarMemoryHandle(x);
    if (memHandle == nullptr)
    { return -1; }
    if (memHandle->position + len > memHandle->sourceBufferLen)
    {
        len = memHandle->sourceBufferLen - memHandle->position;
    }
    if (buf != nullptr)
    {
        memcpy(buf, memHandle->sourceBuffer + memHandle->position, len);
    }
    memHandle->position += (uint32_t) len;
    return (int) len;
}

int Tar::mem_write(int, const uint8_t*, size_t)
{
    return -1;
}

bool Tar::loadFile()
{
    auto* tar = m_tar.get();
    // Read file header
    int rc = th_read(tar);
    if (rc > 0)
    { return false; } // End of archive
    if (rc < 0) throwError(m_fileName);

    constexpr int MAX_PATH_LENGTH = 1024;
    array<char, MAX_PATH_LENGTH> path{};
    th_get_pathname(tar, path.data(), sizeof(path));
    String fileName(path.data());

    if (auto fileSize = (uint32_t) th_get_size(tar); fileSize != 0)
    {
        auto buffer = make_shared<Buffer>(size_t(fileSize) + 1);
        auto* buf = buffer->data();

        uint32_t offset = 0;
        while (offset != fileSize)
        {
            int k = tar->type->readfunc((int) tar->fd, buf + offset, unsigned(fileSize - offset));
            if (k < 0)
            {
                throw Exception("Error reading file '" + fileName + "' from tar archive");
            }
            offset += unsigned(k);
        }
        buf[fileSize] = '\0';
        buffer->bytes(fileSize);

        m_fileNames.push_back(fileName);
        m_files[fileName] = buffer;

        auto emptyTail = off_t(T_BLOCKSIZE - fileSize % T_BLOCKSIZE);
        array<char, T_BLOCKSIZE> buff;
        if (m_memoryRead)
        {
            mem_read((int) tar->fd, nullptr, size_t(emptyTail));
        }
        else
        {
            if (::read((int) tar->fd, buff.data(), size_t(emptyTail)) == -1)
                throwError(m_fileName);
        }
    }
    return true;
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

void Tar::read(const char* fileName)
{
    m_fileName = fileName;
    m_memoryRead = false;
    clear();
    m_tar = tar_open(fileName, nullptr, 0, 0, TAR_GNU);
    while (loadFile())
    {}
    m_tar.reset();
}

void Tar::read(const Buffer& tarData)
{
    m_fileName = "";
    m_memoryRead = true;
    clear();
    m_tar = tar_open("memory", &memtype, 0, 0, TAR_GNU);
    auto* memHandle = tarMemoryHandle((int) m_tar->fd);
    if (memHandle == nullptr)
    {
        m_tar.reset();
        throw Exception("Can't open the archive", __FILE__, __LINE__);
    }
    memHandle->sourceBuffer = (char*) tarData.data();
    memHandle->sourceBufferLen = tarData.bytes();
    while (loadFile())
    {}
    m_tar.reset();
}

void Tar::clear()
{
    m_fileName = "";
    m_files.clear();
    m_fileNames.clear();
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
    auto itor = m_files.find(file->fileName());
    if (itor == m_files.end())
    {
        m_fileNames.push_back(file->fileName());
    }
    m_files[file->fileName()] = file;
}

template <typename T>
static void writeField(ofstream& file, const T& field)
{
    file.write(field.data(), sizeof(field));
}

void Tar::saveToFile(const String& archiveFileName)
{
    ofstream archive(archiveFileName);
    for (const auto& fileName: m_fileNames) {
        auto fileData = m_files[fileName];
        auto archiveFile = dynamic_pointer_cast<ArchiveFile>(fileData);
        if (archiveFile) {
            const auto& header = *(const tar_header *)archiveFile->header();
            writeField(archive, header.name);
            writeField(archive, header.mode);
            writeField(archive, header.uid);
            writeField(archive, header.gid);
            writeField(archive, header.size);
            writeField(archive, header.mtime);
            writeField(archive, header.chksum);
            archive << header.typeflag;
            writeField(archive, header.linkname);
            writeField(archive, header.magic);
            writeField(archive, header.version);
            writeField(archive, header.uname);
            writeField(archive, header.gname);
            writeField(archive, header.devmajor);
            writeField(archive, header.devminor);
            writeField(archive, header.prefix);
            writeField(archive, header.padding);

            size_t paddingLength = T_BLOCKSIZE - archiveFile->length() % T_BLOCKSIZE;
            Buffer padding(paddingLength);
            archive.write(archiveFile->c_str(), archiveFile->length());
            archive.write(padding.c_str(), paddingLength);
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

TEST(SPTK_Tar, write)
{
    Tar tar;
    auto archiveFile = make_shared<ArchiveFile>("/tmp/1.txt", "/tmp");
    tar.append(archiveFile);
    archiveFile = make_shared<ArchiveFile>("/tmp/2.txt", "/tmp");
    tar.append(archiveFile);
    tar.saveToFile("/tmp/2.bin");
}

#endif
