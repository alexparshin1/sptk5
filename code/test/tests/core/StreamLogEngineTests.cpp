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
#include <sptk5/StreamLogEngine.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

namespace {

stringstream testStream;

void logMessages(LogEngine& logEngine)
{
    const auto logger = make_shared<Logger>(logEngine, "(Test application) ");
    logger->debug([]
                  {
                      return "Test started";
                  });
    logger->critical([]
                     {
                         return "Critical message";
                     });
    logger->error([]
                  {
                      return "Error message";
                  });
    logger->warning([]
                    {
                        return "Warning message";
                    });
    logger->info([]
                 {
                     return "Test completed";
                 });

    this_thread::sleep_for(100ms);
}

void testPriority(StreamLogEngine& logEngine, LogPriority priority, size_t expectedMessageCount)
{
    testStream.str("");
    testStream.clear();
    logEngine.minPriority(priority);

    logMessages(logEngine);

    const Strings content(testStream.str(), "\n");

    EXPECT_EQ(expectedMessageCount, content.size());
}

} // namespace
namespace sptk {

TEST(StreamLogEngineTests, testLogPriorities)
{
    StreamLogEngine logEngine(testStream);

    using enum LogPriority;
    testPriority(logEngine, Debug, 5);
    testPriority(logEngine, Info, 4);
    testPriority(logEngine, Error, 2);
}

TEST(StreamLogEngineTests, messageAsString)
{
    testStream.str("");
    StreamLogEngine logEngine(testStream);

    const Logger     logger(logEngine, "(Test application) ");
    Stopwatch        stopWatch;
    constexpr size_t messageCount = 100000;
    for (size_t i = 0; i < messageCount; i++)
    {
        logger.info("Test log message of some length");
    }
    stopWatch.stop();
    COUT("Logged " << messageCount << " messages for " << stopWatch.milliseconds() << "ms ("
                   << static_cast<double>(messageCount) / stopWatch.milliseconds() << " messages/sec)\n");
}

TEST(StreamLogEngineTests, messageAsLambda)
{
    testStream.str("");
    testStream.clear();

    StreamLogEngine logEngine(testStream);

    const Logger     logger(logEngine, "(Test application) ");
    Stopwatch        stopWatch;
    constexpr size_t messageCount = 100000;
    for (size_t i = 0; i < messageCount; i++)
    {
        logger.info([]
                    {
                        return "Test log message of some length";
                    });
    }
    stopWatch.stop();
    COUT("Logged " << messageCount << " messages for " << stopWatch.milliseconds() << "ms ("
                   << static_cast<double>(messageCount) / stopWatch.milliseconds() << " messages/sec)\n");
}

TEST(StreamLogEngineTests, disabledOptionSkipsOutput)
{
    testStream.str("");
    testStream.clear();
    StreamLogEngine logEngine(testStream);
    logEngine.option(LogEngine::Option::ENABLE, false);

    Logger logger(logEngine);
    logger.error("This message should not be logged");

    this_thread::sleep_for(100ms);

    EXPECT_TRUE(testStream.str().empty());
}

TEST(StreamLogEngineTests, outputFormatWithoutDateAndTime)
{
    testStream.str("");
    testStream.clear();
    StreamLogEngine logEngine(testStream);
    logEngine.options({LogEngine::Option::ENABLE, LogEngine::Option::PRIORITY});

    Logger logger(logEngine);
    logger.info("Formatted message");

    this_thread::sleep_for(100ms);

    EXPECT_STREQ(testStream.str().c_str(), "[Info] Formatted message\n");
}

TEST(StreamLogEngineTests, millisecondsOption)
{
    testStream.str("");
    testStream.clear();
    StreamLogEngine logEngine(testStream);
    logEngine.options({LogEngine::Option::ENABLE, LogEngine::Option::TIME, LogEngine::Option::MILLISECONDS});

    Logger logger(logEngine);
    logger.warning("Message with milliseconds");

    this_thread::sleep_for(100ms);

    const Strings lines(testStream.str(), "\n");
    ASSERT_EQ(1, lines.size());

    const auto& line = lines[0];
    const auto  firstSpacePos = line.find(' ');
    ASSERT_NE(String::npos, firstSpacePos);

    const std::string timeToken = line.substr(0, firstSpacePos);
    EXPECT_NE(String::npos, timeToken.find('.'));
}

} // namespace sptk
