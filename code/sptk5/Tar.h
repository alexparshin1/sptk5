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

#pragma once

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/Buffer.h>
#include <sptk5/ArchiveFile.h>

namespace sptk {

/**
 * Tar archive
 *
 * Allows reading tar archive files into memory buffers.
 * The main usage currently is to read an SPTK theme from tar-archive.
 */
class SP_EXPORT Tar
{
    using SBuffer = std::shared_ptr<Buffer>;
    using FileCollection = std::map<String, SArchiveFile>;

public:
    /**
     * Constructor
     */
    Tar() = default;

    /**
     * Constructor
     * @param tarData           Tar archive data
     */
    explicit Tar(const Buffer& tarData);

    /**
     * Constructor
     * @param tarData           Tar file name
     */
    explicit Tar(const String& tarFileName);

    /**
     * Reads tar archive from file
     *
     * The archive content is red into the internal set of buffers
     * @param fileName          File name to open
     */
    void read(const String& fileName)
    {
        read(fileName.c_str());
    }

    /**
     * Reads tar archive from file
     *
     * The archive content is red into the internal set of buffers
     * @param fileName          File name to open
     */
    void read(const char* fileName);

    /**
     * Reads tar archive from buffer
     *
     * The archive content is red into the internal set of buffers
     * @param tarData           Tar file buffer
     */
    void read(const Buffer& tarData);

    /**
     * returns a list of files in tar archive
     */
    Strings fileList() const
    {
        Strings fileNames;
        for (const auto&[fileName, data]: m_files)
        {
            fileNames.push_back(fileName);
        }
        return fileNames;
    }

    /**
     * Return file data by file name
     * @param fileName          File name
     * @return file data
     */
    const ArchiveFile& file(const String& fileName) const;

    /**
     * Remove file data
     * @param fileName          Archive file
     */
    void remove(const String& filename);

    /**
     * Add file data
     * @param fileName          Archive file
     */
    void append(const SArchiveFile& file);

    /**
     * Save tar archive to file
     * @param tarFileName          Tar file name
     */
    void save(const String& tarFileName) const;

    /**
     * Clears the allocated memory
     */
    void clear();

private:
    FileCollection m_files;       ///< File name to the file data map
    bool m_memoryRead {false};    ///< Flag to indicate if tar data is red from the memory buffer
    String m_fileName;            ///< Tar file name

    [[nodiscard]] bool readNextFile(const Buffer& buffer, size_t& offset);

    /**
     * Loads tar file into memory
     */
    void loadFile();
};

}
