/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTar.h - description                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CTAR_H__
#define __CTAR_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/Buffer.h>

namespace sptk {

/**
 * @brief Tar memory handle
 */
class MemoryTarHandle 
{
public:
    /**
     * Memory buffer position
     */
    size_t      position;

    /**
     * Memory buffer
     */
    char*       sourceBuffer;

    /**
     * Memory buffer len
     */
    size_t      sourceBufferLen;

public:
    /**
     * @brief Constructor
     * @param buffer CBuffer*, source data
     */
    MemoryTarHandle(Buffer* buffer=0) {
        position = 0;
        if (buffer) {
            sourceBuffer = buffer->data();
            sourceBufferLen = buffer->bytes();
        } else {
            sourceBuffer = 0;
            sourceBufferLen = 0;
        }
    }
};

typedef std::map<int, MemoryTarHandle*> TarHandleMap;

/**
 * @brief A wrapper for libtar functions
 *
 * Allows reading tar archive files into memory buffers.
 * The main usage currently is to read an SPTK theme from tar-archive.
 */
class Tar
{
    typedef std::map<std::string,Buffer*>  FileCollection;
    /**
     * Tar file header
     */
    void*                 m_tar;

    /**
     * File name to the file data map
     */
    FileCollection       m_files;

    /**
     * List of files in archive
     */
    Strings               m_fileNames;

    /**
     * Flag to indicate if tar data is red from the memory buffer
     */
    bool                  m_memoryRead;

    /**
     * Tar file name
     */
    std::string           m_fileName;


    /**
     * @brief Loads tar file into memory
     */
    bool loadFile();

    /**
     * @brief Throws an error
     */
    void throwError(std::string fileName);
public:
    /**
     * The last generated tar handle
     */
    static int            lastTarHandle;

    /**
     * The map of tar handles
     */
    static TarHandleMap*  tarHandleMap;


        /**
         * @brief Returns memory handle
     * @param handle int, tar handle
         */
    static MemoryTarHandle* tarMemoryHandle(int handle);

    /**
     * @brief Overwrites standard tar open
     */
    static int mem_open(const char *name, int x, ...);

    /**
     * @brief Overwrites standard tar close
     * @param handle int, tar handle
     */
    static int mem_close(int handle);

    /**
     * @brief Overwrites standard tar read
     * @param handle int, tar handle
     * @param buf void*, data buffer
     * @param len size_t, read size
     */
    static int mem_read(int handle, void *buf, size_t len);

    /**
     * @brief Overwrites standard tar write
     *@param handle int, tar handle
     * @param buf void*, data buffer
     * @param len size_t, write size
     */
    static int mem_write(int handle, const void *buf, size_t len);

public:
    /**
     * @brief Constructor
     */
    Tar();

    /**
     * @brief Destructor
     */
    ~Tar() { clear(); }

    /**
     * @brief Reads tar archive from file
     *
     * The archive content is red into the internal set of buffers
     * @param fileName std::string, file name to open
     */
    void read(const std::string& fileName)
    {
        read(fileName.c_str());
    }

    /**
     * @brief Reads tar archive from file
     *
     * The archive content is red into the internal set of buffers
     * @param fileName std::string, file name to open
     */
    void read(const char* fileName);

    /**
     * @brief Reads tar archive from buffer
     *
     * The archive content is red into the internal set of buffers
     * @param tarData const CBuffer&, tar file buffer
     */
    void read(const Buffer& tarData);

    /**
     * @brief returns a list of files in tar archive
     */
    const Strings& fileList() const { return m_fileNames; }

    /**
     * @brief Returns file data by file name
     * @param fileName std::string, file name
     */
    const Buffer& file(std::string fileName) const;

    /**
     * @brief Clears the allocated memory
     */
    void clear();
};

}

#endif
