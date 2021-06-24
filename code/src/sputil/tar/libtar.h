/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  libtar.h - header file for libtar library
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#pragma once

#include <array>

/* useful constants */
constexpr int T_BLOCKSIZE = 512;
constexpr int T_NAMELEN = 100;
constexpr int T_PREFIXLEN = 155;
constexpr int T_MAXPATHLEN = T_NAMELEN + T_PREFIXLEN;

/* GNU extensions for typeflag */
constexpr char GNU_LONGNAME_TYPE = 'L';
constexpr char GNU_LONGLINK_TYPE = 'K';

/* Tar header as it's stored in file */

#pragma pack(push, 1)

struct TarHeader
{
    std::array<char, 100> name;
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
