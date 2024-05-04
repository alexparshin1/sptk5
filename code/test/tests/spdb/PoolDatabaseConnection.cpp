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
#include <sptk5/cutils>
#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

TEST(SPTK_BulkInsert, escapeSqlString)
{
    String sourceString = "Hello, 'World'.\n\rLet's go\n";
    String escapedString = escapeSQLString(sourceString, false);
    EXPECT_STREQ("Hello, ''World''.\\n\\rLet''s go\\n", escapedString.c_str());
}

TEST(SPTK_BulkInsert, escapeSqlStringPerformance)
{
    constexpr auto maxCount = 100000;
    constexpr auto mcsInSecond = 1E6;
    String sourceString = "Hello, 'World'.\n\rLet's go\n";
    StopWatch stopWatch;
    stopWatch.start();
    for (size_t i = 0; i < maxCount; ++i)
    {
        escapeSQLString(sourceString, false);
    }
    stopWatch.stop();
    COUT("Escaped " << maxCount << " SQLs "
                    << " for " << stopWatch.seconds() << " sec, "
                    << fixed << setprecision(2) << maxCount / stopWatch.seconds() / mcsInSecond << "M op/sec" << endl);
}
