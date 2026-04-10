/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Base64.h>

using namespace std;
using namespace sptk;

static constexpr array<uint8_t, 64> B64Chars = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'};

static constexpr array<int8_t, 256> B64Lookup = []
{
    array<int8_t, 256> lookup {};
    lookup.fill(-1);
    for (size_t i = 0; i < B64Chars.size(); ++i)
    {
        lookup[static_cast<uint8_t>(B64Chars[i])] = static_cast<int8_t>(i);
    }
    // Also support URL-safe characters
    lookup[static_cast<uint8_t>('-')] = lookup[static_cast<uint8_t>('+')];
    lookup[static_cast<uint8_t>('_')] = lookup[static_cast<uint8_t>('/')];

    return lookup;
}();

namespace {
inline uint8_t base64chars(int chr)
{
    return B64Chars[static_cast<size_t>(chr & 0x3F)];
}

inline bool is_base64(uint8_t chr) noexcept
{
    return B64Lookup[chr] != -1;
}
} // namespace

void Base64::encode(Buffer& bufDest, const uint8_t* bufSource, size_t len)
{
    if (len == 0)
    {
        bufDest.reset();
        return;
    }

    const auto* current = bufSource;
    auto        outputLen = (len + 2) / 3 * 4;

    bufDest.checkSize(outputLen + 1);
    auto* output = bufDest.data();

    while (len >= 3)
    {
        *output = base64chars((static_cast<int>(current[0]) & 0xFC) >> 2);
        ++output;

        *output = base64chars(((static_cast<int>(current[0]) & 0x03) << 4) | ((static_cast<int>(current[1]) & 0xF0) >> 4));
        ++output;

        *output = base64chars(((static_cast<int>(current[1]) & 0x0F) << 2) | ((static_cast<int>(current[2]) & 0xC0) >> 6));
        ++output;

        *output = base64chars(static_cast<int>(current[2]) & 0x3F);
        ++output;

        len -= 3;
        current += 3;
    }

    /// Now we should clean up remainder
    if (len > 0)
    {
        *output = base64chars(static_cast<int>(current[0]) >> 2);
        ++output;
        if (len > 1)
        {
            *output = base64chars(((static_cast<int>(current[0]) & 0x03) << 4) | ((static_cast<int>(current[1]) & 0xF0) >> 4));
            ++output;
            *output = base64chars((static_cast<int>(current[1]) & 0x0f) << 2);
            ++output;
            *output = '=';
            ++output;
        }
        else
        {
            *output = base64chars((static_cast<int>(current[0]) & 0x03) << 4);
            ++output;
            *output = '=';
            ++output;
            *output = '=';
            ++output;
        }
    }
    *output = 0;
    bufDest.bytes(outputLen);
}

void Base64::encode(Buffer& bufDest, const Buffer& bufSource)
{
    encode(bufDest, bufSource.data(), bufSource.bytes());
}

void Base64::encode(String& strDest, const Buffer& bufSource)
{
    Buffer bufOut;
    encode(bufOut, bufSource);

    strDest = String(bufOut.c_str(), bufOut.bytes());
}

namespace {

size_t internal_decode(Buffer& dest, const uint8_t* source, size_t sourceLen)
{
    dest.reset();
    if (sourceLen == 0)
    {
        return 0;
    }

    dest.checkSize(sourceLen / 4 * 3 + 3);

    uint32_t val = 0;
    int      valb = -8;
    for (size_t i = 0; i < sourceLen; ++i)
    {
        uint8_t c = source[i];
        if (B64Lookup[c] != -1)
        {
            val = (val << 6) | static_cast<uint32_t>(B64Lookup[c]);
            valb += 6;
            if (valb >= 0)
            {
                dest.append(static_cast<uint8_t>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        else if (c == '=')
        {
            break;
        }
    }

    return dest.bytes();
}

} // namespace

size_t Base64::decode(Buffer& bufDest, const Buffer& bufSource)
{
    return internal_decode(bufDest, bufSource.data(), bufSource.bytes());
}

size_t Base64::decode(Buffer& bufDest, const String& strSource)
{
    return internal_decode(bufDest, reinterpret_cast<const uint8_t*>(strSource.c_str()), strSource.length());
}
