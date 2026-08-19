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

#include <gtest/gtest.h>
#include <sptk5/Strings.h>
#include <sptk5/net/HttpParams.h>

using namespace std;
using namespace sptk;

static const String gtestURLencoded("id=1234&items=%5B%22book%22%2C%22pen%22%5D&name=John+Doe");
namespace sptk {

TEST(HttpParamsTests, encode)
{
    HttpParams httpParams;
    httpParams["id"] = "1234";
    httpParams["name"] = "John Doe";
    httpParams["items"] = R"(["book","pen"])";

    Buffer encoded;
    httpParams.encode(encoded);
    EXPECT_STREQ(gtestURLencoded.c_str(), encoded.c_str());
}

TEST(HttpParamsTests, decode)
{
    HttpParams httpParams;
    httpParams["noise"] = "noise";

    const Buffer encoded(gtestURLencoded);
    httpParams.decode(encoded);
    EXPECT_STREQ("1234", httpParams["id"].c_str());
    EXPECT_STREQ("John Doe", httpParams["name"].c_str());
    EXPECT_STREQ(R"(["book","pen"])", httpParams["items"].c_str());
    EXPECT_EQ(static_cast<size_t>(3), httpParams.size());
}

// Keys should be URL-decoded too (e.g. %2B => '+', '+' => space rules apply)
TEST(HttpParamsTests, decodeDecodesKeyAndValue)
{
    HttpParams httpParams;

    const Buffer encoded("a%2Bb=c%2Bd&x+y=1+2");
    httpParams.decode(encoded);

    EXPECT_TRUE(httpParams.has("a+b"));
    EXPECT_STREQ("c+d", httpParams.get("a+b").c_str());

    EXPECT_TRUE(httpParams.has("x y"));
    EXPECT_STREQ("1 2", httpParams.get("x y").c_str());
}

// Parameter names are case-insensitive
TEST(HttpParamsTests, decodeCaseInsensitiveNames)
{
    HttpParams httpParams;

    const Buffer encoded("ID=1234&Name=John+Doe");
    httpParams.decode(encoded);

    EXPECT_TRUE(httpParams.has("id"));
    EXPECT_TRUE(httpParams.has("name"));
    EXPECT_TRUE(httpParams.has("ID"));
    EXPECT_TRUE(httpParams.has("Name"));

    EXPECT_STREQ("1234", httpParams.get("id").c_str());
    EXPECT_STREQ("John Doe", httpParams.get("name").c_str());
}

// Parameters without '=' should map to the empty value
TEST(HttpParamsTests, decodeParameterWithoutEquals)
{
    HttpParams httpParams;

    const Buffer encoded("flag&x=1");
    httpParams.decode(encoded);

    EXPECT_TRUE(httpParams.has("flag"));
    EXPECT_STREQ("", httpParams.get("flag").c_str());

    EXPECT_TRUE(httpParams.has("x"));
    EXPECT_STREQ("1", httpParams.get("x").c_str());
}

// Invalid percent encoding should throw
TEST(HttpParamsTests, decodeThrowsOnInvalidPercentEscape)
{
    HttpParams httpParams;

    EXPECT_ANY_THROW(httpParams.decode(Buffer("a=%ZZ")));
    EXPECT_ANY_THROW(httpParams.decode(Buffer("a=%0G")));
}

// Truncated percent encoding should throw
TEST(HttpParamsTests, decodeThrowsOnTruncatedPercentEscape)
{
    HttpParams httpParams;

    EXPECT_ANY_THROW(httpParams.decode(Buffer("a=%")));
    EXPECT_ANY_THROW(httpParams.decode(Buffer("a=%1")));
    EXPECT_ANY_THROW(httpParams.decode(Buffer("%")));
    EXPECT_ANY_THROW(httpParams.decode(Buffer("%1")));
}

// Valid percent-encoded values at the end of a string should decode successfully
TEST(HttpParamsTests, decodeValidPercentEscapesAtEnd)
{
    HttpParams httpParams;

    httpParams.decode(Buffer("space=%20&slash=%2F&plus=%2B"));

    EXPECT_STREQ(" ", httpParams.get("space").c_str());
    EXPECT_STREQ("/", httpParams.get("slash").c_str());
    EXPECT_STREQ("+", httpParams.get("plus").c_str());
}

// Valid percent-encoded parameter names should decode successfully
TEST(HttpParamsTests, decodeValidPercentEscapesInKey)
{
    HttpParams httpParams;

    httpParams.decode(Buffer("a%2Fb=value"));

    EXPECT_TRUE(httpParams.has("a/b"));
    EXPECT_STREQ("value", httpParams.get("a/b").c_str());
}

// encode() should produce a fresh encoded string in output buffer
TEST(HttpParamsTests, encodeOverwritesOutputBuffer)
{
    HttpParams httpParams;
    httpParams["x"] = "1";
    httpParams["y"] = "2";

    Buffer encoded("prefix");
    httpParams.encode(encoded);

    EXPECT_STREQ("x=1&y=2", encoded.c_str());
}

} // namespace sptk
