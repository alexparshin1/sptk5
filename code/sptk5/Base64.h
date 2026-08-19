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

#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/sptk.h>

namespace sptk {
/**
 * @addtogroup utility Utility Classes.
 * @{
 */

/**
 * @brief Base64 encoding/decoding.
 *
 * This class is used for encoding and decoding the parts of mail messages.
 * All the methods of this class are static.
 */
class SP_EXPORT Base64
{
public:
    /**
     * @brief Data encoding.
     * Encodes data (base64) in given buffer bufSource to destination buffer bufDest.
     * @param bufDest           Destination buffer.
     * @param bufSource         Source buffer.
     * @param len               Size of source buffer.
     */
    static void encode(Buffer& bufDest, const uint8_t* bufSource, size_t len);

    /**
     * @brief Data encoding.
     * Encodes data (base64) in given buffer bufSource to destination buffer bufDest.
     * @param bufDest           Destination buffer.
     * @param bufSource         Source buffer.
     */
    static void encode(Buffer& bufDest, const Buffer& bufSource);

    /**
     * @brief Data encoding.
     * Encodes data (base64) in the given source buffer and returns the result in std::string.
     * @param strDest           Destination string.
     * @param bufSource         Source buffer.
     */
    static void encode(String& strDest, const Buffer& bufSource);

    /**
     * @brief Data decoding.
     * Decodes base64 encoded buffer bufSource into buffer bufDest.
     * Throws Exception in case of error.
     * @param bufDest           Destination buffer.
     * @param bufSource         Source buffer that holds base64 decoded data.
     * @returns length of returned buffer.
     */
    static size_t decode(Buffer& bufDest, const Buffer& bufSource);

    /**
     * @brief Data decoding.
     * Decodes base64 encoded string strSource into buffer bufDest.
     * Throws CException in case of error.
     * @param bufDest           Destination buffer.
     * @param strSource         Source string that holds base64 decoded data.
     * @returns length of the returned string.
     */
    static size_t decode(Buffer& bufDest, const String& strSource);
};
/**
 * @}
 */
} // namespace sptk
