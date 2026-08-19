/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-10                                             ║
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

#include <sptk5/Base64.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

namespace {
const String testPhrase("This is a test");
const String testPhraseBase64("VGhpcyBpcyBhIHRlc3Q=");
const String testPhraseBase64WithWhitespaces("VGhpcyBp cyBhIH\nRlc3Q=");

const String encodedBinary("AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4"
                           "OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3Bx"
                           "cnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmq"
                           "q6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj"
                           "5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+");
} // namespace
namespace sptk {

TEST(Base64Tests, decodeString)
{
    Buffer decoded;
    Base64::decode(decoded, testPhraseBase64);
    EXPECT_STREQ(testPhrase.c_str(), decoded.c_str());
}

TEST(Base64Tests, decodeStringWithWhitespaces)
{
    Buffer decoded;
    Base64::decode(decoded, testPhraseBase64WithWhitespaces);
    EXPECT_STREQ(testPhrase.c_str(), decoded.c_str());
}

TEST(Base64Tests, encodeString)
{
    String encoded;
    Base64::encode(encoded, Buffer(testPhrase));
    EXPECT_STREQ(testPhraseBase64.c_str(), encoded.c_str());
}

TEST(Base64Tests, decodeBinary)
{
    Buffer           expectedBinary;
    constexpr size_t dataSize {255};
    for (uint8_t i = 0; i < dataSize; i++)
    {
        expectedBinary.append(i);
    }

    Buffer decoded;
    Base64::decode(decoded, encodedBinary);
    EXPECT_STREQ(expectedBinary.c_str(), decoded.c_str());
}

TEST(Base64Tests, encodeBinary)
{
    Buffer           source;
    constexpr size_t dataSize {255};
    for (uint8_t i = 0; i < dataSize; i++)
    {
        source.append(i);
    }

    String encoded;
    Base64::encode(encoded, source);
    EXPECT_STREQ(encodedBinary.c_str(), encoded.c_str());
}

TEST(Base64Tests, decodeInvalidCharacters)
{
    Buffer decoded;
    String invalid("VGhpcyBpcyBhIHRlc3Q#"); // # is invalid
    Base64::decode(decoded, invalid);
    EXPECT_STREQ("This is a test", decoded.c_str()); // Should ignore #
}

TEST(Base64Tests, decodeEmpty)
{
    Buffer decoded;
    Base64::decode(decoded, String(""));
    EXPECT_EQ(0, decoded.bytes());
}

TEST(Base64Tests, encodeEmpty)
{
    Buffer source;
    String encoded;
    Base64::encode(encoded, source);
    EXPECT_STREQ("", encoded.c_str());
}

TEST(Base64Tests, decodeDifferentPaddings)
{
    Buffer decoded;

    // 1 byte source -> 2 chars + 2 padding
    Base64::decode(decoded, String("YQ=="));
    EXPECT_STREQ("a", decoded.c_str());

    // 2 bytes source -> 3 chars + 1 padding
    Base64::decode(decoded, String("YWI="));
    EXPECT_STREQ("ab", decoded.c_str());

    // 3 bytes source -> 4 chars
    Base64::decode(decoded, String("YWJj"));
    EXPECT_STREQ("abc", decoded.c_str());
}

TEST(Base64Tests, decodeUrlSafe)
{
    Buffer decoded;
    // Standard Base64: "a+b/c" -> "YStiL2M="
    // URL-safe Base64: "a+b/c" -> "YStiL2M=" (but uses - and _ instead of + and /)
    // "a-b_c" in standard is invalid, but our decoder should now handle it.
    Base64::decode(decoded, String("YStiL2M="));
    EXPECT_STREQ("a+b/c", decoded.c_str());

    Base64::decode(decoded, String("YStiL2M")); // Missing padding should also work
    EXPECT_STREQ("a+b/c", decoded.c_str());

    Base64::decode(decoded, String("YStiL18=")); // "a+b/_" -> a+b/ÿ (if _ is /)
    // Wait, let's use a real example.
    // "subjects?_d=1" -> "c3ViamVjdHM/X2Q9MQ=="
    // "subjects?_d=1" (URL safe) -> "c3ViamVjdHM_X2Q9MQ"
    Base64::decode(decoded, String("c3ViamVjdHM_X2Q9MQ"));
    EXPECT_STREQ("subjects?_d=1", decoded.c_str());
}

} // namespace sptk
