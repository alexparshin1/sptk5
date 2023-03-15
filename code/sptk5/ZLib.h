/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/sptk.h>

#ifdef HAVE_ZLIB

#include <sptk5/Buffer.h>
#include <zlib.h>

namespace sptk {

/**
 * Simple wrapper for ZLib functions
 */
class SP_EXPORT ZLib
{
public:
    /**
     * Compress data using gzip format.
     *
     * Compressed data is appended to destination buffer
     * @param dest Buffer&, Destination buffer
     * @param src const Buffer&, Source buffer
     */
    static void compress(Buffer& dest, const Buffer& src, int level = Z_DEFAULT_COMPRESSION);

    /**
     * Uncompress data in gzip format
     *
     * Uncompressed data is appended to destination buffer
     * @param dest Buffer&, Destination buffer
     * @param src const Buffer&, Source buffer
     */
    static void decompress(Buffer& dest, const Buffer& src);
};

} // namespace sptk

#endif
