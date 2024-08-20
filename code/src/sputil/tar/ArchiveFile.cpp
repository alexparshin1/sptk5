/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/ArchiveFile.h"
#include <chrono>
#include <utility>

#ifndef _WIN32

#include <grp.h>
#include <pwd.h>
#include <sptk5/SystemException.h>
#include <sys/stat.h>

#endif

using namespace std;
using namespace sptk;

template<typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

ArchiveFile::ArchiveFile(const filesystem::path& fileName, const filesystem::path& baseDirectory)
{
    auto relativeFileName = relativePath(fileName, baseDirectory);

    const filesystem::path path(fileName.c_str());
    auto status = filesystem::status(path);

    if (filesystem::is_symlink(path))
    {
        m_type = ArchiveFile::Type::SYM_LINK;
        status = filesystem::symlink_status(path);
        m_linkname = filesystem::read_symlink(path).string();
    }
    else if (filesystem::is_regular_file(status))
    {
        loadFromFile(fileName.c_str());
    }
    else if (filesystem::is_directory(status))
    {
        m_type = ArchiveFile::Type::DIRECTORY;
        filesystem::path relpath(relativeFileName.c_str());
        relpath /= "";
        relativeFileName = relpath;
    }

    m_mode = static_cast<int>(status.permissions());

    const filesystem::file_time_type ftime = filesystem::last_write_time(path);
    const time_t mtime = to_time_t(ftime);
    m_mtime = DateTime::convertCTime(mtime);

#ifndef _WIN32
    struct stat info {
    };
    stat(fileName.c_str(), &info); // Error check omitted

    constexpr int bufferSize = 128;
    Buffer buff(bufferSize);
    struct passwd pw {
    };
    if (struct passwd * pw_result {}; getpwuid_r(info.st_uid, &pw, (char*) buff.data(), bufferSize, &pw_result) != 0)
    {
        throw SystemException("Can't get user information");
    }

    m_ownership.uname = pw.pw_name;
    m_ownership.uid = static_cast<int>(pw.pw_uid);
    m_ownership.gid = static_cast<int>(pw.pw_gid);

    struct group gr {
    };
    if (struct group * gr_result {}; getgrgid_r(info.st_gid, &gr, (char*) buff.data(), bufferSize, &gr_result) != 0)
    {
        throw SystemException("Can't get group information");
    }

    m_ownership.gname = gr.gr_name;
#endif

    makeHeader();
}

filesystem::path ArchiveFile::relativePath(const filesystem::path& fileName, const filesystem::path& baseDirectory)
{
    filesystem::path relativePath;

    auto fdir = fileName.begin();
    for (auto baseDirIterator = baseDirectory.begin(); fdir != fileName.end(); ++fdir, ++baseDirIterator)
    {
        if (baseDirIterator == baseDirectory.end())
        {
            break;
        }
        if (*fdir != *baseDirIterator)
        {
            return fileName;
        }
    }

    relativePath = *(fdir++);
    for (; fdir != fileName.end(); ++fdir)
    {
        relativePath /= *fdir;
    }

    return relativePath;
}

ArchiveFile::ArchiveFile(const filesystem::path& fileName, const Buffer& content, int mode, DateTime mtime,
                         ArchiveFile::Type type, ArchiveFile::Ownership ownership,
                         const filesystem::path& linkName)
    : Buffer(content)
    , m_fileName(fileName.string())
    , m_mode(mode)
    , m_ownership(std::move(ownership))
    , m_mtime(std::move(mtime))
    , m_type(type)
    , m_linkname(linkName.string())
{
    makeHeader();
}

void ArchiveFile::makeHeader()
{
    m_header = make_shared<TarHeader>();
    memset(m_header.get(), 0, sizeof(TarHeader));

    strncpy(m_header->filename.data(), m_fileName.c_str(), sizeof(m_header->filename) - 1);
    snprintf(m_header->mode.data(), sizeof(m_header->mode), "%07o", m_mode);
    snprintf(m_header->uid.data(), sizeof(m_header->uid), "%07o", m_ownership.uid);
    snprintf(m_header->gid.data(), sizeof(m_header->gid), "%07o", m_ownership.gid);
    snprintf(m_header->size.data(), sizeof(m_header->size), "%011o", static_cast<unsigned>(size()));
    snprintf(m_header->mtime.data(), sizeof(m_header->mtime), "%011o", static_cast<unsigned>((time_t) m_mtime));

    m_header->typeflag = static_cast<char>(m_type);

    if (m_type == ArchiveFile::Type::SYM_LINK)
    {
        strncpy(m_header->linkName.data(), m_linkname.c_str(), sizeof(m_header->linkName) - 1);
    }

    memcpy(m_header->magic.data(), "ustar ", sizeof(m_header->magic));
    snprintf(m_header->version.data(), sizeof(m_header->version), " ");

    snprintf(m_header->uname.data(), sizeof(m_header->uname), "%s", m_ownership.uname.c_str());
    snprintf(m_header->gname.data(), sizeof(m_header->gname), "%s", m_ownership.gname.c_str());

    memset(m_header->checkSum.data(), ' ', sizeof(m_header->checkSum));
    unsigned checkSum = 0;

    const auto* header = (const uint8_t*) m_header.get();
    for (size_t i = 0; i < sizeof(TarHeader); ++i)
    {
        checkSum += header[i];
    }

    snprintf(m_header->checkSum.data(), sizeof(m_header->checkSum) - 1, "%06o", checkSum);
}

const char* ArchiveFile::header() const
{
    return (const char*) m_header.get();
}
