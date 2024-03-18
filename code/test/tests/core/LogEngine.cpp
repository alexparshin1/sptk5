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
        const scoped_lock lock(m_mutex);
        storage.clear();
    }

    void saveMessage(const Logger::Message& message) override
    {
        const scoped_lock lock(m_mutex);
        storage.push_back(message);
    }

    std::mutex m_mutex;
    static vector<Logger::Message> storage;
};

vector<Logger::Message> TestLogEngine::storage;

TEST(SPTK_LogEngine, options)
{
    TestLogEngine logEngine;

    EXPECT_TRUE(logEngine.option(LogEngine::Option::ENABLE));
    EXPECT_FALSE(logEngine.option(LogEngine::Option::STDOUT));

    logEngine.option(LogEngine::Option::STDOUT, true);
    EXPECT_TRUE(logEngine.option(LogEngine::Option::STDOUT));
}

TEST(SPTK_LogEngine, priorities)
{
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Debug).c_str(), "DEBUG");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Info).c_str(), "Info");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Notice).c_str(), "NOTICE");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Warning).c_str(), "WARNING");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Error).c_str(), "ERROR");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Critical).c_str(), "Critical");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Alert).c_str(), "Alert");
    EXPECT_STREQ(LogEngine::priorityName(LogPriority::Panic).c_str(), "Panic");

    EXPECT_TRUE(LogEngine::priorityFromName("DEBUG") == LogPriority::Debug);
    EXPECT_TRUE(LogEngine::priorityFromName("info") == LogPriority::Info);
    EXPECT_TRUE(LogEngine::priorityFromName("Notice") == LogPriority::Notice);
    EXPECT_TRUE(LogEngine::priorityFromName("WARNING") == LogPriority::Warning);
    EXPECT_TRUE(LogEngine::priorityFromName("ERROR") == LogPriority::Error);
    EXPECT_TRUE(LogEngine::priorityFromName("Critical") == LogPriority::Critical);
    EXPECT_TRUE(LogEngine::priorityFromName("Alert") == LogPriority::Alert);
    EXPECT_TRUE(LogEngine::priorityFromName("Panic") == LogPriority::Panic);
}

TEST(SPTK_LogEngine, message)
{
    auto logEngine = make_shared<TestLogEngine>();
    logEngine->option(LogEngine::Option::STDOUT, true);
    logEngine->minPriority(LogPriority::Debug);

    Logger logger(*logEngine);

    logger.debug("debug message");
    logger.info("info message");
    logger.notice("notice message");
    logger.warning("warning message");
    logger.error("error message");
    logger.critical("critical message");
    logger.log(LogPriority::Alert, "alert message");
    logger.log(LogPriority::Panic, "panic message");

    this_thread::sleep_for(chrono::milliseconds(10));
    logEngine.reset();

    EXPECT_TRUE(TestLogEngine::storage[0].priority == LogPriority::Debug);
    EXPECT_STREQ(TestLogEngine::storage[0].message.c_str(), "debug message");

    EXPECT_TRUE(TestLogEngine::storage[1].priority == LogPriority::Info);
    EXPECT_STREQ(TestLogEngine::storage[1].message.c_str(), "info message");

    EXPECT_TRUE(TestLogEngine::storage[6].priority == LogPriority::Alert);
    EXPECT_STREQ(TestLogEngine::storage[6].message.c_str(), "alert message");
}
