/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CBase64.cpp - description                              ║
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

#include <sptk5/Base64.h>

using namespace std;
using namespace sptk;

static char B64Chars[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};

//#define base64val(c) Index_64[(uint32_t)(c)]
#define base64chars(c) B64Chars[(uint32_t)((c) & 0x3F)]

void Base64::encode(Buffer& bufDest, const char* bufSource, size_t len)
{
    const char* current = bufSource;
    auto outputLen = size_t(len / 3 * 4);
    if ((len % 3) != 0)
        outputLen += 4;
    bufDest.checkSize(outputLen + 1);
    char* output = bufDest.data();

    while (len >= 3) {
        *output = base64chars((current[0] & 0xFC) >> 2);
        output++;

        *output = base64chars(((current[0] & 0x03) << 4) | ((current[1] & 0xF0) >> 4));
        output++;

        *output = base64chars(((current[1] & 0x0F) << 2) | ((current[2] & 0xC0) >> 6));
        output++;

        *output = base64chars(current[2] & 0x3F);
        output++;

        len     -= 3;
        current += 3;   /* move pointer 3 characters forward */
    }

    /// Now we should clean up remainder
    if (len > 0) {
        *output = base64chars(current[0] >> 2);
        output++;
        if (len > 1) {
            *output = base64chars(((current[0] & 0x03) << 4) | ((current[1] & 0xF0) >> 4));
            output++;
            *output = base64chars((current[1] & 0x0f) << 2);
            output++;
            *output = '=';
            output++;
        }
        else {
            *output = base64chars((current[0] & 0x03) << 4);
            output++;
            *output = '=';
            output++;
            *output = '=';
            output++;
        }
    }
    *output = 0;
    bufDest.bytes(outputLen);
}

void Base64::encode(Buffer& bufDest, const Buffer& bufSource)
{
    encode(bufDest, bufSource.data(), bufSource.bytes());
}

void Base64::encode(string& strDest, const Buffer& bufSource)
{
    Buffer bufOut;
    encode(bufOut, bufSource);

    strDest = string(bufOut.c_str(), bufOut.bytes());
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
	uint8_t char_array_4[4] {}, char_array_3[3] {};

    dest.reset();

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64((uint8_t) encoded_string[in_])) {
    char_array_4[i++] = (uint8_t) encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = (uint8_t) base64_chars.find(char_array_4[i]);

      char_array_3[0] = uint8_t( (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4) );
      char_array_3[1] = uint8_t( ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2) );
      char_array_3[2] = uint8_t( ((char_array_4[2] & 0x3) << 6) + char_array_4[3] );

      dest.append((char*)char_array_3, 3);
      i = 0;
    }
  }

  if (i != 0) {
    int j = 0;
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = (uint8_t) base64_chars.find(char_array_4[j]);

    char_array_3[0] = uint8_t( (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4) );
    char_array_3[1] = uint8_t( ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2) );
    char_array_3[2] = uint8_t( ((char_array_4[2] & 0x3) << 6) + char_array_4[3] );

    for (j = 0; (j < i - 1); j++)
        dest.append((char)char_array_3[j]);
  }

  return dest.bytes();
}

size_t Base64::decode(Buffer& bufDest, const Buffer& bufSource)
{
    string source(bufSource.c_str(), bufSource.bytes());
    return internal_decode(bufDest, source);
}

size_t Base64::decode(Buffer &bufDest, const string& strSource)
{
    return internal_decode(bufDest, strSource);
}

#if USE_GTEST
#include <gtest/gtest.h>

static const char* testPhrase = "This is a test";
static const char* testPhraseBase64 = "VGhpcyBpcyBhIHRlc3Q=";

TEST(SPTK_Base64, decode)
{
    Buffer decoded;
    Base64::decode(decoded, testPhraseBase64);
    EXPECT_STREQ(testPhrase, decoded.c_str());
}

TEST(SPTK_Base64, encode)
{
    string encoded;
    Base64::encode(encoded, Buffer(testPhrase));
    EXPECT_STREQ(testPhraseBase64, encoded.c_str());
}

#endif