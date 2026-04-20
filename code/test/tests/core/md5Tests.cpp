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

#include <sptk5/Stopwatch.h>
#include <sptk5/cutils>

#include <array>
#include <gtest/gtest.h>
#include <sstream>

using namespace std;
using namespace sptk;

static const String testPhrase("This is a test text to verify MD5 algorithm");

static const String testSQL(
    "SELECT * FROM schema1.employee "
    "JOIN schema1.department ON employee.department_id = department.id "
    "JOIN schema1.city ON employee.city_id = city_id "
    "WHERE employee.id in (1,2,3,4) "
    "AND employee.name LIKE 'John%' "
    "AND department.name = 'Information Technologies' "
    "LIMIT 1024");
namespace sptk {

TEST(MD5Tests, md5)
{
    String testMD5 = md5(testPhrase);
    EXPECT_STREQ("7d84a2b9dfe798bdbf9ad343bde9322d", testMD5.c_str());

    testMD5 = md5(Buffer(testPhrase));
    EXPECT_STREQ("7d84a2b9dfe798bdbf9ad343bde9322d", testMD5.c_str());
}

TEST(MD5Tests, rfc1321Vectors)
{
    static constexpr std::array<std::pair<const char*, const char*>, 7> vectors = {{
        {"", "d41d8cd98f00b204e9800998ecf8427e"},
        {"a", "0cc175b9c0f1b6a831c399e269772661"},
        {"abc", "900150983cd24fb0d6963f7d28e17f72"},
        {"message digest", "f96b697d7cb7938d525a2f31aaf161d0"},
        {"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"},
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f"},
        {"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
         "57edf4a22be3c955ac49da2e2107b67a"},
    }};

    for (const auto& [input, expected]: vectors)
    {
        EXPECT_STREQ(expected, md5(String(input)).c_str());
        EXPECT_STREQ(expected, md5(Buffer(input, strlen(input))).c_str());
    }
}

TEST(MD5Tests, incrementalUpdateMatchesOneShot)
{
    const std::array<unsigned char, 256> bytes = []
    {
        std::array<unsigned char, 256> b {};
        for (size_t i = 0; i < b.size(); ++i)
        {
            b[i] = static_cast<unsigned char>(i);
        }
        return b;
    }();

    const String expected = md5(Buffer(bytes.data(), bytes.size()));
    for (const auto chunkSize: {1UL, 2UL, 7UL, 31UL, 64UL, 127UL})
    {
        MD5 md5ByChunks;
        for (size_t offset = 0; offset < bytes.size(); offset += chunkSize)
        {
            const auto len = std::min(chunkSize, bytes.size() - offset);
            md5ByChunks.update(&bytes[offset], len);
        }
        EXPECT_EQ(expected, md5ByChunks.finalize().hexDigest()) << "chunkSize=" << chunkSize;
    }
}

TEST(MD5Tests, hexDigestRequiresFinalize)
{
    MD5 digest;
    EXPECT_THROW((void) digest.hexDigest(), Exception);
}

TEST(MD5Tests, performance)
{
    Stopwatch        stopWatch;
    constexpr size_t iterations = 200000;

    stopWatch.start();
    for (size_t i = 0; i < iterations; ++i)
    {
        auto testMD5 = md5(Buffer(testSQL));
    }
    stopWatch.stop();

    COUT("Computed " << iterations << " MD5s for " << fixed << setprecision(1) << stopWatch.seconds() << " seconds, "
                     << iterations / stopWatch.seconds() << " per second" << endl);
}

} // namespace sptk
