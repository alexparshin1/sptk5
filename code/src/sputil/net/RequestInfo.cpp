/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/net/RequestInfo.h>
#include <sptk5/Buffer.h>
#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>

using namespace std;
using namespace sptk;

void RequestInfo::Message::input(const Buffer& content, const String& contentEncoding)
{
    m_compressedLength = content.length();
    m_contentEncoding = contentEncoding;
    if (contentEncoding.empty()) {
        m_content = content;
        return;
    }

#if HAVE_BROTLI
    if (contentEncoding == "br") {
        Brotli::decompress(m_content, content);
        return;
    }
#endif

#if HAVE_ZLIB
    if (contentEncoding == "gzip") {
        ZLib::decompress(m_content, content);
        return;
    }
#endif

    throw Exception("Content-Encoding '" + contentEncoding + "' is not supported");
}

Buffer RequestInfo::Message::output(const Strings& contentEncodings)
{
    m_contentEncoding = "";
    if (m_content.bytes() > 64 && !contentEncodings.empty()) {
        Buffer outputData;
#if HAVE_BROTLI
        if (contentEncodings.indexOf("br") >= 0) {
            m_contentEncoding = "br";
            Brotli::compress(outputData, m_content);
            m_compressedLength = outputData.length();
            return outputData;
        }
#endif
#if HAVE_ZLIB
        if (contentEncodings.indexOf("gzip") >= 0) {
            m_contentEncoding = "gzip";
            ZLib::compress(outputData, m_content);
            m_compressedLength = outputData.length();
            return outputData;
        }
#endif
    }

    m_compressedLength = m_content.length();

    return m_content;
}
