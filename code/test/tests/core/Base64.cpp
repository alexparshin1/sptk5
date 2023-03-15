/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

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
