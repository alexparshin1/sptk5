/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                           ZLib.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_ZLIB_H__
#define __SPTK_ZLIB_H__

#include <sptk5/sptk.h>

#if HAVE_ZLIB

#include <sptk5/Buffer.h>

namespace sptk
{

/**
 * Simple wrapper for ZLib functions
 */
class ZLib
{
public:
    /**
     * Compress data using gzip format.
     * 
     * Compressed data is appended to destination buffer
     * @param dest Buffer&, Destination buffer
     * @param src const Buffer&, Source buffer
     */
    static void compress(Buffer& dest, const Buffer& src);

    /**
     * Uncompress data in gzip format
     * 
     * Uncompressed data is appended to destination buffer
     * @param dest Buffer&, Destination buffer
     * @param src const Buffer&, Source buffer
     */
    static void decompress(Buffer& dest, const Buffer& src);
};

}

#endif

#endif
