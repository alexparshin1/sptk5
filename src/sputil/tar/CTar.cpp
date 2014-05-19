/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTar.cpp  -  description
                             -------------------
    begin                : Fri Sep 1 2006
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.

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

#include <sptk5/CTar.h>
#include <errno.h>
#include <string.h>
#include "libtar.h"
#include <sptk5/CException.h>

#ifdef __UNIX_COMPILER__
#include <unistd.h>
#else
#include <io.h>
#endif

using namespace std;
using namespace sptk;

extern "C" {
   typedef int (*CMemOpenCallback)  (const char*,int,...);
   typedef int (*CMemCloseCallback) (int);
   typedef int (*CMemReadCallback)  (int,void*,size_t);
   typedef int (*CMemWriteCallback) (int,const void*,size_t);
}

#ifdef _MSC_VER
#define lseek _lseek
#endif

static tartype_t memtype;
int            CTar::lastTarHandle;
CTarHandleMap *CTar::tarHandleMap;

CMemoryTarHandle* CTar::tarMemoryHandle(int handle) {
    CTarHandleMap::iterator itor = tarHandleMap->find(handle);
    if (itor == tarHandleMap->end())
        return 0;
    return itor->second;
}

int CTar::mem_open(const char *name, int x, ...) {
    lastTarHandle++;
    CMemoryTarHandle* memHandle = new CMemoryTarHandle;
    (*tarHandleMap)[lastTarHandle] = memHandle;
    return lastTarHandle;
}

int CTar::mem_close(int handle) {
    CTarHandleMap::iterator itor = tarHandleMap->find(handle);
    if (itor == tarHandleMap->end())
        return -1;
    CMemoryTarHandle* memHandle = itor->second;
    delete memHandle;
    tarHandleMap->erase(itor);
    return 0;
}

int CTar::mem_read(int x, void *buf, size_t len) {
    CMemoryTarHandle* memHandle = tarMemoryHandle(x);
    if (!memHandle) return -1;
    if (memHandle->position + len > memHandle->sourceBufferLen)
        len = memHandle->sourceBufferLen - memHandle->position;
    if (buf)
        memcpy(buf, memHandle->sourceBuffer + memHandle->position, len);
    memHandle->position += (uint32_t)len;
    return (int) len;
}

int CTar::mem_write(int x, const void *buf, size_t len) {
    return -1;
}

CTar::CTar() {
    m_tar = 0;
    if (!tarHandleMap) {
        memtype.openfunc  = (CMemOpenCallback)  mem_open;
        memtype.closefunc = (CMemCloseCallback) mem_close;
        memtype.readfunc  = (CMemReadCallback)  mem_read;
        memtype.writefunc = (CMemWriteCallback) mem_write;
        tarHandleMap = new CTarHandleMap;
    }
}

bool CTar::loadFile() THROWS_EXCEPTIONS {
    TAR* tar = (TAR*)m_tar;
    // Read file header
    int rc = th_read(tar);
    if (rc > 0) return false; // End of archive
    if (rc < 0) throwError(m_fileName);

    string fileName = th_get_pathname(tar);
    uint32_t fileSize = (uint32_t) th_get_size(tar);

    if (fileSize) {
        CBuffer *buffer = new CBuffer(fileSize + 1);
        char *buf = buffer->data();

        uint32_t offset = 0;
        while (offset != fileSize) {
            int k = tar->type->readfunc((int)tar->fd, buf + offset, unsigned(fileSize - offset));
            if (k < 0) {
                delete buffer;
                throw CException("Error reading file '"+fileName+"' from tar archive");
            }
            offset += unsigned(k);
        }
        buf[fileSize] = '\0';
        buffer->bytes(fileSize);

        m_fileNames.push_back(fileName);
        m_files[fileName] = buffer;

        off_t emptyTail = off_t(T_BLOCKSIZE - fileSize % T_BLOCKSIZE);
        char buff[T_BLOCKSIZE];
        if (m_memoryRead) {
            mem_read((int)tar->fd, NULL, size_t(emptyTail));
        } else {
            if (::read((int)tar->fd, buff, size_t(emptyTail)) == -1)
            //if (lseek(tar->fd,emptyTail,SEEK_CUR) == (off_t) -1)
                throwError(m_fileName);
        }
    }
    return true;
}

void CTar::throwError(string fileName) THROWS_EXCEPTIONS {
    char* ptr = strerror(errno);
    if (fileName.empty())
        throw CException(ptr);
    throw CException(fileName+": " + string(ptr));
}

void CTar::read(const char* fileName) THROWS_EXCEPTIONS {
    m_fileName = fileName;
    m_memoryRead = false;
    TAR* tar;
    clear();
    int rc = tar_open(&tar,(char*)fileName,0,0,0,TAR_GNU);
    if (rc < 0) throwError(fileName);
    m_tar = tar;
    while (loadFile()) {}
    tar_close((TAR*)m_tar);
    m_tar = 0;
}

void CTar::read(const CBuffer& tarData) THROWS_EXCEPTIONS {
    m_fileName = "";
    m_memoryRead = true;
    TAR* tar;
    clear();
    tar_open(&tar,(char *)"memory",&memtype,0,0,TAR_GNU);
    CMemoryTarHandle* memHandle = tarMemoryHandle((int)tar->fd);
    if (!memHandle)
        throw CException("Can't open the archive",__FILE__,__LINE__);
    memHandle->sourceBuffer = tarData.data();
    memHandle->sourceBufferLen = tarData.bytes();
    m_tar = tar;
    while (loadFile()) {}
    tar_close((TAR*)m_tar);
    m_tar = 0;
}

void CTar::clear() {
    for (CFileCollection::iterator itor = m_files.begin(); itor != m_files.end(); itor++)
        delete itor->second;
    m_fileName = "";
    m_files.clear();
    m_fileNames.clear();
}

const CBuffer& CTar::file(std::string fileName) const THROWS_EXCEPTIONS {
    CFileCollection::const_iterator itor = m_files.find(fileName);
    if (itor == m_files.end())
        throw CException("File '"+fileName+"' isn't found",__FILE__,__LINE__);
    return *(itor->second);
}
