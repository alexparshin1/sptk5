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

#include <gtest/gtest.h>
#include <sptk5/FileLogEngine.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

static const filesystem::path logFileName("/tmp/file_log_test.tmp");

static shared_ptr<FileLogEngine> makeFileLogEngine(LogPriority minLogPriority)
{
    unlink(logFileName.string().c_str());

    auto logEngine = make_shared<FileLogEngine>(logFileName);
    logEngine->minPriority(minLogPriority);

    return logEngine;
}

static void logMessages(const shared_ptr<LogEngine>& logEngine)
{
    auto logger = make_shared<Logger>(*logEngine, "(Test application) ");
    logger->debug("Test started");
    logger->critical("Critical message");
    logger->error("Error message");
    logger->warning("Warning message");
    logger->info("Test completed");

    this_thread::sleep_for(chrono::milliseconds(50));
}

static void testPriority(LogPriority priority, size_t expectedMessageCount)
{
    auto logEngine = makeFileLogEngine(priority);

    logEngine->reset();

    logMessages(logEngine);

    Strings content;
    content.loadFromFile(logFileName);

    EXPECT_EQ(expectedMessageCount, content.size());
}

TEST(SPTK_FileLogEngine, testLogPriorities)
{
    testPriority(LogPriority::DEBUG, 5);
    testPriority(LogPriority::INFO, 4);
    testPriority(LogPriority::ERR, 2);
}

TEST(SPTK_FileLogEngine, performance)
{
    FileLogEngine logEngine(logFileName);
    Logger logger(logEngine, "(Test application) ");
    StopWatch stopWatch;
    stopWatch.start();
    constexpr size_t messageCount = 100000;
    for (size_t i = 0; i < messageCount; i++)
    {
        logger.info("Test log message of some length");
    }
    stopWatch.stop();
    COUT("Logged " << messageCount << " messages for " << stopWatch.milliseconds() << "ms ("
                   << messageCount / (int) stopWatch.milliseconds() << " msgs/sec)" << endl);
}
