#ifndef SPTK_ARCHIVEFILE_H
#define SPTK_ARCHIVEFILE_H

#include <sptk5/Buffer.h>
#include "src/sputil/tar/libtar.h"

namespace sptk {

class ArchiveFile: public Buffer
{
public:
    explicit ArchiveFile(const String& fileName, const String& baseDirectory);
    ArchiveFile(const String& fileName, const Buffer& content, int mode, int uid, int gid, time_t mtime,
            char typeflag, const String& uname, const String& gname);

    const char* header() const;
    String fileName() const;

private:

    std::shared_ptr<tar_header> m_header;

    void makeHeader(const String& fileName, int mode, int uid, int gid, time_t mtime,
                    char typeflag, const String& uname, const String& gname);
};

using SArchiveFile = std::shared_ptr<ArchiveFile>;

}

#endif
