/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTar.cpp  -  description
                             -------------------
    begin                : Fri Sep 1 2006
    copyright            : (C) 1999-2018 by Alexey Parshin. All rights reserved.

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

using namespace std;
using namespace sptk;

extern "C" {
typedef int (* CMemOpenCallback)(const char*, int, ...);
typedef int (* CMemCloseCallback)(int);
typedef int (* CMemReadCallback)(int, void*, size_t);
typedef int (* CMemWriteCallback)(int, const void*, size_t);
}

#ifdef _MSC_VER
#define lseek _lseek
#endif

static tartype_t memtype;
int            Tar::lastTarHandle;
TarHandleMap* Tar::tarHandleMap;

MemoryTarHandle* Tar::tarMemoryHandle(int handle)
{
    auto itor = tarHandleMap->find(handle);
    if (itor == tarHandleMap->end())
        return nullptr;
    return itor->second;
}

int Tar::mem_open(const char*, int, ...)
{
    lastTarHandle++;
    auto memHandle = new MemoryTarHandle;
    (*tarHandleMap)[lastTarHandle] = memHandle;
    return lastTarHandle;
}

int Tar::mem_close(int handle)
{
    auto itor = tarHandleMap->find(handle);
    if (itor == tarHandleMap->end())
        return -1;
    MemoryTarHandle* memHandle = itor->second;
    delete memHandle;
    tarHandleMap->erase(itor);
    return 0;
}

int Tar::mem_read(int x, void* buf, size_t len)
{
    MemoryTarHandle* memHandle = tarMemoryHandle(x);
    if (memHandle == nullptr) return -1;
    if (memHandle->position + len > memHandle->sourceBufferLen)
        len = memHandle->sourceBufferLen - memHandle->position;
    if (buf != nullptr)
        memcpy(buf, memHandle->sourceBuffer + memHandle->position, len);
    memHandle->position += (uint32_t) len;
    return (int) len;
}

int Tar::mem_write(int, const void*, size_t)
{
    return -1;
}

Tar::Tar()
: m_memoryRead(false)
{
    m_tar = nullptr;
    if (tarHandleMap == nullptr) {
        memtype.openfunc = (CMemOpenCallback) mem_open;
        memtype.closefunc = (CMemCloseCallback) mem_close;
        memtype.readfunc = (CMemReadCallback) mem_read;
        memtype.writefunc = (CMemWriteCallback) mem_write;
        tarHandleMap = new TarHandleMap;
    }
}

bool Tar::loadFile()
{
    auto tar = (TAR*) m_tar;
    // Read file header
    int rc = th_read(tar);
    if (rc > 0) return false; // End of archive
    if (rc < 0) throwError(m_fileName);

	char path[1024];
    th_get_pathname(tar, path, sizeof(path));
	string fileName(path);
    auto fileSize = (uint32_t) th_get_size(tar);

    if (fileSize != 0) {
        auto buffer = new Buffer(fileSize + 1);
        char* buf = buffer->data();

        uint32_t offset = 0;
        while (offset != fileSize) {
            int k = tar->type->readfunc((int) tar->fd, buf + offset, unsigned(fileSize - offset));
            if (k < 0) {
                delete buffer;
                throw Exception("Error reading file '" + fileName + "' from tar archive");
            }
            offset += unsigned(k);
        }
        buf[fileSize] = '\0';
        buffer->bytes(fileSize);

        m_fileNames.push_back(fileName);
        m_files[fileName] = buffer;

        auto emptyTail = off_t(T_BLOCKSIZE - fileSize % T_BLOCKSIZE);
        char buff[T_BLOCKSIZE];
        if (m_memoryRead) {
            mem_read((int) tar->fd, nullptr, size_t(emptyTail));
        } else {
            if (::read((int) tar->fd, buff, size_t(emptyTail)) == -1)
                //if (lseek(tar->fd,emptyTail,SEEK_CUR) == (off_t) -1)
                throwError(m_fileName);
        }
    }
    return true;
}

void Tar::throwError(string fileName)
{
    char* ptr = strerror(errno);
    if (fileName.empty())
        throw Exception(ptr);
    throw Exception(fileName + ": " + string(ptr));
}

void Tar::read(const char* fileName)
{
    m_fileName = fileName;
    m_memoryRead = false;
    TAR* tar;
    clear();
    int rc = tar_open(&tar, (char*) fileName, nullptr, 0, 0, TAR_GNU);
    if (rc < 0) throwError(fileName);
    m_tar = tar;
    while (loadFile()) { }
    tar_close((TAR*) m_tar);
    m_tar = nullptr;
}

void Tar::read(const Buffer& tarData)
{
    m_fileName = "";
    m_memoryRead = true;
    TAR* tar;
    clear();
    tar_open(&tar, (char*) "memory", &memtype, 0, 0, TAR_GNU);
    MemoryTarHandle* memHandle = tarMemoryHandle((int) tar->fd);
    if (memHandle == nullptr)
        throw Exception("Can't open the archive", __FILE__, __LINE__);
    memHandle->sourceBuffer = tarData.data();
    memHandle->sourceBufferLen = tarData.bytes();
    m_tar = tar;
    while (loadFile()) { }
    tar_close((TAR*) m_tar);
    m_tar = nullptr;
}

void Tar::clear()
{
    for (auto m_file : m_files)
        delete m_file.second;
    m_fileName = "";
    m_files.clear();
    m_fileNames.clear();
}

const Buffer& Tar::file(std::string fileName) const
{
    auto itor = m_files.find(fileName);
    if (itor == m_files.end())
        throw Exception("File '" + fileName + "' isn't found", __FILE__, __LINE__);
    return *(itor->second);
}

#if USE_GTEST
#include <gtest/gtest.h>
#include <fstream>
#include <sptk5/md5.h>

static const string gtestTempDirectory("gtest_temp_directory3");
static const string file1_md5("2934e1a7ae11b11b88c9b0e520efd978");
static const string file2_md5("adb45e22bba7108bb4ad1b772ecf6b40");

TEST(SPTK_Tar, read)
{
    Tar tar;

    ASSERT_EQ(0, system(("mkdir " + gtestTempDirectory).c_str()));

    Buffer file1;
    for (int i = 0; i < 1000; i++) {
        file1.append((const char*)&i, sizeof(i));
    }
    file1.saveToFile(gtestTempDirectory + "/file1.txt");
    EXPECT_STREQ(file1_md5.c_str(), md5(file1).c_str());

    Buffer file2;
    for (int i = 0; i < 1000; i++) {
        file2.append("ABCDEFG HIJKLMN OPQRSTUV\n");
    }
    file2.saveToFile(gtestTempDirectory + "/file2.txt");
    EXPECT_STREQ(file2_md5.c_str(), md5(file2).c_str());

    ASSERT_EQ(0, system(("tar cf gtest_temp.tar " + gtestTempDirectory).c_str()));
    EXPECT_NO_THROW(tar.read("gtest_temp.tar"));

    Buffer outfile1, outfile2;
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

#endif
