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
#include <sptk5/OsProcess.h>

using namespace std;
using namespace sptk;

/**
 * @brief Test OsProcess class executes an OS command and captures output.
 */
TEST(SPTK_OsProcess, execute)
{
#ifdef _WIN32
    String command("cmd /?");
#else
    String command("ls -h");
#endif

    stringstream str;
    OsProcess osProcess(command,
                        [&str](const String& text)
                        {
                            str << text;
                        });
    osProcess.start();

    auto result = osProcess.wait();

    Strings output(str.str(), "\n");

    EXPECT_LE(0, result);
    EXPECT_GE(1, result);
    EXPECT_LE(10, output.size());
}

/**
 * @brief Test OsProcess class start and kills a long-running OS command.
 */
TEST(SPTK_OsProcess, kill)
{
    StopWatch stopWatch;
#ifdef _WIN32
    String command("cmd /C sleep 10");
#else
    String command("sleep 10s");
#endif

    stopWatch.start();

    OsProcess osProcess(command,
                        [](const String& text)
                        {
                            cout << text << flush;
                        });

    osProcess.start();

    this_thread::sleep_for(1s);

    osProcess.kill();

    auto result = osProcess.wait();

    stopWatch.stop();

    EXPECT_LE(0, result);
    EXPECT_GE(1, result);

    EXPECT_LT(1000, stopWatch.milliseconds());
    EXPECT_GT(1100, stopWatch.milliseconds());
}
