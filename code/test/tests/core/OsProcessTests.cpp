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

#include <gtest/gtest.h>
#include <sptk5/OsProcess.h>

using namespace std;
using namespace sptk;

/**
 * @brief Test OsProcess class executes an OS command and captures output.
 */
namespace sptk {
TEST(OsProcessTests, execute)
{
#ifdef _WIN32
    String command("cmd /?");
#else
    String command("ls --help");
#endif

    stringstream str;
    OsProcess    osProcess(command,
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
TEST(OsProcessTests, kill)
{
    Stopwatch stopWatch;
#ifdef _WIN32
    String command("cmd /C sleep 10");
#else
    String command("sleep 10s");
#endif

    stopWatch.start();

    OsProcess osProcess(command);

    osProcess.start();

    this_thread::sleep_for(1s);

    osProcess.kill();

    auto result = osProcess.wait();

    stopWatch.stop();

    EXPECT_LE(0, result);
    EXPECT_LT(1000, stopWatch.milliseconds());
    EXPECT_GT(1100, stopWatch.milliseconds());
}

TEST(OsProcessTests, multiple_waits)
{
#ifdef _WIN32
    String command("cmd /C echo hello");
#else
    String command("echo hello");
#endif

    OsProcess osProcess(command);
    osProcess.start();

    auto result1 = osProcess.wait();
    EXPECT_EQ(0, result1);

    // This should NOT throw std::future_error (no_state) if correctly handled
    auto result2 = osProcess.wait();
    EXPECT_EQ(0, result2);
}

TEST(OsProcessTests, double_close)
{
#ifdef _WIN32
    String command("cmd /C echo hello");
#else
    String command("echo hello");
#endif

    OsProcess osProcess(command);
    osProcess.start();

    // Explicitly call close
    auto result1 = osProcess.close();
    EXPECT_EQ(0, result1);

    // Should be safe to call again (possibly from destructor)
    auto result2 = osProcess.close();
    EXPECT_EQ(0, result2);
}

} // namespace sptk
