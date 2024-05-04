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

#include <gtest/gtest.h>
#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>
#include <sptk5/cnet>
#include <sptk5/net/RequestInfo.h>

using namespace std;
using namespace sptk;

static Buffer decode(const Buffer& data, const String& encoding)
{
    Buffer decoded;
    if (encoding == "br")
    {
#ifdef HAVE_BROTLI
        Buffer brotliData;
        Brotli::decompress(decoded, data);
        return decoded;
#endif
    }
    if (encoding == "gzip")
    {
#ifdef HAVE_ZLIB
        Buffer brotliData;
        ZLib::decompress(decoded, data);
        return decoded;
#endif
    }
    throw Exception("Unsupported encoding: " + encoding);
}

TEST(SPTK_RequestInfo, Message)
{
    constexpr int nodeCount = 16;
    Buffer testData;
    for (size_t i = 0; i < nodeCount; ++i)
    {
        testData.append("<0123456789=ABCDEF>");
    }

    Strings outputEncodings {"br", "gzip"};
    RequestInfo::Message message;

    message.input(testData, "");
    auto output = message.output(outputEncodings);
    auto decoded = decode(output, message.contentEncoding());

    Buffer urlEncoded(Url::encode(testData.c_str()));
    message.input(urlEncoded, "x-www-form-urlencoded");
    output = message.output(outputEncodings);
    decoded = decode(output, message.contentEncoding());
    EXPECT_STREQ(testData.c_str(), decoded.c_str());

#ifdef HAVE_BROTLI
    Buffer brotliData;
    Brotli::compress(brotliData, testData);
    EXPECT_TRUE(testData.size() > brotliData.size());
    message.input(brotliData, "br");
    output = message.output(outputEncodings);
    decoded = decode(output, message.contentEncoding());
    EXPECT_STREQ(testData.c_str(), decoded.c_str());

    try
    {
        message.input(brotliData, "gzip");
        FAIL() << "MUST FAIL: wrong encoding";
    }
    catch (const Exception&)
    {
        SUCCEED() << "Correct: wrong encoding";
    }
#endif

#ifdef HAVE_ZLIB
    outputEncodings.remove("br");
    Buffer gzipData;
    ZLib::compress(gzipData, testData);
    EXPECT_TRUE(testData.size() > gzipData.size());
    message.input(gzipData, "gzip");
    output = message.output(outputEncodings);
    decoded = decode(output, message.contentEncoding());
    EXPECT_STREQ(testData.c_str(), decoded.c_str());

    try
    {
        message.input(gzipData, "br");
        FAIL() << "MUST FAIL: wrong encoding";
    }
    catch (const Exception&)
    {
        SUCCEED() << "Correct: wrong encoding";
    }
#endif
}
