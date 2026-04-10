/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Buffer.h>
#include <sptk5/DateTime.h>

namespace sptk {
constexpr size_t TAR_BLOCK_SIZE = 512; ///< Tar archive block size.

#pragma pack(push, 1)

/**
 * Tar header as it's stored in the file.
 */
struct TarHeader
{
    std::array<char, 100> filename;
    std::array<char, 8>   mode;
    std::array<char, 8>   uid;
    std::array<char, 8>   gid;
    std::array<char, 12>  size;
    std::array<char, 12>  mtime;
    std::array<char, 8>   checkSum;
    char                  typeflag;
    std::array<char, 100> linkName;
    std::array<char, 6>   magic;
    std::array<char, 2>   version;
    std::array<char, 32>  uname;
    std::array<char, 32>  gname;
    std::array<char, 8>   devmajor;
    std::array<char, 8>   devminor;
    std::array<char, 155> prefix;
    std::array<char, 12>  padding;
};

#pragma pack(pop)

/**
 * @brief File inside the tar archive.
 */
class SP_EXPORT ArchiveFile
    : public Buffer
{
public:
    /**
     * @brief File type for file inside the tar archive.
     */
    enum class Type : uint8_t
    {
        REGULAR_FILE = '0',   ///< Regular file (preferred code).
        REGULAR_FILE2 = '\0', ///< Regular file (alternate code).
        HARD_LINK = '1',      ///< Hard link.
        SYM_LINK = '2',       ///< Symbolic link (hard if not supported).
        CHARACTER = '3',      ///< Character special.
        BLOCK = '4',          ///< Block special.
        DIRECTORY = '5',      ///< Directory.
        FIFO = '6',           ///< Named pipe.
        CONTTYPE = '7'        ///< Contiguous file (regular file if not supported).
    };

    struct Ownership
    {
        int    uid {0};
        int    gid {0};
        String uname;
        String gname;
    };

    /**
     * @brief Constructor.
     * @param fileName          File name.
     * @param baseDirectory     Directory used as a base for the relative path for files inside the archive.
     */
    explicit ArchiveFile(const std::filesystem::path& fileName, const std::filesystem::path& baseDirectory);

    /**
     * @brief Constructor.
     * @param fileName          File name.
     * @param content           File data (regular files only).
     * @param mode              File mode, i.e., 0640.
     * @param mtime             Modification time.
     * @param type              File type.
     * @param ownership         File owners.
     * @param linkName          Name, the link is pointing to.
     */
    ArchiveFile(const std::filesystem::path& fileName, const Buffer& content, int mode, DateTime mtime,
                Type type, Ownership ownership, const std::filesystem::path& linkName);

    /**
     * @brief Actual tar file header, length is TAR_BLOCK_SIZE
     * @return Actual tar file header data
     */
    [[nodiscard]] const char* header() const;

    /**
     * @brief File name.
     * @return File name, including the relative path.
     */
    [[nodiscard]] String fileName() const
    {
        return m_fileName;
    }

    /**
     * @brief File mode, i.e., 0640.
     * @return File mode.
     */
    [[nodiscard]] unsigned mode() const
    {
        return m_mode;
    }

    /**
     * @brief File ownership information.
     * @return File ownership information.
     */
    [[nodiscard]] const Ownership& ownership() const
    {
        return m_ownership;
    }

    /**
     * @brief File modification time.
     * @return File modification time.
     */
    [[nodiscard]] DateTime mtime() const
    {
        return m_mtime;
    }

    /**
     * @brief File type.
     * @return File type.
     */
    [[nodiscard]] Type type() const
    {
        return m_type;
    }

    /**
     * @brief Link name, the link is pointing to (for symbolic and hard links).
     * @return Link name.
     */
    [[nodiscard]] String linkname() const
    {
        return m_linkname;
    }

    /**
     * @brief Returns the relative path for the file inside the archive.
     * @param fileName          File name.
     * @param baseDirectory     Directory used as a base for the relative path for files inside the archive.
     * @return Relative path for the file inside the archive.
     */
    static std::filesystem::path relativePath(const std::filesystem::path& fileName, const std::filesystem::path& baseDirectory);

private:
    String                     m_fileName;                  ///< File name, including the relative path.
    unsigned                   m_mode {777};                ///< File mode, i.e., 0640.
    Ownership                  m_ownership {};              ///< File ownership information.
    DateTime                   m_mtime;                     ///< File modification time.
    Type                       m_type {Type::REGULAR_FILE}; ///< File type.
    String                     m_linkname;                  ///< Link name, the link is pointing to (for symbolic and hard links).
    std::shared_ptr<TarHeader> m_header;                    ///< Actual tar file header, length is TAR_BLOCK_SIZE.

    /**
     * @brief Create the tar header for the file.
     */
    void makeHeader();
};

using SArchiveFile = std::shared_ptr<ArchiveFile>;

} // namespace sptk
