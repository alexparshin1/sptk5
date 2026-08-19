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

#include <sptk5/Exception.h>
#include <sptk5/RegularExpression.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;
namespace sptk {

TEST(ExceptionTests, throwException)
{
    // The line is taken relative to the throw rather than written out, so that the test asserts
    // that a line is reported and not which one. Written out, it fails whenever anything above it
    // in the file grows - a licence banner, an include - and says "exception" while meaning
    // "someone edited the top of this file".
    size_t plainThrowLine = 0;
    try
    {
        plainThrowLine = __LINE__ + 1;
        throw Exception("Test exception");
    }
    catch (const Exception& e)
    {
        EXPECT_EQ("Test exception in ExceptionTests.cpp:" + to_string(plainThrowLine), string(e.what()));
    }

    size_t describedThrowLine = 0;
    try
    {
        describedThrowLine = __LINE__ + 1;
        throw Exception("Test exception", source_location::current(), "This happens sometimes");
    }
    catch (const Exception& e)
    {
        EXPECT_EQ("Test exception in ExceptionTests.cpp:" + to_string(describedThrowLine) +
                      ". This happens sometimes.",
                  string(e.what()));
        EXPECT_STREQ("Test exception", e.message().c_str());
    }
}

TEST(HttpExceptionTests, throwException)
{
    constexpr size_t firstErrorCode = 400;
    constexpr size_t maxErrorCode = 512;
    for (size_t code = firstErrorCode; code < maxErrorCode; ++code)
    {
        const auto expectedStatus = HTTPException::httpResponseStatus(code);
        if (expectedStatus.empty())
        {
            continue;
        }
        size_t throwLine = 0;
        try
        {
            throwLine = __LINE__ + 1;
            throw HTTPException(code, "Something happened", source_location::current(), "This happens sometimes");
        }
        catch (const HTTPException& e)
        {
            EXPECT_EQ("Something happened in ExceptionTests.cpp:" + to_string(throwLine) +
                          ". This happens sometimes.",
                      string(e.what()));
            EXPECT_STREQ("Something happened", e.message().c_str());
            EXPECT_EQ(code, e.statusCode());
            EXPECT_EQ(expectedStatus, e.statusText());
        }
    }
}

} // namespace sptk
