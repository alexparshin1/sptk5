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

#include <sptk5/Base64.h>

using namespace std;
using namespace sptk;

static const array<char, 64> B64Chars = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'};

inline uint8_t base64chars(int c)
{
    return B64Chars[(c & 0x3F)];
}

void Base64::encode(Buffer& bufDest, const uint8_t* bufSource, size_t len)
{
    const auto* current = bufSource;
    auto outputLen = size_t(len / 3 * 4);
    if ((len % 3) != 0)
    {
        outputLen += 4;
    }
    bufDest.checkSize(outputLen + 1);
    auto* output = bufDest.data();

    while (len >= 3)
    {
        *output = base64chars((int(current[0]) & 0xFC) >> 2);
        ++output;

        *output = base64chars(((int(current[0]) & 0x03) << 4) | ((int(current[1]) & 0xF0) >> 4));
        ++output;

        *output = base64chars(((int(current[1]) & 0x0F) << 2) | ((int(current[2]) & 0xC0) >> 6));
        ++output;

        *output = base64chars(int(current[2]) & 0x3F);
        ++output;

        len -= 3;
        current += 3; /* move pointer 3 characters forward */
    }

    /// Now we should clean up remainder
    if (len > 0)
    {
        *output = base64chars((int) current[0] >> 2);
        ++output;
        if (len > 1)
        {
            *output = base64chars((((int) current[0] & 0x03) << 4) | (((int) current[1] & 0xF0) >> 4));
            ++output;
            *output = base64chars(((int) current[1] & 0x0f) << 2);
            ++output;
            *output = '=';
            ++output;
        }
        else
        {
            *output = base64chars(((int) current[0] & 0x03) << 4);
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

static const String base64_chars(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/");

static inline bool is_base64(uint8_t c) noexcept
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static size_t internal_decode(Buffer& dest, std::string const& encoded_string)
{
    size_t in_len = encoded_string.size();
    int i = 0;
    int in_ = 0;
    array<uint8_t, 4> char_array_4 {};
    array<uint8_t, 3> char_array_3 {};

    dest.reset();

    while (in_len && (encoded_string[in_] != '=') && is_base64((uint8_t) encoded_string[in_]))
    {
        --in_len;
        char_array_4[i] = (uint8_t) encoded_string[in_];
        ++i;
        ++in_;
        if (i == 4)
        {
            for (i = 0; i < 4; ++i)
            {
                char_array_4[i] = (uint8_t) base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = uint8_t(((int) char_array_4[0] << 2) + (((int) char_array_4[1] & 0x30) >> 4));
            char_array_3[1] = uint8_t((((int) char_array_4[1] & 0xf) << 4) + (((int) char_array_4[2] & 0x3c) >> 2));
            char_array_3[2] = uint8_t((((int) char_array_4[2] & 0x3) << 6) + (int) char_array_4[3]);

            dest.append((char*) char_array_3.data(), 3);
            i = 0;
        }
    }

    if (i != 0)
    {
        int j = i;
        for (; j < 4; ++j)
        {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; ++j)
        {
            char_array_4[j] = (uint8_t) base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = uint8_t(((int) char_array_4[0] << 2) + (((int) char_array_4[1] & 0x30) >> 4));
        char_array_3[1] = uint8_t((((int) char_array_4[1] & 0xf) << 4) + (((int) char_array_4[2] & 0x3c) >> 2));
        char_array_3[2] = uint8_t((((int) char_array_4[2] & 0x3) << 6) + (int) char_array_4[3]);

        for (j = 0; (j < i - 1); ++j)
        {
            dest.append((char) char_array_3[j]);
        }
    }

    return dest.bytes();
}

size_t Base64::decode(Buffer& bufDest, const Buffer& bufSource)
{
    string source(bufSource.c_str(), bufSource.bytes());
    return internal_decode(bufDest, source);
}

size_t Base64::decode(Buffer& bufDest, const String& strSource)
{
    return internal_decode(bufDest, strSource);
}

#ifdef USE_GTEST

static const String testPhrase("This is a test");
static const String testPhraseBase64("VGhpcyBpcyBhIHRlc3Q=");

static const String encodedBinary("AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4"
                                  "OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3Bx"
                                  "cnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmq"
                                  "q6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj"
                                  "5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+");

TEST(SPTK_Base64, decodeString)
{
    Buffer decoded;
    Base64::decode(decoded, testPhraseBase64);
    EXPECT_STREQ(testPhrase.c_str(), decoded.c_str());
}

TEST(SPTK_Base64, encodeString)
{
    String encoded;
    Base64::encode(encoded, Buffer(testPhrase));
    EXPECT_STREQ(testPhraseBase64.c_str(), encoded.c_str());
}

TEST(SPTK_Base64, decodeBinary)
{
    Buffer expectedBinary;
    constexpr size_t dataSize {255};
    for (uint8_t i = 0; i < dataSize; i++)
    {
        expectedBinary.append(i);
    }

    Buffer decoded;
    Base64::decode(decoded, encodedBinary);
    EXPECT_STREQ(expectedBinary.c_str(), decoded.c_str());
}

TEST(SPTK_Base64, encodeBinary)
{
    Buffer source;
    constexpr size_t dataSize {255};
    for (uint8_t i = 0; i < dataSize; i++)
    {
        source.append(i);
    }

    source.saveToFile("/tmp/source");

    String encoded;
    Base64::encode(encoded, source);
    EXPECT_STREQ(encodedBinary.c_str(), encoded.c_str());
}

#endif
