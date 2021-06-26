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

#include "sptk5/ArchiveFile.h"
#include "sptk5/Tar.h"
#include <chrono>
#include <filesystem>

#ifndef _WIN32

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sptk5/DateTime.h>
#include <sptk5/SystemException.h>

#endif

using namespace std;
using namespace sptk;

namespace fs = filesystem;

template<typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
                                                        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

ArchiveFile::ArchiveFile(const String& fileName, const String& baseDirectory)
{
    auto relativeFileName = relativePath(fileName, baseDirectory);

    fs::path path(fileName.c_str());
    auto status = fs::status(path);

    m_type = ArchiveFile::Type::REGULAR_FILE;
    if (fs::is_symlink(path))
    {
        m_type = ArchiveFile::Type::SYM_LINK;
        status = fs::symlink_status(path);
        m_linkname = fs::read_symlink(path);
    }
    else if (fs::is_regular_file(status))
    {
        loadFromFile(fileName.c_str());
    }
    else if (fs::is_directory(status))
    {
        m_type = ArchiveFile::Type::DIRECTORY;
        fs::path relpath(relativeFileName.c_str());
        relpath /= "";
        relativeFileName = relpath;
    }

    m_mode = (int) status.permissions();

    fs::file_time_type ftime = fs::last_write_time(path);
    time_t mtime = to_time_t(ftime);
    m_mtime = DateTime::convertCTime(mtime);

#ifndef _WIN32
    struct stat info;
    stat(fileName.c_str(), &info);  // Error check omitted

    Buffer buff(128);
    struct passwd pw {};
    if (struct passwd* pw_result; getpwuid_r(info.st_uid, &pw, (char*) buff.data(), 128, &pw_result) != 0)
    {
        throw SystemException("Can't get user information");
    }

    m_uname = pw.pw_name;
    m_uid = pw.pw_uid;
    m_gid = pw.pw_gid;

    struct group gr {};
    if (struct group* gr_result; getgrgid_r(info.st_gid, &gr, (char*) buff.data(), 128, &gr_result) != 0)
    {
        throw SystemException("Can't get group information");
    }

    m_gname = gr.gr_name;
#endif

    makeHeader();
}

String ArchiveFile::relativePath(const String& fileName, const String& baseDirectory) const
{
    const fs::path basePath(baseDirectory.c_str());
    if (!fileName.startsWith(basePath.c_str()))
    {
        return fileName;
    }

    return fileName.substr(string(basePath).length() + 1);
}

ArchiveFile::ArchiveFile(const String& fileName, const uint8_t* content, size_t contentLength, int mode, int uid,
                         int gid, const DateTime& mtime, ArchiveFile::Type type, const String& uname,
                         const String& gname, const String& linkName)
    : Buffer(content, contentLength),
      m_fileName(fileName), m_mode(mode), m_uid(uid), m_gid(gid), m_mtime(mtime),
      m_type(type), m_linkname(linkName), m_uname(uname), m_gname(gname)
{
    makeHeader();
}

void ArchiveFile::makeHeader()
{
    m_header = make_shared<TarHeader>();
    memset(m_header.get(), 0, sizeof(TarHeader));

    strncpy(m_header->filename.data(), m_fileName.c_str(), sizeof(m_header->filename));
    snprintf(m_header->mode.data(), sizeof(m_header->mode), "%07o", m_mode);
    snprintf(m_header->uid.data(), sizeof(m_header->uid), "%07o", m_uid);
    snprintf(m_header->gid.data(), sizeof(m_header->gid), "%07o", m_gid);
    snprintf(m_header->size.data(), sizeof(m_header->size), "%011o", (unsigned) length());
    snprintf(m_header->mtime.data(), sizeof(m_header->mtime), "%011o", (unsigned) (time_t) m_mtime);

    m_header->typeflag = (char) m_type;

    if (m_type == ArchiveFile::Type::SYM_LINK)
    {
        strncpy(m_header->linkname.data(), m_linkname.c_str(), sizeof(m_header->linkname));
    }

    memcpy(m_header->magic.data(), "ustar ", sizeof(m_header->magic));
    snprintf(m_header->version.data(), sizeof(m_header->version), " ");

    snprintf(m_header->uname.data(), sizeof(m_header->uname), "%s", m_uname.c_str());
    snprintf(m_header->gname.data(), sizeof(m_header->gname), "%s", m_gname.c_str());

    memset(m_header->chksum.data(), ' ', sizeof(m_header->chksum));
    unsigned chksum = 0;

    const auto* header = (const uint8_t*) m_header.get();
    for (size_t i = 0; i < sizeof(TarHeader); ++i)
    {
        chksum += header[i];
    }

    snprintf(m_header->chksum.data(), sizeof(m_header->chksum) - 1, "%06o", chksum);
}

const char* ArchiveFile::header() const
{
    return (const char*) m_header.get();
}
