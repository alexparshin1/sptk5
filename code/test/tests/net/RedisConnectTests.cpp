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
#ifndef _WIN32
#include "sptk5/Printer.h"
#include "sptk5/Stopwatch.h"
#include "sptk5/db/DatabaseConnectionPool.h"
#include "sptk5/db/Query.h"
#include "sptk5/db/Transaction.h"
#include "sptk5/net/RedisConnect.h"
#include <chrono>
#include <cstdlib>
#include <gtest/gtest.h>
#include <ocilibcpp/detail/Transaction.hpp>
#include <thread>

using namespace sptk;
using namespace std;

class RedisConnectTests : public ::testing::Test
{
};

namespace sptk {

TEST_F(RedisConnectTests, connectDisconnect)
{
    RedisConnect redis;
    EXPECT_NO_THROW(redis.connect("127.0.0.1", 6379));
    EXPECT_NO_THROW(redis.disconnect());
}

TEST_F(RedisConnectTests, setGet)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const string  key = "test_key";
    const Variant value = "test_value";

    EXPECT_NO_THROW(redis.set(key, value));
    EXPECT_EQ(redis.get(key).asString(), value.asString());

    redis.disconnect();
}

TEST_F(RedisConnectTests, setGetInt)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const string  key = "int_key";
    const Variant value = static_cast<int64_t>(1234567890123LL);

    EXPECT_NO_THROW(redis.set(key, value));
    EXPECT_EQ(redis.get(key).asInt64(), value.asInt64());

    redis.disconnect();
}

TEST_F(RedisConnectTests, setGetBool)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const string key_t = "bool_key_t";
    const string key_f = "bool_key_f";

    EXPECT_NO_THROW(redis.set(key_t, true));
    EXPECT_EQ(redis.get(key_t).asBool(), true);

    EXPECT_NO_THROW(redis.set(key_f, false));
    EXPECT_EQ(redis.get(key_f).asBool(), false);

    redis.disconnect();
}

TEST_F(RedisConnectTests, setGetDouble)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const string   key = "double_key";
    constexpr auto value = 3.1415926535;

    EXPECT_NO_THROW(redis.set(key, value));
    // to_string(double) might have different precision than what Redis returns or how it's stored,
    // but we implemented it using to_string(value)
    EXPECT_EQ(redis.get(key).asFloat(), value);

    redis.disconnect();
}

TEST_F(RedisConnectTests, setGetBinary)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const string key = "binary_key";
    const char   data[] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x04, 0x05};
    const Buffer binaryData(data, sizeof(data));

    EXPECT_NO_THROW(redis.set(key, binaryData));

    const auto retrievedData = redis.get(key).asBuffer();
    EXPECT_EQ(retrievedData, binaryData);

    redis.disconnect();
}

TEST_F(RedisConnectTests, getNonExistentKey)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    EXPECT_EQ(redis.get("non_existent_key").asString(), "");

    redis.disconnect();
}

TEST_F(RedisConnectTests, setOverwrites)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const string key = "overwrite_key_different";
    redis.set(key, "value1");
    EXPECT_EQ(redis.get(key).asString(), "value1");

    redis.set(key, "value2");
    EXPECT_EQ(redis.get(key).asString(), "value2");

    redis.disconnect();
}

namespace {

const String sessionJson = R"({
    "session_id":12345,
    "client_name":"client1",
    "clean_session":true,
    "subscriptions":[
        {"id":12345, topic:"devices/usb/12345"},
        {"id":12346, topic:"sensors/temperature/123456"}
    ],
})";

}

TEST_F(RedisConnectTests, performanceSingleThread)
{
    constexpr auto iterations = 100;

    RedisConnect redis;
    redis.connect("localhost", 6379);

    Stopwatch watch;
    watch.start();
    for (auto i = 0; i < iterations; ++i)
    {
        auto key = format("key_{}", i);
        redis.set(key, sessionJson);
    }
    watch.stop();
    cout << format("Set performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());

    redis.disconnect();
}

TEST_F(RedisConnectTests, performanceMultipleThreads)
{
    constexpr auto iterations = 1000;
    constexpr auto threadCount = 128;

    Stopwatch watch;
    watch.start();

    vector<jthread> threads;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([threadIndex]
                             {
                                 RedisConnect redis;
                                 redis.connect("theater", 6379);

                                 for (auto i = 0; i < iterations; ++i)
                                 {
                                     auto key = format("session_{}_{}", threadIndex, i);
                                     redis.set(key, sessionJson);
                                 }

                                 redis.disconnect();
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    watch.stop();
    cout << format("Set performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations * threadCount / watch.milliseconds());
}

TEST_F(RedisConnectTests, performanceMultipleThreadsPG)
{
    DatabaseConnectionPool connectionPool("postgresql://gtest:test#123@theater/xmq_test");
    auto                   db = connectionPool.getConnection();

    Query dropTable(db, "DROP TABLE IF EXISTS test_table");
    dropTable.exec();

    Query createTable(db, "CREATE TABLE test_table (key VARCHAR(40) PRIMARY KEY, value VARCHAR)");
    createTable.exec();

    constexpr auto iterations = 1000;
    constexpr auto threadCount = 128;

    Stopwatch watch;
    watch.start();

    vector<jthread> threads;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([threadIndex, &connectionPool]
                             {
                                 const auto conn = connectionPool.getConnection();
                                 Query      insert(conn, "INSERT INTO test_table (key, value) VALUES (:key, :value)");

                                 Transaction transaction(conn);
                                 transaction.begin();
                                 for (auto i = 0; i < iterations; ++i)
                                 {
                                     const auto key = format("session_{}_{}", threadIndex, i);
                                     insert.param(0) = key;
                                     insert.param(1) = sessionJson;
                                     insert.exec();
                                 }
                                 transaction.commit();

                                 conn->close();
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    watch.stop();
    cout << format("Set performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations * threadCount / watch.milliseconds());
}

TEST_F(RedisConnectTests, mget)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    redis.set("mget-key1", 12345);
    redis.set("mget-key2", 1234);

    std::vector<Variant> values = redis.mget({"mget-key1", "mget-key2"});

    Strings keys;
    keys.resize(values.size());
    ranges::transform(values, keys.begin(), [](const Variant& value)
                      {
                          COUT(format("key: {}", value.asString().c_str()));
                          return value.asString();
                      });

    // Expect two keys matched
    ASSERT_EQ(2, values.size());

    EXPECT_NE(-1, keys.indexOf("12345"));
    EXPECT_NE(-1, keys.indexOf("1234"));
}

TEST_F(RedisConnectTests, mgetPerformance)
{
    RedisConnect redis;
    redis.connect("theater", 6379);

    ASSERT_TRUE(redis.isConnected());

    constexpr auto iterations = 10000;
    vector<string> keys;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("mget-perf-key{}", i));
    }

    for (auto i = 0; i < iterations; ++i)
    {
        redis.set(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    std::vector<Variant> values = redis.mget(keys);
    watch.stop();

    cout << format("Mget performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, scan)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    redis.set("scan-key1", 12345);
    redis.set("scan-key2", 1234);

    std::vector<Variant> values = redis.scan("scan-*", 2);

    Strings keys;
    keys.resize(values.size());
    ranges::transform(values, keys.begin(), [](const Variant& value)
                      {
                          COUT(format("key: {}", value.asString().c_str()));
                          return value.asString();
                      });

    // Expect two keys matched
    ASSERT_EQ(2, values.size());

    EXPECT_NE(-1, keys.indexOf("scan-key1"));
    EXPECT_NE(-1, keys.indexOf("scan-key2"));

    redis.disconnect();
}

TEST_F(RedisConnectTests, scanPerformance)
{
    RedisConnect redis;
    redis.connect("theater", 6379);

    ASSERT_TRUE(redis.isConnected());

    constexpr auto iterations = 10;
    vector<string> keys;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("scan-perf-key{}", i));
    }

    for (auto i = 0; i < iterations; ++i)
    {
        redis.set(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    const vector<Variant> keysFound = redis.scan("scan-perf-key{}", iterations);
    watch.stop();

    EXPECT_EQ(iterations, keysFound.size());

    cout << format("Scan performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, scanAndMgetPerformance)
{
    RedisConnect redis;
    redis.connect("theater", 6379);

    ASSERT_TRUE(redis.isConnected());

    constexpr auto iterations = 10;
    vector<string> keys;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("scan-perf-key{}", i));
    }

    for (auto i = 0; i < iterations; ++i)
    {
        redis.set(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    const vector<Variant> keysFound = redis.scan("scan-perf-key*", iterations);
    const vector<Variant> values = redis.mget(keys);
    watch.stop();

    EXPECT_EQ(iterations, keysFound.size());
    EXPECT_EQ(iterations, values.size());

    cout << format("Scan performance: {:3.1f} ms, {:3.2f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, remove)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    redis.set("remove-key1", 12345);
    redis.set("remove-key2", 1234);

    const auto removeCount = redis.remove({"remove-key1", "remove-key2", "remove-key3"});

    // Expect two keys removed
    ASSERT_EQ(2, removeCount);

    redis.disconnect();
}

} // namespace sptk
#endif
