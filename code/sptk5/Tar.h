/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <ranges>
#include <sptk5/ArchiveFile.h>
#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/Strings.h>
#include <sptk5/sptk.h>

namespace sptk {
/**
 * Tar archive
 *
 * Allows reading tar archive files into memory buffers.
 * The main usage currently is to read an SPTK theme from tar-archive.
 */
class SP_EXPORT Tar
{
    using FileCollection = std::map<std::filesystem::path, SArchiveFile>;

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
    explicit Tar(const std::filesystem::path& tarFileName);

    /**
     * Reads tar archive from file
     *
     * The archive content is red into the internal set of buffers
     * @param fileName          File name to open
     */
    void read(const std::filesystem::path& fileName);

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
    [[maybe_unused]] std::vector<std::filesystem::path> fileList() const
    {
        std::vector<std::filesystem::path> fileNames;
        for (const auto& fileName: m_files | std::views::keys)
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
    const ArchiveFile& file(const std::filesystem::path& fileName) const;

    /**
     * Remove file data
     * @param fileName          Archive file
     */
    void remove(const std::filesystem::path& filename);

    /**
     * Add file data
     * @param fileName          Archive file
     */
    void append(const SArchiveFile& file);

    /**
     * Save tar archive to file
     * @param tarFileName          Tar file name
     */
    void save(const std::filesystem::path& tarFileName) const;

    /**
     * Clears the allocated memory
     */
    void clear();

private:
    FileCollection m_files;    ///< File name to the file data map
    String         m_fileName; ///< Tar file name

    [[nodiscard]] bool readNextFile(const Buffer& buffer, size_t& offset);
};

} // namespace sptk
