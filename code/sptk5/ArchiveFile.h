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

#ifndef SPTK_ARCHIVEFILE_H
#define SPTK_ARCHIVEFILE_H

#include <sptk5/Buffer.h>
#include <sptk5/DateTime.h>

namespace sptk {

constexpr int TAR_BLOCK_SIZE = 512;     ///< Tar archive block size

#pragma pack(push, 1)

/**
 * Tar header as it's stored in file
 */
struct TarHeader
{
    std::array<char, 100> filename;
    std::array<char, 8> mode;
    std::array<char, 8> uid;
    std::array<char, 8> gid;
    std::array<char, 12> size;
    std::array<char, 12> mtime;
    std::array<char, 8> chksum;
    char typeflag;
    std::array<char, 100> linkname;
    std::array<char, 6> magic;
    std::array<char, 2> version;
    std::array<char, 32> uname;
    std::array<char, 32> gname;
    std::array<char, 8> devmajor;
    std::array<char, 8> devminor;
    std::array<char, 155> prefix;
    std::array<char, 12> padding;
};

#pragma pack(pop)

/**
 * @brief File inside tar archive
 */
class SP_EXPORT ArchiveFile
    : public Buffer
{
public:

    /**
     * @brief File type for file inside tar archive
     */
    enum class Type
        : uint8_t
    {
        REGULAR_FILE = '0',    ///< Regular file (preferred code).
        REGULAR_FILE2 = '\0',  ///< Regular file (alternate code).
        HARD_LINK = '1',       ///< Hard link.
        SYM_LINK = '2',        ///< Symbolic link (hard if not supported).
        CHARACTER = '3',       ///< Character special.
        BLOCK = '4',           ///< Block special.
        DIRECTORY = '5',       ///< Directory.
        FIFO = '6',            ///< Named pipe.
        CONTTYPE = '7'         ///< Contiguous file (regular file if not supported).
    };

    struct Ownership
    {
        int uid {0};
        int gid {0};
        String uname;
        String gname;
    };

    /**
     * @brief Constructor
     * @param fileName          File name
     * @param baseDirectory     Directory used as a base for relative path for files inside archive
     */
    explicit ArchiveFile(const fs::path& fileName, const fs::path& baseDirectory);

    /**
     * @brief Constructor
     * @param fileName          File name
     * @param content           File data (regular files only)
     * @param mode              File mode, i.e. 0640
     * @param mtime             Modification time
     * @param type              File type
     * @param ownership         File owners
     * @param linkName          Name the link is pointing to
     */
    ArchiveFile(const fs::path& fileName, const Buffer& content, int mode, const DateTime& mtime,
                ArchiveFile::Type type, const Ownership& ownership, const fs::path& linkName);

    /**
     * @brief Actual tar file header, length is TAR_BLOCK_SIZE
     * @return Actual tar file header data
     */
    const char* header() const;

    String fileName() const
    {
        return m_fileName;
    }

    unsigned mode() const
    {
        return m_mode;
    }

    const Ownership& ownership() const
    {
        return m_ownership;
    }

    unsigned size() const
    {
        return m_size;
    }

    DateTime mtime() const
    {
        return m_mtime;
    }

    Type type() const
    {
        return m_type;
    }

    String linkname() const
    {
        return m_linkname;
    }

    static fs::path relativePath(const fs::path& fileName, const fs::path& baseDirectory);

private:

    String m_fileName;
    unsigned m_mode {777};
    Ownership m_ownership {};
    unsigned m_size {0};
    DateTime m_mtime;
    Type m_type {Type::REGULAR_FILE};
    String m_linkname;

    std::shared_ptr<TarHeader> m_header;

    void makeHeader();
};

using SArchiveFile = std::shared_ptr<ArchiveFile>;

}

#endif
