/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTar.h  -  description
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

#ifndef __CTAR_H__
#define __CTAR_H__

#include <sptk5/sptk.h>
#include <sptk5/CException.h>
#include <sptk5/CStrings.h>
#include <sptk5/CBuffer.h>

namespace sptk {

/// @brief Tar memory handle
class CMemoryTarHandle {
public:
    size_t		position;         ///< Memory buffer position
    char*       sourceBuffer;     ///< Memory buffer
    size_t		sourceBufferLen;  ///< Memory buffer len
public:
    /// @brief Constructor
    /// @param buffer CBuffer*, source data
    CMemoryTarHandle(CBuffer* buffer=0) {
        position = 0;
        if (buffer) {
            sourceBuffer = buffer->data();
            sourceBufferLen = buffer->size();
        } else {
            sourceBuffer = 0;
            sourceBufferLen = 0;
        }
    }
};

typedef std::map<int,CMemoryTarHandle*> CTarHandleMap;

/// @brief A wrapper for libtar functions
///
/// Allows reading tar archive files into memory buffers.
/// The main usage currently is to read an SPTK theme from tar-archive.
class CTar {
    typedef std::map<std::string,CBuffer*>  CFileCollection;
    void*                 m_tar;         ///< Tar file header
    CFileCollection       m_files;       ///< File name to the file data map
    CStrings              m_fileNames;   ///< List of files in archive
    bool                  m_memoryRead;  ///< Flag to indicate if tar data is red from the memory buffer
    std::string           m_fileName;    ///< Tar file name

    /// @brief Loads tar file into memory
    bool loadFile() THROWS_EXCEPTIONS;

    /// @brief Throws an error
    void throwError(std::string fileName) THROWS_EXCEPTIONS;
public:
    static int            lastTarHandle; ///< The last generated tar handle
    static CTarHandleMap *tarHandleMap;  ///< The map of tar handles

        /// @brief Returns memory handle
    /// @param handle int, tar handle
    static CMemoryTarHandle* tarMemoryHandle(int handle);

    /// @brief Overwrites standard tar open
    static int mem_open(const char *name, int x, ...);

    /// @brief Overwrites standard tar close
    /// @param handle int, tar handle
    static int mem_close(int handle);

    /// @brief Overwrites standard tar read
    /// @param handle int, tar handle
    /// @param buf void*, data buffer
    /// @param len size_t, read size
    static int mem_read(int handle, void *buf, size_t len);

    /// @brief Overwrites standard tar write
    ///@param handle int, tar handle
    /// @param buf void*, data buffer
    /// @param len size_t, write size
    static int mem_write(int handle, const void *buf, size_t len);

public:
    /// @brief Constructor
    CTar();

    /// @brief Destructor
    ~CTar() { clear(); }

    /// @brief Reads tar archive from file
    ///
    /// The archive content is red into the internal set of buffers
    /// @param fileName std::string, file name to open
    void read(const std::string& fileName) THROWS_EXCEPTIONS {
        read(fileName.c_str());
    }

    /// @brief Reads tar archive from file
    ///
    /// The archive content is red into the internal set of buffers
    /// @param fileName std::string, file name to open
    void read(const char* fileName) THROWS_EXCEPTIONS;

    /// @brief Reads tar archive from buffer
    ///
    /// The archive content is red into the internal set of buffers
    /// @param tarData const CBuffer&, tar file buffer
    void read(const CBuffer& tarData) THROWS_EXCEPTIONS;

    /// @brief returns a list of files in tar archive
    const CStrings& fileList() const { return m_fileNames; }

    /// @brief Returns file data by file name
    /// @param fileName std::string, file name
    const CBuffer& file(std::string fileName) const THROWS_EXCEPTIONS;

    /// @brief Clears the allocated memory
    void clear();
};

}

#endif
