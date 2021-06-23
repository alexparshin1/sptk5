#include "sptk5/ArchiveFile.h"
#include <chrono>
#include <filesystem>

#ifndef _WIN32

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

#endif

using namespace std;
using namespace sptk;

namespace fs = filesystem;

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
                                                        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

ArchiveFile::ArchiveFile(const String& fileName, const String& baseDirectory)
{
    loadFromFile(fileName);

    String relativeFileName(fileName);

    const fs::path basePath(baseDirectory.c_str());
    if (!fileName.startsWith(basePath.c_str())) {
        throw Exception("File path " + fileName + " is not located under " + baseDirectory);
    }

    relativeFileName = fileName.substr(string(basePath).length() + 1);

    const fs::path path(fileName.c_str());
    const auto status = fs::status(path);

    char typeflag = REGTYPE;
    if (fs::is_symlink(status))
    {
        typeflag = SYMTYPE;
    }

    int mode = (int) status.permissions();

    fs::file_time_type ftime = fs::last_write_time(path);
    time_t mtime = to_time_t(ftime);

    String uname;
    String gname;
    int uid = 0;
    int gid = 0;
#ifndef _WIN32
    struct stat info;
    stat(fileName.c_str(), &info);  // Error check omitted
    struct passwd* pw = getpwuid(info.st_uid);
    struct group* gr = getgrgid(info.st_gid);
    uname = pw->pw_name;
    gname = gr->gr_name;
    uid = pw->pw_uid;
    gid = pw->pw_gid;
#endif

    makeHeader(relativeFileName, mode, uid, gid, mtime, typeflag, uname, gname);
}

ArchiveFile::ArchiveFile(const String& fileName, const Buffer& content, int mode, int uid, int gid, time_t mtime,
                         char typeflag, const String& uname, const String& gname)
    : Buffer(content)
{
    makeHeader(fileName, mode, uid, gid, mtime, typeflag, uname, gname);
}

void ArchiveFile::makeHeader(const String& fileName, int mode, int uid, int gid, time_t mtime,
                             char typeflag, const String& uname, const String& gname)
{
    m_header = make_shared<tar_header>();
    memset(m_header.get(), 0, sizeof(tar_header));

    strncpy(m_header->name.data(), fileName.c_str(), sizeof(m_header->name));
    snprintf(m_header->mode.data(), sizeof(m_header->mode), "%07o", mode);
    snprintf(m_header->uid.data(), sizeof(m_header->uid), "%07o", uid);
    snprintf(m_header->gid.data(), sizeof(m_header->gid), "%07o", gid);
    snprintf(m_header->size.data(), sizeof(m_header->size), "%011o", (unsigned) length());
    snprintf(m_header->mtime.data(), sizeof(m_header->mtime), "%011o", (unsigned) mtime);

    m_header->typeflag = typeflag;

    snprintf(m_header->magic.data(), sizeof(m_header->magic), "ustar ");
    snprintf(m_header->version.data(), sizeof(m_header->version), " ");

    snprintf(m_header->uname.data(), sizeof(m_header->uname), "%s", uname.c_str());
    snprintf(m_header->gname.data(), sizeof(m_header->gname), "%s", gname.c_str());

    memset(m_header->chksum.data(), ' ', sizeof(m_header->chksum));
    unsigned chksum = ' '; // Compensating difference from gtar - should be starting from 0

    const auto* header = (const uint8_t*) m_header.get();
    for (int i = 0; i < sizeof(tar_header); ++i)
    {
        chksum += header[i];
    }

    snprintf(m_header->chksum.data(), sizeof(m_header->chksum) - 1, "%06o", chksum);
}

const char* ArchiveFile::header() const
{
    return (const char*) m_header.get();
}

String ArchiveFile::fileName() const
{
    return m_header->name.data();
}
