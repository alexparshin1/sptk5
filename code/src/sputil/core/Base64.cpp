/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

static constexpr array B64Chars = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'};

inline uint8_t base64chars(int chr)
{
    return B64Chars[(chr & 0x3F)];
}

void Base64::encode(Buffer& bufDest, const uint8_t* bufSource, size_t len)
{
    const auto* current = bufSource;
    auto        outputLen = static_cast<size_t>(len / 3 * 4);
    if ((len % 3) != 0)
    {
        outputLen += 4;
    }
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
        current += 3; /* move pointer 3 characters forward */
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

const String base64_chars(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/");

inline bool is_base64(uint8_t chr) noexcept
{
    return (isalnum(chr) || (chr == '+') || (chr == '/'));
}

size_t internal_decode(Buffer& dest, std::string const& encoded_string)
{
    size_t            in_len = encoded_string.size();
    int               index = 0;
    int               in_ = 0;
    array<uint8_t, 4> char_array_4 {};
    array<uint8_t, 3> char_array_3 {};

    dest.reset();

    while (in_len && (encoded_string[in_] != '=') && is_base64(static_cast<uint8_t>(encoded_string[in_])))
    {
        --in_len;
        char_array_4[index] = static_cast<uint8_t>(encoded_string[in_]);
        ++index;
        ++in_;
        if (index == 4)
        {
            for (index = 0; index < 4; ++index)
            {
                char_array_4[index] = static_cast<uint8_t>(base64_chars.find(static_cast<char>(char_array_4[index])));
            }

            char_array_3[0] = static_cast<uint8_t>((static_cast<int>(char_array_4[0]) << 2) + ((static_cast<int>(char_array_4[1]) & 0x30) >> 4));
            char_array_3[1] = static_cast<uint8_t>(((static_cast<int>(char_array_4[1]) & 0xf) << 4) + ((static_cast<int>(char_array_4[2]) & 0x3c) >> 2));
            char_array_3[2] = static_cast<uint8_t>(((static_cast<int>(char_array_4[2]) & 0x3) << 6) + static_cast<int>(char_array_4[3]));

            dest.append(char_array_3.data(), 3);
            index = 0;
        }
    }

    if (index != 0)
    {
        int j = index;
        for (; j < 4; ++j)
        {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; ++j)
        {
            char_array_4[j] = static_cast<uint8_t>(base64_chars.find(static_cast<char>(char_array_4[j])));
        }

        char_array_3[0] = static_cast<uint8_t>((static_cast<int>(char_array_4[0]) << 2) + ((static_cast<int>(char_array_4[1]) & 0x30) >> 4));
        char_array_3[1] = static_cast<uint8_t>(((static_cast<int>(char_array_4[1]) & 0xf) << 4) + ((static_cast<int>(char_array_4[2]) & 0x3c) >> 2));
        char_array_3[2] = static_cast<uint8_t>(((static_cast<int>(char_array_4[2]) & 0x3) << 6) + static_cast<int>(char_array_4[3]));

        for (j = 0; (j < index - 1); ++j)
        {
            dest.append(static_cast<char>(char_array_3[j]));
        }
    }

    return dest.bytes();
}
} // namespace

size_t Base64::decode(Buffer& bufDest, const Buffer& bufSource)
{
    const string source(bufSource.c_str(), bufSource.bytes());
    return internal_decode(bufDest, source);
}

size_t Base64::decode(Buffer& bufDest, const String& strSource)
{
    return internal_decode(bufDest, strSource);
}
