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
#include <sptk5/FileLogEngine.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

namespace {

#ifdef _WIN32
const filesystem::path logFileName("C:/Windows/temp/file_log_test.tmp");
#else
const filesystem::path logFileName("/tmp/file_log_test.tmp");
#endif

void logMessages(LogEngine& logEngine)
{
    const auto logger = make_shared<Logger>(logEngine, "(Test application) ");
    logger->debug("Test started");
    logger->critical("Critical message");
    logger->error("Error message");
    logger->warning("Warning message");
    logger->info("Test completed");

    this_thread::sleep_for(100ms);
}

void testPriority(FileLogEngine& logEngine, LogPriority priority, size_t expectedMessageCount)
{
    logEngine.reset();
    logEngine.minPriority(priority);

    logMessages(logEngine);
    logEngine.flush();

    Strings content;
    content.loadFromFile(logFileName);

    EXPECT_EQ(expectedMessageCount, content.size());
}

} // namespace
namespace sptk {

TEST(FileLogEngineTests,testLogPriorities)
{
    FileLogEngine logEngine(logFileName);

    testPriority(logEngine, LogPriority::Debug, 5);
    testPriority(logEngine, LogPriority::Info, 4);
    testPriority(logEngine, LogPriority::Error, 2);
}

TEST(FileLogEngineTests, rotateKeepsTheOldLogAndStartsAnEmptyOne)
{
    FileLogEngine logEngine(logFileName);
    logEngine.reset();
    logMessages(logEngine);
    logEngine.flush();

    Strings before;
    before.loadFromFile(logFileName);
    ASSERT_FALSE(before.empty());

    const auto archived = logEngine.rotate();

    // The old log is kept, under a name derived from the original.
    ASSERT_FALSE(archived.empty());
    EXPECT_TRUE(filesystem::exists(archived));
    EXPECT_TRUE(archived.string().starts_with(logFileName.string() + "."));
    Strings kept;
    kept.loadFromFile(archived);
    EXPECT_EQ(before.size(), kept.size());

    // And the log carries on under its own name, from empty.
    ASSERT_TRUE(filesystem::exists(logFileName));
    EXPECT_EQ(0U, filesystem::file_size(logFileName));

    const auto logger = make_shared<Logger>(logEngine, "(Test application) ");
    logger->info("After rotation");
    this_thread::sleep_for(100ms);
    logEngine.flush();

    Strings after;
    after.loadFromFile(logFileName);
    EXPECT_EQ(1U, after.size());

    filesystem::remove(archived);
}

TEST(FileLogEngineTests, rotateTwiceInTheSameMinuteKeepsBoth)
{
    FileLogEngine logEngine(logFileName);
    logEngine.reset();

    logMessages(logEngine);
    logEngine.flush();
    const auto first = logEngine.rotate();

    logMessages(logEngine);
    logEngine.flush();
    const auto second = logEngine.rotate();

    // The second rotation lands in the same minute as the first, and must not write over it.
    ASSERT_FALSE(first.empty());
    ASSERT_FALSE(second.empty());
    EXPECT_NE(first, second);
    EXPECT_TRUE(filesystem::exists(first));
    EXPECT_TRUE(filesystem::exists(second));

    filesystem::remove(first);
    filesystem::remove(second);
}

TEST(FileLogEngineTests, rotateAnEmptyLogKeepsNothing)
{
    FileLogEngine logEngine(logFileName);
    logEngine.reset();

    // Nothing has been written, so there is nothing worth a timestamped name. A service that
    // restarts often would otherwise leave a directory of empty files behind it.
    EXPECT_TRUE(logEngine.rotate().empty());
    EXPECT_TRUE(filesystem::exists(logFileName));
}

TEST(FileLogEngineTests,performance)
{
    FileLogEngine logEngine(logFileName);
    Logger        logger(logEngine, "(Test application) ");
    Stopwatch     stopWatch;
    stopWatch.start();
    constexpr size_t messageCount = 100000;
    for (size_t i = 0; i < messageCount; i++)
    {
        logger.info("Test log message of some length");
    }
    stopWatch.stop();
    COUT("Logged " << messageCount << " messages for " << stopWatch.milliseconds() << "ms ("
                   << static_cast<double>(messageCount) / stopWatch.milliseconds() << " messages/sec)\n");
}

} // namespace sptk_test
