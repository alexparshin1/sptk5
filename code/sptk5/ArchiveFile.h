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
#include "src/sputil/tar/libtar.h"

namespace sptk {

class ArchiveFile
    : public Buffer
{
public:

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

    explicit ArchiveFile(const String& fileName, const String& baseDirectory);

    ArchiveFile(const String& fileName, const uint8_t* content, size_t contentLength, int mode, int uid,
                int gid, const DateTime& mtime, ArchiveFile::Type type, const String& uname, const String& gname,
                const String& linkName);

    const char* header() const;

    String fileName() const;

    unsigned mode() const
    {
        return m_mode;
    }

    unsigned uid() const
    {
        return m_uid;
    }

    unsigned gid() const
    {
        return m_gid;
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

    String uname() const
    {
        return m_uname;
    }

    String gname() const
    {
        return m_gname;
    }

private:

    String m_name;
    unsigned m_mode{777};
    unsigned m_uid{0};
    unsigned m_gid{0};
    unsigned m_size{0};
    DateTime m_mtime;
    Type m_type{Type::REGULAR_FILE};
    String m_linkname;
    String m_uname;
    String m_gname;

    std::shared_ptr<TarHeader> m_header;

    void makeHeader();

    String relativePath(const String& fileName, const String& baseDirectory) const;
};

using SArchiveFile = std::shared_ptr<ArchiveFile>;

}

#endif
