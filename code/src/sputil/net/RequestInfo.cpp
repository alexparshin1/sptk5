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

#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>
#include <sptk5/cnet>
#include <sptk5/net/RequestInfo.h>

using namespace std;
using namespace sptk;

void RequestInfo::Message::input(const Buffer& content, const String& contentEncoding)
{
    static const Strings knowContentEncodings({"", "br", "gzip", "x-www-form-urlencoded"});
    constexpr auto       initialBufferSize = 128;
    m_content.reset(initialBufferSize);
    m_compressedLength = content.size();
    m_contentEncoding = contentEncoding;

    switch (knowContentEncodings.indexOf(contentEncoding))
    {
        case 0:
            m_content = content;
            break;

#ifdef HAVE_BROTLI
        case 1:
            Brotli::decompress(m_content, content);
            break;
#endif

#ifdef HAVE_ZLIB
        case 2:
            ZLib::decompress(m_content, content);
            break;
#endif

        case 3:
            m_content = Url::decode(content.c_str());
            break;

        default:
            throw Exception("Content-Encoding '" + contentEncoding + "' is not supported");
    }
}

Buffer RequestInfo::Message::output(const Strings& contentEncodings)
{
    constexpr auto minimumSizeForCompression = 64;
    m_contentEncoding = "";
    if (m_content.bytes() > minimumSizeForCompression && !contentEncodings.empty())
    {
        Buffer outputData;
#ifdef HAVE_BROTLI
        if (contentEncodings.indexOf("br") >= 0)
        {
            m_contentEncoding = "br";
            Brotli::compress(outputData, m_content);
            m_compressedLength = outputData.size();
            return outputData;
        }
#endif
#ifdef HAVE_ZLIB
        if (contentEncodings.indexOf("gzip") >= 0)
        {
            m_contentEncoding = "gzip";
            ZLib::compress(outputData, m_content);
            m_compressedLength = outputData.size();
            return outputData;
        }
#endif
    }

    m_compressedLength = m_content.size();

    return m_content;
}
