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

#include <sptk5/StopWatch.h>
#include <sptk5/cutils>

#include <gtest/gtest.h>

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

TEST(SPTK_MD5, md5)
{
    String testMD5 = md5(testPhrase);
    EXPECT_STREQ("7d84a2b9dfe798bdbf9ad343bde9322d", testMD5.c_str());

    testMD5 = md5(Buffer(testPhrase));
    EXPECT_STREQ("7d84a2b9dfe798bdbf9ad343bde9322d", testMD5.c_str());
}

TEST(SPTK_MD5, performance)
{
    StopWatch stopWatch;
    const size_t iterations = 200000;

    stopWatch.start();
    for (size_t i = 0; i < iterations; ++i)
    {
        auto testMD5 = md5(Buffer(testSQL));
    }
    stopWatch.stop();

    COUT("Computed " << iterations << " MD5s for " << fixed << setprecision(1) << stopWatch.seconds() << " seconds, "
                     << iterations / stopWatch.seconds() << " per second" << endl);
}
