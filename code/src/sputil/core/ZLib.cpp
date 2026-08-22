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

#include "zlib.h"
#include <sptk5/Exception.h>
#include <sptk5/ZLib.h>

using namespace std;
using namespace sptk;

constexpr size_t CHUNK = 16384;

void ZLib::compress(Buffer& dest, const Buffer& src, const int level, const bool append)
{
    z_stream strm = {};

    if (!append)
    {
        dest.reset();
    }

    // allocate deflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int ret = deflateInit2(&strm,
                           level,
                           Z_DEFLATED,
                           MAX_WBITS + 16,
                           8,
                           Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
    {
        throw Exception("deflateInit() error");
    }

    bool   eof = false;
    size_t readPosition = 0;
    // Compress until the end of data
    do
    {
        auto bytesToRead = src.bytes() - readPosition;
        if (bytesToRead > CHUNK)
        {
            bytesToRead = CHUNK;
        }
        else
        {
            eof = true;
        }

        auto* inputBuffer = (uint8_t*) src.data() + readPosition;

        readPosition += bytesToRead;
        strm.avail_in = static_cast<uInt>(bytesToRead);
        const int flush = eof ? Z_FINISH : Z_PARTIAL_FLUSH;
        strm.next_in = inputBuffer;

        // Run deflate() on input until output buffer not full, finish
        // compression if all the source has been read inputBuffer
        do
        {
            dest.reserve(dest.bytes() + CHUNK * 2);
            strm.avail_out = CHUNK;
            strm.next_out = dest.data() + dest.bytes();
            ret = deflate(&strm, flush); // no bad return value
            if (ret == Z_STREAM_ERROR)
            {
                deflateEnd(&strm);
                throw Exception("Compressed data error.");
            }
            const size_t have = CHUNK - strm.avail_out;
            dest.bytes(dest.bytes() + have);
        } while (strm.avail_out == 0);

        // Done when the last data inputBuffer file processed
    } while (!eof);

    // Clean up and return
    deflateEnd(&strm);
}

void ZLib::decompress(Buffer& dest, const Buffer& src, const bool append)
{
    z_stream strm = {};

    if (!append)
    {
        dest.reset();
    }

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int ret = inflateInit2(&strm, 16 + MAX_WBITS);
    if (ret != Z_OK)
    {
        throw Exception("inflateInit() error");
    }

    size_t readPosition = 0;
    // Decompress until deflate stream ends or end of file
    do
    {
        auto bytesToRead = src.bytes() - readPosition;
        if (bytesToRead > CHUNK)
        {
            bytesToRead = CHUNK;
        }
        auto* inputBuffer = (uint8_t*) src.data() + readPosition;
        readPosition += bytesToRead;
        strm.avail_in = static_cast<uInt>(bytesToRead);
        if (strm.avail_in == 0 && ret != Z_STREAM_END)
        {
            (void) inflateEnd(&strm);
            throw Exception("Input buffer is insufficient.");
        }
        strm.next_in = inputBuffer;

        // Run inflate() on input until output buffer not full
        do
        {
            dest.reserve(dest.bytes() + CHUNK * 2);

            strm.avail_out = CHUNK;
            strm.next_out = dest.data() + dest.bytes();
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret)
            {
                case Z_STREAM_ERROR:
                    (void) inflateEnd(&strm);
                    throw Exception("Compressed data error.");
                case Z_BUF_ERROR:
                    // Not a real error: inflate() returns this whenever avail_in reaches 0 in the
                    // same call that leaves avail_out unfilled - "no progress possible without more
                    // input". avail_out is untouched, so the outer loop's already-out-of-input check
                    // (above) still catches a truly corrupt/truncated stream.
                    break;
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void) inflateEnd(&strm);
                    throw Exception("premature compressed data error");
                default:
                    break;
            }
            const unsigned have = CHUNK - strm.avail_out;
            dest.bytes(dest.bytes() + have);
        } while (strm.avail_out == 0);

        // Done when inflate() says it's done
    } while (ret != Z_STREAM_END);

    // clean up and return
    inflateEnd(&strm);
}
