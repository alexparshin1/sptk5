/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Base64.h - description                                 ║
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

#ifndef __SPTK_BASE64_H__
#define __SPTK_BASE64_H__

#include <sptk5/Exception.h>
#include <sptk5/sptk.h>
#include <sptk5/Buffer.h>
#include <string>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Base64 encoding/decoding.
 *
 * This class is used for encoding and decoding the parts of mail messages.
 * All the methods of this class are static.
 */
class Base64
{
public:
    /**
     * @brief Data encoding.
     * Encodes data (base64) in given buffer bufSource to destination buffer bufDest.
     * @param bufDest           Destination buffer
     * @param bufSource         Source buffer
     * @param len               Size of source buffer
     */
    static void encode(Buffer& bufDest, const char* bufSource, size_t len);

    /**
     * @brief Data encoding.
     * Encodes data (base64) in given buffer bufSource to destination buffer bufDest.
     * @param bufDest           Destination buffer
     * @param bufSource         Source buffer
     */
    static void encode(Buffer& bufDest, const Buffer& bufSource);

    /**
     * @brief Data encoding.
     * Encodes data (base64) in given buffer bufSource and returns result in std::string.
     * @param strDest std::string Destination string
     * @param bufSource CBuffer& Source buffer
     */
    static void encode(std::string& strDest, const Buffer& bufSource);

    /**
     * @brief Data decoding.
     * Decodes base64 encoded buffer bufSource into buffer bufDest.
     * Throws CException in case of error
     * @param bufDest CBuffer destination buffer
     * @param bufSource CBuffer source buffer that holds base64 decoded data
     * @returns length of returned buffer
     */
    static size_t decode(Buffer &bufDest, const Buffer &bufSource);

    /**
     * @brief Data decoding.
     * Decodes base64 encoded string strSource into buffer bufDest.
     * Throws CException in case of error
     * @param bufDest CBuffer, destination buffer
     * @param strSource const std::string &, source string that holds base64 decoded data
     * @returns length of the returned string
     */
    static size_t decode(Buffer &bufDest, const std::string &strSource);
};
/**
 * @}
 */
}

#endif
