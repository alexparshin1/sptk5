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

#pragma once

#include <sptk5/Buffer.h>

namespace sptk
{

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * Request information
 */
class SP_EXPORT RequestInfo
{
public:
    /**
     * Message information
     */
    class SP_EXPORT Message
    {
    public:
        void input(const Buffer& content, const String& contentEncoding);
        Buffer output(const Strings& contentEncodings);

        Buffer& content() { return m_content; }
        const Buffer& content() const { return m_content; }

        String contentEncoding() const { return m_contentEncoding; }
        size_t compressedLength() const { return m_compressedLength; }

    private:
        Buffer  m_content;              ///< Message content (decompressed)
        String  m_contentEncoding;      ///< Content encoding, i.e. gzip, br, etc
        size_t  m_compressedLength {0}; ///< Compressed length if contentEncoding isn't empty
    };

    /**
     * Constructor
     * @param name              Request name
     */
    RequestInfo(const String& name="") : name(name) {}

    Message     request;                ///< Request data
    Message     response;               ///< Response data
    String      name;                   ///< Request name
};

/**
 * @}
 */
}

