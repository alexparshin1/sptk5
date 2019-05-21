/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MemoryMappedFile.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday May 19 2019                                     ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __PERSIST_MEMORY_BLOCK_H__
#define __PERSIST_MEMORY_BLOCK_H__

#include <sptk5/sptk.h>
#include <sptk5/cutils>

namespace smq {
namespace persistent {

/**
 * Persistent memory block, associated with memory-mapped file
 */
class SP_EXPORT MemoryMappedFile
{
#ifndef _WIN32
    static constexpr int INVALID_HANDLE_VALUE {-1};  ///< Invalid handle value
#endif

public:
    /**
     * Constructor
     * @param fileName          Memory mapped file name
     * @param fileSize          Size of memory mapped file and block of memory
     */
    MemoryMappedFile(const std::string& fileName, size_t fileSize);

    /**
     * Destructor
     */
    virtual ~MemoryMappedFile();

    /**
     * Open existing memory mapped file, or create new one
     */
    void open();

    /**
     * Close memory mapped file
     */
    void close();

    /**
     * Size of memory block
     * @return memory block size in bytes
     */
    uint64_t size() const
    {
        return m_fileSize;
    }

    sptk::String fileName() const
    {
        return m_fileName;
    }

private:

    static size_t       m_allocationUnit;                       ///< Allocation unit granularity

    sptk::String        m_fileName;                             ///< Memory mapped file name
    uint64_t            m_fileSize {0};                         ///< Memory mapped file size, same as memory block size

#ifdef _WIN32
    HANDLE              m_file {INVALID_HANDLE_VALUE};          ///< Memory mapped file handle
    HANDLE              m_mapFile {INVALID_HANDLE_VALUE};       ///< Handle for the file's memory-mapped region
#else
    int m_file{INVALID_HANDLE_VALUE};                           ///< Memory mapped file handle
#endif

    void*               m_data {nullptr};                       ///< Mapped memory

    /**
     * Open existing file, or create a new one
     */
    void createOrOpenFile();

    /**
     * Map file to memory
     */
    void createFileMapping();

public:

    /**
     * @return Mapped memory
     */
    void* data()
    {
        return m_data;
    }

    /**
     * @return Mapped memory
     */
    const void* data() const
    {
        return m_data;
    }
};

}
}

#endif
