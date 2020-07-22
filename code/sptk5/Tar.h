/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
 * Tar memory handle
 */
class MemoryTarHandle 
{
public:
    /**
     * Memory buffer position
     */
    size_t      position {0};

    /**
     * Memory buffer
     */
    char*       sourceBuffer {nullptr};

    /**
     * Memory buffer len
     */
    size_t      sourceBufferLen {0};

    /**
     * Constructor
     * @param buffer CBuffer*, source data
     */
    explicit MemoryTarHandle(const Buffer* buffer=nullptr)
    {
        if (buffer) {
            sourceBuffer = buffer->data();
            sourceBufferLen = buffer->bytes();
        }
    }
};

typedef std::map<int, MemoryTarHandle*> TarHandleMap;

/**
 * A wrapper for libtar functions
 *
 * Allows reading tar archive files into memory buffers.
 * The main usage currently is to read an SPTK theme from tar-archive.
 */
class SP_EXPORT Tar
{
    typedef std::map<String,Buffer>  FileCollection;

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
     * Returns memory handle
     * @param handle int, tar handle
     */
    static MemoryTarHandle* tarMemoryHandle(int handle);

    /**
     * Overwrites standard tar open
     */
    static int mem_open(const char *name, int x, ...);

    /**
     * Overwrites standard tar close
     * @param handle int, tar handle
     */
    static int mem_close(int handle);

    /**
     * Overwrites standard tar read
     * @param handle int, tar handle
     * @param buf void*, data buffer
     * @param len size_t, read size
     */
    static int mem_read(int handle, void *buf, size_t len);

    /**
     * Overwrites standard tar write
     *@param handle int, tar handle
     * @param buf void*, data buffer
     * @param len size_t, write size
     */
    static int mem_write(int handle, const void *buf, size_t len);

    /**
     * Constructor
     */
    Tar();

    Tar(const Tar&) = delete;
    Tar(Tar&&) noexcept = default;
    Tar& operator = (const Tar&) = delete;
    Tar& operator = (Tar&&) noexcept = default;

    /**
     * Destructor
     */
    ~Tar() { clear(); }

    /**
     * Reads tar archive from file
     *
     * The archive content is red into the internal set of buffers
     * @param fileName std::string, file name to open
     */
    void read(const std::string& fileName)
    {
        read(fileName.c_str());
    }

    /**
     * Reads tar archive from file
     *
     * The archive content is red into the internal set of buffers
     * @param fileName std::string, file name to open
     */
    void read(const char* fileName);

    /**
     * Reads tar archive from buffer
     *
     * The archive content is red into the internal set of buffers
     * @param tarData const CBuffer&, tar file buffer
     */
    void read(const Buffer& tarData);

    /**
     * returns a list of files in tar archive
     */
    const Strings& fileList() const { return m_fileNames; }

    /**
     * Returns file data by file name
     * @param fileName std::string, file name
     */
    const Buffer& file(std::string fileName) const;

    /**
     * Clears the allocated memory
     */
    void clear();

private:

    void*           m_tar {nullptr};      ///< Tar file header
    FileCollection  m_files;              ///< File name to the file data map
    Strings         m_fileNames;          ///< List of files in archive
    bool            m_memoryRead {false}; ///< Flag to indicate if tar data is red from the memory buffer
    String          m_fileName;           ///< Tar file name

    /**
     * Loads tar file into memory
     */
    bool loadFile();

    /**
     * Throws an error
     */
    [[noreturn]] static void throwError(std::string fileName);
};

}

#endif
