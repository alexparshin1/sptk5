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

#include <sptk5/Exception.h>
#include <sptk5/RegularExpression.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

namespace {
#ifndef _WIN32
string delimiter = "/";
#else
string delimiter = "\\";
#endif
}

TEST(SPTK_Exception, throwException)
{
    try
    {
        throw Exception("Test exception");
    }
    catch (const Exception& e)
    {
        EXPECT_STREQ(("Test exception in core" + delimiter + "Exception.cpp(45)").c_str(), e.what());
    }

    try
    {
        throw Exception("Test exception", source_location::current(), "This happens sometimes");
    }
    catch (const Exception& e)
    {
        EXPECT_STREQ(("Test exception in core" + delimiter + "Exception.cpp(54). This happens sometimes.").c_str(), e.what());
        EXPECT_STREQ("Test exception", e.message().c_str());
    }
}

TEST(SPTK_HttpException, throwException)
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
        try
        {
            throw HTTPException(code, "Something happened", source_location::current(), "This happens sometimes");
        }
        catch (const HTTPException& e)
        {
            EXPECT_STREQ(("Something happened in core" + delimiter + "Exception.cpp(78). This happens sometimes.").c_str(), e.what());
            EXPECT_STREQ("Something happened", e.message().c_str());
            EXPECT_EQ(code, e.statusCode());
            EXPECT_EQ(expectedStatus, e.statusText());
        }
    }
}
