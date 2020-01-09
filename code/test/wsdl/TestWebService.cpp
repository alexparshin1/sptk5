/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include "TestWebService.h"

using namespace std;
using namespace sptk;

void TestWebService::Hello(const CHello& input, CHelloResponse& output, HttpAuthentication*)
{
    if (input.m_first_name.asString() != "John" || input.m_last_name.asString() != "Doe")
        throw Exception("Invalid first or last name: expecting John Doe");
    output.m_date_of_birth = DateTime("1981-02-01");
    output.m_height = 6.5;
    output.m_hour_rate = 15.6;
    output.m_retired = false;
    output.m_vacation_days = 21;
    output.m_verified = DateTime("2020-01-02 10:00:00+10");
}

#if USE_GTEST

TEST(SPTK_TestWebService, hello)
{
    TestWebService service;

    CHello hello;
    hello.m_first_name = "John";
    hello.m_last_name = "Doe";

    CHelloResponse response;
    service.Hello(hello, response, nullptr);

    if (response.m_date_of_birth.asDate() != DateTime("1981-02-01").date())
        FAIL() << "m_date_of_birth has invalid value";
    if (response.m_verified.asDateTime() != DateTime("2020-01-02 10:00:00+10"))
        FAIL() << "m_verified has invalid value";
    EXPECT_DOUBLE_EQ(response.m_height, 6.5);
    EXPECT_DOUBLE_EQ(response.m_hour_rate, 15.6);
    EXPECT_EQ(response.m_retired.asBool(), false);
    EXPECT_EQ(response.m_vacation_days.asInteger(), 21);
}
#endif
