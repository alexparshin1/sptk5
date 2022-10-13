/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/LogEngine.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

class TestLogEngine : public LogEngine
{
public:
    TestLogEngine()
        : LogEngine("TestLogEngine")
    {
        storage.clear();
    }

    void saveMessage(const Logger::UMessage& message) override
    {
        storage.emplace_back(message);
    }

    static vector<Logger::UMessage> storage;
};

vector<Logger::UMessage> TestLogEngine::storage;

TEST(SPTK_LogEngine, options)
{
    TestLogEngine logEngine;

    EXPECT_EQ((int) logEngine.options(), LO_ENABLE | LO_DATE | LO_TIME | LO_PRIORITY);

    logEngine.option(LO_STDOUT, true);
    EXPECT_EQ((int) logEngine.options() & LO_STDOUT, LO_STDOUT);

    logEngine.option(LO_STDOUT, false);
    EXPECT_EQ((int) logEngine.options() & LO_STDOUT, 0);
}

TEST(SPTK_LogEngine, priorities)
{
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::DEBUG).c_str(), "DEBUG");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::INFO).c_str(), "INFO");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::NOTICE).c_str(), "NOTICE");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::WARNING).c_str(), "WARNING");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::ERR).c_str(), "ERROR");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::CRITICAL).c_str(), "CRITICAL");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::ALERT).c_str(), "ALERT");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::PANIC).c_str(), "PANIC");

    EXPECT_EQ(LogEngine::priorityFromName("DEBUG"), LogPriority::DEBUG);
    EXPECT_EQ(LogEngine::priorityFromName("info"), LogPriority::INFO);
    EXPECT_EQ(LogEngine::priorityFromName("Notice"), LogPriority::NOTICE);
    EXPECT_EQ(LogEngine::priorityFromName("WARNING"), LogPriority::WARNING);
    EXPECT_EQ(LogEngine::priorityFromName("ERROR"), LogPriority::ERR);
    EXPECT_EQ(LogEngine::priorityFromName("CRITICAL"), LogPriority::CRITICAL);
    EXPECT_EQ(LogEngine::priorityFromName("ALERT"), LogPriority::ALERT);
    EXPECT_EQ(LogEngine::priorityFromName("PANIC"), LogPriority::PANIC);
}

TEST(SPTK_LogEngine, message)
{
    auto logEngine = make_shared<TestLogEngine>();
    logEngine->option(LO_STDOUT, true);

    Logger logger(*logEngine);

    logger.debug("debug message");
    logger.info("info message");
    logger.notice("notice message");
    logger.warning("warning message");
    logger.error("error message");
    logger.critical("critical message");
    logger.log(LogPriority::ALERT, "alert message");
    logger.log(LogPriority::PANIC, "panic message");

    this_thread::sleep_for(chrono::milliseconds(10));
    logEngine.reset();

    EXPECT_EQ(TestLogEngine::storage[0]->priority, LogPriority::DEBUG);
    EXPECT_STREQ(TestLogEngine::storage[0]->message.c_str(), "debug message");

    EXPECT_EQ(TestLogEngine::storage[1]->priority, LogPriority::INFO);
    EXPECT_STREQ(TestLogEngine::storage[1]->message.c_str(), "info message");

    EXPECT_EQ(TestLogEngine::storage[6]->priority, LogPriority::ALERT);
    EXPECT_STREQ(TestLogEngine::storage[6]->message.c_str(), "alert message");
}
