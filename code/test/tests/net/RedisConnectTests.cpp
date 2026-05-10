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
#include <ranges>
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

TEST_F(RedisConnectTests, mset)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const RedisConnect::KeysAndValues testValues = {
        {"orange", 120},
        {"apple", 230},
        {"watermelon", 3500}};

    EXPECT_NO_THROW(redis.mset(testValues));

    vector<string> keys;
    for (const auto& key: views::keys(testValues))
    {
        keys.push_back(key);
    }

    auto results = redis.mget(keys);

    for (const auto& [key, value]: testValues)
    {
        EXPECT_EQ(value.asString(), results[key].asString());
    }

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

TEST_F(RedisConnectTests, incr)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    const string  key = "int_key";
    const Variant value = static_cast<int64_t>(1234);

    redis.set(key, value);
    EXPECT_EQ(value.asInt64(), redis.get(key).asInt64());

    EXPECT_EQ(value.asInt64() + 1, redis.incr(key));

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

const String subscriptionJson = R"({"session_id":12345,"id":12345, topic:"devices/usb/12345","qos":1})";

} // namespace

TEST_F(RedisConnectTests, performanceSetSingleThread)
{
    constexpr auto iterations = 1000;

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

TEST_F(RedisConnectTests, performanceMSetSingleThread)
{
    constexpr auto iterations = 1000;

    RedisConnect redis;
    redis.connect("theater", 6379);

    Stopwatch stopwatch;
    stopwatch.start();
    RedisConnect::KeysAndValues keysAndValues;
    for (auto i = 0; i < iterations; ++i)
    {
        auto key = format("key_{}", i);
        keysAndValues[key] = sessionJson;
    }

    redis.mset(keysAndValues);

    stopwatch.stop();
    cout << format("Set performance: {:3.1f} ms, {:3.1f} K/s\n", stopwatch.milliseconds(), iterations / stopwatch.milliseconds());

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

    auto keysAndValues = redis.mget({"mget-key1", "mget-key2"});

    // Expect two keys matched
    ASSERT_EQ(2, keysAndValues.size());

    EXPECT_EQ(12345, keysAndValues["mget-key1"].asInteger());
    EXPECT_EQ(1234, keysAndValues["mget-key2"].asInteger());
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
    auto keysAndValues = redis.mget(keys);
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

    constexpr auto iterations = 100;
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
    watch.stop();

    EXPECT_EQ(iterations, keysFound.size());

    cout << format("Scan performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, scanAndMgetPerformance)
{
    RedisConnect redis;
    redis.connect("theater", 6379);

    ASSERT_TRUE(redis.isConnected());

    constexpr auto iterations = 10000;
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
    const auto            keysAndValues = redis.mget(keys);
    watch.stop();

    EXPECT_EQ(iterations, keysFound.size());
    EXPECT_EQ(iterations, keysAndValues.size());

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

TEST_F(RedisConnectTests, hsetSingleKey)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string hash = "test_hash";
    const string key = "test";
    Variant      value(3.45678);

    (void) redis.remove({hash});

    EXPECT_NO_THROW(redis.hset(hash, key, value));
    auto keysAndValues = redis.hmget(hash, {key});
    EXPECT_EQ(1, keysAndValues.size());
    EXPECT_NEAR(value.asFloat(), keysAndValues[key].asFloat(), 0.001);

    redis.disconnect();
}

TEST_F(RedisConnectTests, hset)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    EXPECT_NO_THROW(redis.hset(hash, testValues));

    redis.disconnect();
}

TEST_F(RedisConnectTests, hkeys)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash_keys";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"},
        {"field3", "value3"}};

    redis.hset(hash, testValues);

    const auto keys = redis.hkeys(hash);

    set<string> keysFound;
    for (const auto& key: keys)
    {
        keysFound.insert(key);
    }

    ASSERT_EQ(3, keys.size());

    EXPECT_TRUE(keysFound.contains("field1"));
    EXPECT_TRUE(keysFound.contains("field2"));
    EXPECT_TRUE(keysFound.contains("field3"));

    redis.disconnect();
}

TEST_F(RedisConnectTests, hmget)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash_mget";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    redis.hset(hash, testValues);

    auto keysAndValues = redis.hmget(hash, {"field1", "field2", "field3"});

    ASSERT_EQ(3, keysAndValues.size());

    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ(12345, keysAndValues["field2"].asInteger());
    EXPECT_NEAR(3.45678, keysAndValues["field3"].asFloat(), 0.00001);

    redis.disconnect();
}

TEST_F(RedisConnectTests, hmgetNonExistentField)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash_missing";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    redis.hset(hash, testValues);

    auto keysAndValues = redis.hmget(hash, {"field1", "non_existent_field"});

    ASSERT_EQ(2, keysAndValues.size());

    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ("", keysAndValues["non_existent_field"].asString());

    redis.disconnect();
}

TEST_F(RedisConnectTests, hgetall)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash_hgetall";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    (void) redis.remove({hash});
    redis.hset(hash, testValues);

    auto keysAndValues = redis.hgetall(hash);

    ASSERT_EQ(3, keysAndValues.size());
    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ(12345, keysAndValues["field2"].asInteger());
    EXPECT_NEAR(3.45678, keysAndValues["field3"].asFloat(), 0.00001);

    (void) redis.remove({hash});

    redis.disconnect();
}

TEST_F(RedisConnectTests, hgetallEmpty)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string hash = "test_hash_hgetall_empty";

    (void) redis.remove({hash});

    const auto keysAndValues = redis.hgetall(hash);
    EXPECT_TRUE(keysAndValues.empty());

    redis.disconnect();
}

TEST_F(RedisConnectTests, hdel)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash_hdel";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"},
        {"field3", "value3"}};

    (void) redis.remove({hash});
    redis.hset(hash, testValues);

    EXPECT_NO_THROW(redis.hdel(hash, {"field1", "field2"}));

    const auto remainingKeys = redis.hkeys(hash);
    ASSERT_EQ(1, remainingKeys.size());
    EXPECT_EQ("field3", remainingKeys[0]);

    auto keysAndValues = redis.hmget(hash, {"field1", "field2", "field3"});
    EXPECT_EQ("", keysAndValues["field1"].asString());
    EXPECT_EQ("", keysAndValues["field2"].asString());
    EXPECT_EQ("value3", keysAndValues["field3"].asString());

    (void) redis.remove({hash});

    redis.disconnect();
}

TEST_F(RedisConnectTests, hdelNonExistentField)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash_hdel_missing";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    (void) redis.remove({hash});
    redis.hset(hash, testValues);

    EXPECT_NO_THROW(redis.hdel(hash, {"field1", "non_existent_field"}));

    const auto remainingKeys = redis.hkeys(hash);
    ASSERT_EQ(1, remainingKeys.size());
    EXPECT_EQ("field2", remainingKeys[0]);

    (void) redis.remove({hash});

    redis.disconnect();
}

TEST_F(RedisConnectTests, hdelNonExistentHash)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string hash = "test_hash_hdel_no_hash";

    (void) redis.remove({hash});

    EXPECT_NO_THROW(redis.hdel(hash, {"field1", "field2"}));

    redis.disconnect();
}

TEST_F(RedisConnectTests, hdelAllFields)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string                      hash = "test_hash_hdel_all";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    (void) redis.remove({hash});
    redis.hset(hash, testValues);

    EXPECT_NO_THROW(redis.hdel(hash, {"field1", "field2"}));

    const auto remainingKeys = redis.hkeys(hash);
    EXPECT_TRUE(remainingKeys.empty());

    redis.disconnect();
}

TEST_F(RedisConnectTests, hsetPerformance)
{
    constexpr size_t maxHashes = 100;
    constexpr size_t maxKeysPerHash = 100;
    constexpr size_t maxThreads = 16;

    Stopwatch stopwatch;
    stopwatch.start();

    vector<jthread> threads;
    for (size_t i = 0; i < maxThreads; i++)
    {
        threads.emplace_back([threadNumber = i]
                             {
                                 RedisConnect redis;
                                 redis.connect("theater", 6379);

                                 for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
                                 {
                                     auto hashName = format("hash_{}_{}", threadNumber, hashIndex);
                                     for (size_t keyIndex = 0; keyIndex < maxKeysPerHash; ++keyIndex)
                                     {
                                         const RedisConnect::KeysAndValues testValues = {
                                             {format("message_{}", keyIndex), subscriptionJson},
                                         };
                                         redis.hset(hashName, testValues);
                                     }
                                 }

                                 redis.disconnect();
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    stopwatch.stop();

    COUT(format("Set {} hashes of {} keys each ({} total keys) for {:3.1f}ms ({:3.1f}K/s)", maxThreads * maxHashes, maxKeysPerHash, maxThreads * maxHashes * maxKeysPerHash, stopwatch.milliseconds(), maxThreads * maxHashes * maxKeysPerHash / stopwatch.milliseconds()));
}

TEST_F(RedisConnectTests, hsetGroupPerformance)
{
    constexpr size_t maxHashes = 100;
    constexpr size_t maxKeysPerHash = 100;
    constexpr size_t maxThreads = 4;

    Stopwatch stopwatch;
    stopwatch.start();

    vector<jthread> threads;
    for (size_t i = 0; i < maxThreads; i++)
    {
        threads.emplace_back([threadNumber = i]
                             {
                                 RedisConnect redis;
                                 redis.connect("theater", 6379);

                                 for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
                                 {
                                     auto                        hashName = format("hash_{}_{}", threadNumber, hashIndex);
                                     RedisConnect::KeysAndValues testValues;
                                     for (size_t keyIndex = 0; keyIndex < maxKeysPerHash; ++keyIndex)
                                     {
                                         testValues[format("message_{}", keyIndex)] = subscriptionJson;
                                     }
                                     redis.hset(hashName, testValues);
                                 }

                                 redis.disconnect();
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    stopwatch.stop();

    COUT(format("Set {} hashes of {} keys each ({} total keys) for {:3.1f}ms ({:3.1f}K/s", maxThreads * maxHashes, maxKeysPerHash, maxThreads * maxHashes * maxKeysPerHash, stopwatch.milliseconds(), maxThreads * maxHashes * maxKeysPerHash / stopwatch.milliseconds()));
}

TEST_F(RedisConnectTests, hmgetPerformance)
{
    constexpr size_t maxHashes = 100;
    constexpr size_t maxKeysPerHash = 100;

    RedisConnect redis;
    redis.connect("theater", 6379);

    for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
    {
        auto                        hashName = format("hash_mget_{}", hashIndex);
        RedisConnect::KeysAndValues testValues;
        for (size_t keyIndex = 0; keyIndex < maxKeysPerHash; ++keyIndex)
        {
            testValues[format("message_{}", keyIndex)] = subscriptionJson;
        }
        redis.hset(hashName, testValues);
    }

    Stopwatch stopwatch;
    stopwatch.start();

    for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
    {
        auto hashName = format("hash_mget_{}", hashIndex);
        auto hashKeys = redis.hkeys(hashName);
        auto values = redis.hmget(hashName, hashKeys);
    }

    stopwatch.stop();

    COUT(format("Get {} hashes of {} keys each ({} total keys) for {:3.1f}ms ({:3.1f}K/s", maxHashes, maxKeysPerHash, maxHashes * maxKeysPerHash, stopwatch.milliseconds(), maxHashes * maxKeysPerHash / stopwatch.milliseconds()));

    redis.disconnect();
}

TEST_F(RedisConnectTests, rename)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string oldKey = "rename_test_old";
    const string newKey = "rename_test_new";
    const string value = "test_value";

    // Set initial value
    redis.set(oldKey, Variant(value));
    EXPECT_EQ(value, redis.get(oldKey).asString());

    // Rename the key
    EXPECT_NO_THROW(redis.rename(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    EXPECT_EQ("", redis.get(oldKey).asString());
    EXPECT_EQ(value, redis.get(newKey).asString());

    // Clean up
    (void) redis.remove({newKey});

    redis.disconnect();
}

TEST_F(RedisConnectTests, renameOverwrite)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string oldKey = "rename_overwrite_old";
    const string newKey = "rename_overwrite_new";
    const string value1 = "value1";
    const string value2 = "value2";

    // Set both keys
    redis.set(oldKey, Variant(value1));
    redis.set(newKey, Variant(value2));

    // Rename should overwrite newKey
    EXPECT_NO_THROW(redis.rename(oldKey, newKey));

    // Verify newKey has value1 (from oldKey)
    EXPECT_EQ(value1, redis.get(newKey).asString());

    // Clean up
    (void) redis.remove({newKey});

    redis.disconnect();
}

TEST_F(RedisConnectTests, renameNonExistentKey)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    // Try to rename a non-existent key
    EXPECT_THROW(redis.rename("nonexistent_key_12345", "new_key_12345"), RedisConnectException);

    redis.disconnect();
}

TEST_F(RedisConnectTests, renameNX)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string oldKey = "renamenx_test_old";
    const string newKey = "renamenx_test_new";
    const string value = "test_value";

    // Clean up any previous test data
    (void) redis.remove({oldKey, newKey});

    // Set initial value
    redis.set(oldKey, Variant(value));

    // RenameNX should succeed when newKey doesn't exist
    EXPECT_TRUE(redis.renameNX(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    EXPECT_EQ("", redis.get(oldKey).asString());
    EXPECT_EQ(value, redis.get(newKey).asString());

    // Clean up
    (void) redis.remove({newKey});

    redis.disconnect();
}

TEST_F(RedisConnectTests, renameNXExistingKey)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string oldKey = "renamenx_existing_old";
    const string newKey = "renamenx_existing_new";
    const string value1 = "value1";
    const string value2 = "value2";

    // Set both keys
    redis.set(oldKey, Variant(value1));
    redis.set(newKey, Variant(value2));

    // RenameNX should fail when newKey already exists
    EXPECT_FALSE(redis.renameNX(oldKey, newKey));

    // Verify both keys still have their original values
    EXPECT_EQ(value1, redis.get(oldKey).asString());
    EXPECT_EQ(value2, redis.get(newKey).asString());

    // Clean up
    (void) redis.remove({oldKey, newKey});

    redis.disconnect();
}

TEST_F(RedisConnectTests, renameHash)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string oldKey = "hash_old";
    const string newKey = "hash_new";
    const string value = "test_value";

    // Set initial value
    redis.hset(oldKey, {{"akey", 1234}});
    auto keysAndValues = redis.hmget(oldKey, {"akey"});
    EXPECT_EQ(1234, keysAndValues["akey"].asInteger());

    // Rename the key
    EXPECT_NO_THROW(redis.rename(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    keysAndValues = redis.hmget(oldKey, {"akey"});
    EXPECT_EQ(0, keysAndValues["akey"].asInteger());

    keysAndValues = redis.hmget(newKey, {"akey"});
    EXPECT_EQ(1234, keysAndValues["akey"].asInteger());

    // Clean up
    (void) redis.remove({newKey});

    redis.disconnect();
}

TEST_F(RedisConnectTests, transactionBasic)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string key1 = "txn_key1";
    const string key2 = "txn_key2";
    const string value1 = "value1";
    const string value2 = "value2";

    // Clean up any previous test data
    (void) redis.remove({key1, key2});

    // Begin transaction
    EXPECT_NO_THROW(redis.beginTransaction());

    // Queue commands
    redis.set(key1, Variant(value1));
    redis.set(key2, Variant(value2));

    // Commit transaction
    auto results = redis.commitTransaction();

    // Verify results - EXEC returns array of results
    EXPECT_FALSE(results.empty());

    // Verify values were set
    EXPECT_EQ(value1, redis.get(key1).asString());
    EXPECT_EQ(value2, redis.get(key2).asString());

    // Clean up
    (void) redis.remove({key1, key2});

    redis.disconnect();
}

TEST_F(RedisConnectTests, transactionRollback)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string key = "txn_rollback_key";
    const string initialValue = "initial";
    const string newValue = "new";

    // Set initial value
    redis.set(key, Variant(initialValue));
    EXPECT_EQ(initialValue, redis.get(key).asString());

    // Begin transaction
    EXPECT_NO_THROW(redis.beginTransaction());

    // Queue command
    redis.set(key, Variant(newValue));

    // Rollback transaction
    EXPECT_NO_THROW(redis.rollbackTransaction());

    // Verify value was NOT changed
    EXPECT_EQ(initialValue, redis.get(key).asString());

    // Clean up
    (void) redis.remove({key});

    redis.disconnect();
}

TEST_F(RedisConnectTests, transactionMultipleOperations)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string key1 = "txn_multi_key1";
    const string key2 = "txn_multi_key2";
    const string key3 = "txn_multi_incr";

    // Clean up any previous test data
    (void) redis.remove({key1, key2, key3});

    // Begin transaction
    redis.beginTransaction();

    // Queue multiple different operations
    redis.set(key1, Variant("value1"));
    redis.set(key2, Variant(42));
    (void) redis.incr(key3);
    (void) redis.incr(key3);
    (void) redis.incr(key3);

    // Commit transaction
    auto results = redis.commitTransaction();

    // Verify all operations executed
    EXPECT_EQ("value1", redis.get(key1).asString());
    EXPECT_EQ(42, redis.get(key2).asInteger());
    EXPECT_EQ(3, redis.get(key3).asInteger());

    // Clean up
    (void) redis.remove({key1, key2, key3});

    redis.disconnect();
}

TEST_F(RedisConnectTests, transactionWithHash)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    const string hashKey = "txn_hash";

    // Clean up any previous test data
    (void) redis.remove({hashKey});

    // Begin transaction
    redis.beginTransaction();

    // Queue hash operations
    redis.hset(hashKey, {{"field1", 100}, {"field2", 200}});

    // Commit transaction
    auto results = redis.commitTransaction();

    // Verify hash was set
    auto values = redis.hmget(hashKey, {"field1", "field2"});
    EXPECT_EQ(100, values["field1"].asInteger());
    EXPECT_EQ(200, values["field2"].asInteger());

    // Clean up
    (void) redis.remove({hashKey});

    redis.disconnect();
}

TEST_F(RedisConnectTests, transactionCommitWithoutBegin)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    // Try to commit without beginning a transaction
    // Redis will return an error
    EXPECT_THROW((void) redis.commitTransaction(), RedisConnectException);

    redis.disconnect();
}

TEST_F(RedisConnectTests, transactionRollbackWithoutBegin)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    // Try to rollback without beginning a transaction
    // Redis will return an error
    EXPECT_THROW(redis.rollbackTransaction(), RedisConnectException);

    redis.disconnect();
}

TEST_F(RedisConnectTests, threadSafety)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    constexpr auto threadCount = 10;
    constexpr auto operationsPerThread = 100;

    // Clean up keys from previous test runs
    vector<string> keysToDelete;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        keysToDelete.push_back(format("threadsafe_incr_{}", threadIndex));
        keysToDelete.push_back(format("threadsafe_hash_{}", threadIndex));
        for (auto i = 0; i < operationsPerThread; ++i)
        {
            keysToDelete.push_back(format("threadsafe_key_{}_{}", threadIndex, i));
        }
    }
    (void) redis.remove(keysToDelete);

    vector<jthread> threads;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([&redis, threadIndex]
                             {
                                 for (auto i = 0; i < operationsPerThread; ++i)
                                 {
                                     const auto key = format("threadsafe_key_{}_{}", threadIndex, i);
                                     const auto value = format("value_{}_{}", threadIndex, i);

                                     // Test set/get operations
                                     redis.set(key, Variant(value));
                                     const auto retrieved = redis.get(key).asString();
                                     EXPECT_EQ(value, retrieved);

                                     // Test incr operation
                                     const auto incrKey = format("threadsafe_incr_{}", threadIndex);
                                     (void) redis.incr(incrKey);

                                     // Test hash operations
                                     const auto                        hashKey = format("threadsafe_hash_{}", threadIndex);
                                     const RedisConnect::KeysAndValues hashValues = {{format("field_{}", i), Variant(value)}};
                                     redis.hset(hashKey, hashValues);
                                     const auto keys = redis.hkeys(hashKey);
                                     EXPECT_FALSE(keys.empty());
                                 }
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    // Verify incr results
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        const auto incrKey = format("threadsafe_incr_{}", threadIndex);
        const auto value = redis.get(incrKey).asInt64();
        EXPECT_EQ(operationsPerThread, value);
    }

    redis.disconnect();
}

} // namespace sptk
#endif
