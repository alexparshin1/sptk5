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

namespace {
const string RedisHost {"theater"};
}

class RedisConnectTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EXPECT_NO_THROW(redis.connect(RedisHost, 6379));
        ASSERT_TRUE(redis.isConnected());
    }

    void TearDown() override
    {
        if (redis.isConnected())
        {
            EXPECT_NO_THROW(redis.disconnect());
        }
    }

    RedisConnect redis;
};

namespace sptk {

TEST_F(RedisConnectTests, connectDisconnect)
{
}

TEST_F(RedisConnectTests, setGet)
{
    const string  key = "test_key";
    const Variant value = "test_value";

    EXPECT_NO_THROW(redis.setValue(key, value));
    EXPECT_EQ(redis.getValue(key).asString(), value.asString());
}

TEST_F(RedisConnectTests, mset)
{
    const RedisConnect::KeysAndValues testValues = {
        {"orange", 120},
        {"apple", 230},
        {"watermelon", 3500}};

    EXPECT_NO_THROW(redis.setValues(testValues));

    vector<string> keys;
    for (const auto& key: views::keys(testValues))
    {
        keys.push_back(key);
    }

    auto results = redis.getValues(keys);

    for (const auto& [key, value]: testValues)
    {
        EXPECT_EQ(value.asString(), results[key].asString());
    }
}

TEST_F(RedisConnectTests, setGetInt)
{
    const string  key = "int_key";
    const Variant value = static_cast<int64_t>(1234567890123LL);

    EXPECT_NO_THROW(redis.setValue(key, value));
    EXPECT_EQ(redis.getValue(key).asInt64(), value.asInt64());
}

TEST_F(RedisConnectTests, incr)
{
    const string  key = "int_key";
    const Variant value = static_cast<int64_t>(1234);

    redis.setValue(key, value);
    EXPECT_EQ(value.asInt64(), redis.getValue(key).asInt64());

    EXPECT_EQ(value.asInt64() + 1, redis.incrementKey(key));
}

TEST_F(RedisConnectTests, setGetBool)
{
    const string key_t = "bool_key_t";
    const string key_f = "bool_key_f";

    EXPECT_NO_THROW(redis.setValue(key_t, true));
    EXPECT_EQ(redis.getValue(key_t).asBool(), true);

    EXPECT_NO_THROW(redis.setValue(key_f, false));
    EXPECT_EQ(redis.getValue(key_f).asBool(), false);
}

TEST_F(RedisConnectTests, setGetDouble)
{
    const string   key = "double_key";
    constexpr auto value = 3.1415926535;

    EXPECT_NO_THROW(redis.setValue(key, value));
    // to_string(double) might have different precision than what Redis returns or how it's stored,
    // but we implemented it using to_string(value)
    EXPECT_EQ(redis.getValue(key).asFloat(), value);
}

TEST_F(RedisConnectTests, setGetBinary)
{
    const string key = "binary_key";
    const char   data[] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x04, 0x05};
    const Buffer binaryData(data, sizeof(data));

    EXPECT_NO_THROW(redis.setValue(key, binaryData));

    const auto retrievedData = redis.getValue(key).asBuffer();
    EXPECT_EQ(retrievedData, binaryData);
}

TEST_F(RedisConnectTests, getNonExistentKey)
{
    EXPECT_EQ(redis.getValue("non_existent_key").asString(), "");
}

TEST_F(RedisConnectTests, setOverwrites)
{
    const string key = "overwrite_key_different";
    redis.setValue(key, "value1");
    EXPECT_EQ(redis.getValue(key).asString(), "value1");

    redis.setValue(key, "value2");
    EXPECT_EQ(redis.getValue(key).asString(), "value2");
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

    Stopwatch watch;
    watch.start();
    for (auto i = 0; i < iterations; ++i)
    {
        auto key = format("key_{}", i);
        redis.setValue(key, sessionJson);
    }
    watch.stop();
    cout << format("Set performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
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

    redis.setValues(keysAndValues);

    stopwatch.stop();
    cout << format("Set performance: {:3.1f} ms, {:3.1f} K/s\n", stopwatch.milliseconds(), iterations / stopwatch.milliseconds());
}

TEST_F(RedisConnectTests, performanceMultipleThreads)
{
    constexpr auto iterations = 1000;
    constexpr auto threadCount = 32;

    Stopwatch watch;
    watch.start();

    vector<jthread> threads;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([threadIndex]
                             {
                                 RedisConnect redis;
                                 redis.connect(RedisHost, 6379);

                                 for (auto i = 0; i < iterations; ++i)
                                 {
                                     auto key = format("session_{}_{}", threadIndex, i);
                                     redis.setValue(key, sessionJson);
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
    DatabaseConnectionPool connectionPool("postgresql://gtest:test#123@" + RedisHost + "/xmq_test");
    auto                   db = connectionPool.getConnection();

    Query dropTable(db, "DROP TABLE IF EXISTS test_table");
    dropTable.exec();

    Query createTable(db, "CREATE TABLE test_table (key VARCHAR(40) PRIMARY KEY, value VARCHAR)");
    createTable.exec();

    constexpr auto iterations = 1000;
    constexpr auto threadCount = 32;

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
    redis.setValue("mget-key1", 12345);
    redis.setValue("mget-key2", 1234);

    auto keysAndValues = redis.getValues({"mget-key1", "mget-key2"});

    // Expect two keys matched
    ASSERT_EQ(2, keysAndValues.size());

    EXPECT_EQ(12345, keysAndValues["mget-key1"].asInteger());
    EXPECT_EQ(1234, keysAndValues["mget-key2"].asInteger());
}

TEST_F(RedisConnectTests, mgetPerformance)
{
    ASSERT_TRUE(redis.isConnected());

    constexpr auto iterations = 10000;
    vector<string> keys;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("mget-perf-key{}", i));
    }

    for (auto i = 0; i < iterations; ++i)
    {
        redis.setValue(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    auto keysAndValues = redis.getValues(keys);
    watch.stop();

    cout << format("Mget performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, scan)
{
    ASSERT_TRUE(redis.isConnected());

    redis.setValue("scan-key1", 12345);
    redis.setValue("scan-key2", 1234);

    auto    foundKeys = redis.scan("scan-key*", 2);
    Strings keys;
    keys.reserve(keys.size());
    for (const auto& key: foundKeys)
    {
        keys.push_back(key);
        COUT("[" << key << "]" << " " << key.size() << " bytes");
    }

    // Expect two keys matched
    ASSERT_EQ(2, keys.size());

    EXPECT_NE(-1, keys.indexOf("scan-key1"));
    EXPECT_NE(-1, keys.indexOf("scan-key2"));
}

TEST_F(RedisConnectTests, getPerformance)
{
    constexpr auto iterations = 10000;
    constexpr auto threadCount = 32;

    vector<string>            keys;
    SynchronizedQueue<string> keysQueue;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("s:client{}@node-{}", i, i % 10));
        keysQueue.push_back(keys.back());
    }

    for (auto i = 0; i < iterations; ++i)
    {
        redis.setValue(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    atomic_size_t foundKeyCount = 0;

    vector<jthread> threads;
    for (auto thread = 0; thread < threadCount; ++thread)
    {
        threads.emplace_back([&keysQueue, &foundKeyCount]
                             {
                                 RedisConnect threadRedis;
                                 threadRedis.connect("theater", 6379);
                                 string key;
                                 while (keysQueue.pop_front(key, 1ms))
                                 {
                                     if (const auto value = threadRedis.getValue(key);
                                         !value.isNull())
                                     {
                                         ++foundKeyCount;
                                     }
                                 }
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }
    threads.clear();

    watch.stop();

    EXPECT_EQ(iterations, foundKeyCount);

    cout << format("{} gets: {:3.1f} ms, {:3.1f} K/s\n", iterations, watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, hgetPerformance)
{
    constexpr auto iterations = 100000;
    constexpr auto threadCount = 32;
    constexpr auto hashCount = 10;

    vector<string> hashes;
    for (auto i = 0; i < hashCount; ++i)
    {
        hashes.push_back(format("n:node-{}", i));
    }

    vector<string>            keys;
    SynchronizedQueue<string> keysQueue;
    for (auto i = 0; i < iterations; ++i)
    {
        const auto& hash = hashes[i % hashCount];
        const auto  key = format("s:client{}", i);
        redis.setHashValue(hash, key, i);
        keysQueue.push_back(key);
    }

    Stopwatch watch;
    watch.start();
    atomic_size_t foundKeyCount = 0;

    vector<jthread> threads;
    for (auto thread = 0; thread < threadCount; ++thread)
    {
        threads.emplace_back([&hashes, &keysQueue, &foundKeyCount]
                             {
                                 RedisConnect threadRedis;
                                 threadRedis.connect("theater", 6379);
                                 string key;
                                 while (keysQueue.pop_front(key, 1ms))
                                 {
                                     for (const auto& hash: hashes)
                                     {
                                         if (auto value = threadRedis.getHashValue(hash, key);
                                             !value.isNull())
                                         {
                                             ++foundKeyCount;
                                             break;
                                         }
                                     }
                                 }
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }
    threads.clear();

    watch.stop();

    EXPECT_EQ(iterations, foundKeyCount);

    cout << format("{} gets: {:3.1f} ms, {:3.1f} K/s\n", iterations, watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, scanAndMgetPerformance)
{
    constexpr auto iterations = 10000;
    vector<string> keys;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("scan-perf-key{}", i));
    }

    for (auto i = 0; i < iterations; ++i)
    {
        redis.setValue(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    const auto keysFound = redis.scan("scan-perf-key*", iterations);
    const auto keysAndValues = redis.getValues(keys);
    watch.stop();

    EXPECT_EQ(iterations, keysFound.size());
    EXPECT_EQ(iterations, keysAndValues.size());

    cout << format("Scan performance: {:3.1f} ms, {:3.2f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, remove)
{
    redis.setValue("remove-key1", 12345);
    redis.setValue("remove-key2", 1234);

    const auto removeCount = redis.deleteKeys({"remove-key1", "remove-key2", "remove-key3"});

    // Expect two keys removed
    ASSERT_EQ(2, removeCount);
}

TEST_F(RedisConnectTests, hgetSetSingleKey)
{
    const string hash = "test_hash";
    const string key = "test";
    Variant      value(3.45678);

    (void) redis.deleteKeys({hash});

    EXPECT_NO_THROW(redis.setHashValue(hash, key, value));
    auto returnedValue = redis.getHashValue(hash, key);
    EXPECT_NEAR(value.asFloat(), returnedValue.asFloat(), 0.001);
}

TEST_F(RedisConnectTests, hset)
{
    const string                      hash = "test_hash";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    EXPECT_NO_THROW(redis.setHashValues(hash, testValues));
}

TEST_F(RedisConnectTests, hkeys)
{
    const string                      hash = "test_hash_keys";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"},
        {"field3", "value3"}};

    redis.setHashValues(hash, testValues);

    const auto keys = redis.getHashKeys(hash);

    set<string, less<>> keysFound;
    for (const auto& key: keys)
    {
        keysFound.insert(key);
    }

    ASSERT_EQ(3, keys.size());

    EXPECT_TRUE(keysFound.contains("field1"));
    EXPECT_TRUE(keysFound.contains("field2"));
    EXPECT_TRUE(keysFound.contains("field3"));
}

TEST_F(RedisConnectTests, hmget)
{
    const string                      hash = "test_hash_mget";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    redis.setHashValues(hash, testValues);

    auto keysAndValues = redis.getHashValues(hash, {"field1", "field2", "field3"});

    ASSERT_EQ(3, keysAndValues.size());

    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ(12345, keysAndValues["field2"].asInteger());
    EXPECT_NEAR(3.45678, keysAndValues["field3"].asFloat(), 0.00001);
}

TEST_F(RedisConnectTests, hmgetNonExistentField)
{
    const string                      hash = "test_hash_missing";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    redis.setHashValues(hash, testValues);

    auto keysAndValues = redis.getHashValues(hash, {"field1", "non_existent_field"});

    ASSERT_EQ(2, keysAndValues.size());

    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ("", keysAndValues["non_existent_field"].asString());
}

TEST_F(RedisConnectTests, hgetall)
{
    const string                      hash = "test_hash_hgetall";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    (void) redis.deleteKeys({hash});
    redis.setHashValues(hash, testValues);

    auto keysAndValues = redis.getHashValues(hash);

    ASSERT_EQ(3, keysAndValues.size());
    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ(12345, keysAndValues["field2"].asInteger());
    EXPECT_NEAR(3.45678, keysAndValues["field3"].asFloat(), 0.00001);

    (void) redis.deleteKeys({hash});
}

TEST_F(RedisConnectTests, hgetallEmpty)
{
    const string hash = "test_hash_hgetall_empty";

    (void) redis.deleteKeys({hash});

    const auto keysAndValues = redis.getHashValues(hash);
    EXPECT_TRUE(keysAndValues.empty());
}

TEST_F(RedisConnectTests, hdel)
{
    const string                      hash = "test_hash_hdel";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"},
        {"field3", "value3"}};

    (void) redis.deleteKeys({hash});
    redis.setHashValues(hash, testValues);

    EXPECT_NO_THROW(redis.deleteHashKeys(hash, {"field1", "field2"}));

    const auto remainingKeys = redis.getHashKeys(hash);
    ASSERT_EQ(1, remainingKeys.size());
    EXPECT_EQ("field3", remainingKeys[0]);

    auto keysAndValues = redis.getHashValues(hash, {"field1", "field2", "field3"});
    EXPECT_EQ("", keysAndValues["field1"].asString());
    EXPECT_EQ("", keysAndValues["field2"].asString());
    EXPECT_EQ("value3", keysAndValues["field3"].asString());

    (void) redis.deleteKeys({hash});
}

TEST_F(RedisConnectTests, hdelNonExistentField)
{
    const string                      hash = "test_hash_hdel_missing";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    (void) redis.deleteKeys({hash});
    redis.setHashValues(hash, testValues);

    EXPECT_NO_THROW(redis.deleteHashKeys(hash, {"field1", "non_existent_field"}));

    const auto remainingKeys = redis.getHashKeys(hash);
    ASSERT_EQ(1, remainingKeys.size());
    EXPECT_EQ("field2", remainingKeys[0]);

    (void) redis.deleteKeys({hash});
}

TEST_F(RedisConnectTests, hdelNonExistentHash)
{
    const string hash = "test_hash_hdel_no_hash";

    (void) redis.deleteKeys({hash});

    EXPECT_NO_THROW(redis.deleteHashKeys(hash, {"field1", "field2"}));
}

TEST_F(RedisConnectTests, hdelAllFields)
{
    const string                      hash = "test_hash_hdel_all";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    (void) redis.deleteKeys({hash});
    redis.setHashValues(hash, testValues);

    EXPECT_NO_THROW(redis.deleteHashKeys(hash, {"field1", "field2"}));

    const auto remainingKeys = redis.getHashKeys(hash);
    EXPECT_TRUE(remainingKeys.empty());
}

TEST_F(RedisConnectTests, setAddAndMembers)
{
    const string key = "set_test_add_members";
    (void) redis.deleteKeys({key});

    const size_t added = redis.addSetMembers(key, {"alpha", "beta", "gamma"});
    EXPECT_EQ(3u, added);

    const auto          members = redis.getSetMembers(key);
    set<string, less<>> membersFound(members.begin(), members.end());

    ASSERT_EQ(3u, membersFound.size());
    EXPECT_TRUE(membersFound.contains("alpha"));
    EXPECT_TRUE(membersFound.contains("beta"));
    EXPECT_TRUE(membersFound.contains("gamma"));

    (void) redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setAddDuplicates)
{
    const string key = "set_test_add_duplicates";
    (void) redis.deleteKeys({key});

    EXPECT_EQ(3u, redis.addSetMembers(key, {"a", "b", "c"}));
    // Adding existing + one new: only new one counts
    EXPECT_EQ(1u, redis.addSetMembers(key, {"a", "b", "d"}));

    const auto members = redis.getSetMembers(key);
    EXPECT_EQ(4u, members.size());

    (void) redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setAddEmptyMembers)
{
    EXPECT_EQ(0u, redis.addSetMembers("set_test_empty_add", {}));
}

TEST_F(RedisConnectTests, setIsMember)
{
    const string key = "set_test_ismember";
    (void) redis.deleteKeys({key});

    redis.addSetMembers(key, {"apple", "banana"});

    EXPECT_TRUE(redis.isSetMember(key, "apple"));
    EXPECT_TRUE(redis.isSetMember(key, "banana"));
    EXPECT_FALSE(redis.isSetMember(key, "cherry"));
    EXPECT_FALSE(redis.isSetMember(key, "nonexistent_set_key"));

    (void) redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setRemove)
{
    const string key = "set_test_remove";
    (void) redis.deleteKeys({key});

    redis.addSetMembers(key, {"x", "y", "z"});

    const size_t removed = redis.deleteSetMembers(key, {"x", "z"});
    EXPECT_EQ(2u, removed);

    const auto members = redis.getSetMembers(key);
    ASSERT_EQ(1u, members.size());
    EXPECT_EQ("y", members[0]);

    (void) redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setRemoveNonExistentMember)
{
    const string key = "set_test_remove_missing";
    (void) redis.deleteKeys({key});

    redis.addSetMembers(key, {"p", "q"});

    // Removing a mix of existing and non-existing members
    const size_t removed = redis.deleteSetMembers(key, {"p", "no_such_member"});
    EXPECT_EQ(1u, removed);

    const auto members = redis.getSetMembers(key);
    ASSERT_EQ(1u, members.size());
    EXPECT_EQ("q", members[0]);

    (void) redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setRemoveEmptyMembers)
{
    EXPECT_EQ(0u, redis.deleteSetMembers("set_test_empty_remove", {}));
}

TEST_F(RedisConnectTests, setMembersOnEmptySet)
{
    const string key = "set_test_members_empty";
    (void) redis.deleteKeys({key});

    const auto members = redis.getSetMembers(key);
    EXPECT_TRUE(members.empty());
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
                                         redis.setHashValues(hashName, testValues);
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
                                     redis.setHashValues(hashName, testValues);
                                 }

                                 redis.disconnect();
                             });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    stopwatch.stop();

    COUT(format("Set {} hashes of {} keys each ({} total keys) for {:3.1f}ms ({:3.1f}K/s)",
                maxThreads * maxHashes,
                maxKeysPerHash,
                maxThreads * maxHashes * maxKeysPerHash,
                stopwatch.milliseconds(),
                maxThreads * maxHashes * maxKeysPerHash / stopwatch.milliseconds()));
}

TEST_F(RedisConnectTests, nodesPerformance)
{
    constexpr size_t maxHashes = 100;
    constexpr size_t maxKeysPerHash = 100;

    for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
    {
        auto                        hashName = format("hash_mget_{}", hashIndex);
        RedisConnect::KeysAndValues testValues;
        for (size_t keyIndex = 0; keyIndex < maxKeysPerHash; ++keyIndex)
        {
            testValues[format("message_{}", keyIndex)] = subscriptionJson;
        }
        redis.setHashValues(hashName, testValues);
    }

    Stopwatch stopwatch;
    stopwatch.start();

    for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
    {
        auto hashName = format("hash_mget_{}", hashIndex);
        auto hashKeys = redis.getHashKeys(hashName);
        auto values = redis.getHashValues(hashName, hashKeys);
    }

    stopwatch.stop();

    COUT(format("Get {} hashes of {} keys each ({} total keys) for {:3.1f}ms ({:3.1f}K/s)",
                maxHashes, maxKeysPerHash, maxHashes * maxKeysPerHash,
                stopwatch.milliseconds(),
                maxHashes * maxKeysPerHash / stopwatch.milliseconds()));
}

TEST_F(RedisConnectTests, rename)
{
    const string oldKey = "rename_test_old";
    const string newKey = "rename_test_new";
    const string value = "test_value";

    // Set initial value
    redis.setValue(oldKey, Variant(value));
    EXPECT_EQ(value, redis.getValue(oldKey).asString());

    // Rename the key
    EXPECT_NO_THROW(redis.renameKey(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    EXPECT_EQ("", redis.getValue(oldKey).asString());
    EXPECT_EQ(value, redis.getValue(newKey).asString());

    // Clean up
    (void) redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, renameOverwrite)
{
    const string oldKey = "rename_overwrite_old";
    const string newKey = "rename_overwrite_new";
    const string value1 = "value1";
    const string value2 = "value2";

    // Set both keys
    redis.setValue(oldKey, Variant(value1));
    redis.setValue(newKey, Variant(value2));

    // Rename should overwrite newKey
    EXPECT_NO_THROW(redis.renameKey(oldKey, newKey));

    // Verify newKey has value1 (from oldKey)
    EXPECT_EQ(value1, redis.getValue(newKey).asString());

    // Clean up
    (void) redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, renameNonExistentKey)
{
    // Try to rename a non-existent key
    EXPECT_THROW(redis.renameKey("nonexistent_key_12345", "new_key_12345"), RedisConnectException);
}

TEST_F(RedisConnectTests, renameNX)
{
    const string oldKey = "renamenx_test_old";
    const string newKey = "renamenx_test_new";
    const string value = "test_value";

    // Clean up any previous test data
    (void) redis.deleteKeys({oldKey, newKey});

    // Set initial value
    redis.setValue(oldKey, Variant(value));

    // RenameNX should succeed when newKey doesn't exist
    EXPECT_TRUE(redis.renameKeyIfExists(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    EXPECT_EQ("", redis.getValue(oldKey).asString());
    EXPECT_EQ(value, redis.getValue(newKey).asString());

    // Clean up
    (void) redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, renameNXExistingKey)
{
    const string oldKey = "renamenx_existing_old";
    const string newKey = "renamenx_existing_new";
    const string value1 = "value1";
    const string value2 = "value2";

    // Set both keys
    redis.setValue(oldKey, Variant(value1));
    redis.setValue(newKey, Variant(value2));

    // RenameNX should fail when newKey already exists
    EXPECT_FALSE(redis.renameKeyIfExists(oldKey, newKey));

    // Verify both keys still have their original values
    EXPECT_EQ(value1, redis.getValue(oldKey).asString());
    EXPECT_EQ(value2, redis.getValue(newKey).asString());

    // Clean up
    (void) redis.deleteKeys({oldKey, newKey});

    redis.disconnect();
}

TEST_F(RedisConnectTests, renameHash)
{
    const string oldKey = "hash_old";
    const string newKey = "hash_new";
    const string value = "test_value";

    // Set initial value
    redis.setHashValues(oldKey, {{"akey", 1234}});
    auto keysAndValues = redis.getHashValues(oldKey, {"akey"});
    EXPECT_EQ(1234, keysAndValues["akey"].asInteger());

    // Rename the key
    EXPECT_NO_THROW(redis.renameKey(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    keysAndValues = redis.getHashValues(oldKey, {"akey"});
    EXPECT_EQ(0, keysAndValues["akey"].asInteger());

    keysAndValues = redis.getHashValues(newKey, {"akey"});
    EXPECT_EQ(1234, keysAndValues["akey"].asInteger());

    // Clean up
    (void) redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, transactionBasic)
{
    const string key1 = "txn_key1";
    const string key2 = "txn_key2";
    const string value1 = "value1";
    const string value2 = "value2";

    // Clean up any previous test data
    (void) redis.deleteKeys({key1, key2});

    // Begin transaction
    EXPECT_NO_THROW(redis.beginTransaction());

    // Queue commands
    redis.setValue(key1, Variant(value1));
    redis.setValue(key2, Variant(value2));

    // Commit transaction
    auto results = redis.commitTransaction();

    // Verify results - EXEC returns array of results
    EXPECT_FALSE(results.empty());

    // Verify values were set
    EXPECT_EQ(value1, redis.getValue(key1).asString());
    EXPECT_EQ(value2, redis.getValue(key2).asString());

    // Clean up
    (void) redis.deleteKeys({key1, key2});
}

TEST_F(RedisConnectTests, transactionRollback)
{
    const string key = "txn_rollback_key";
    const string initialValue = "initial";
    const string newValue = "new";

    // Set initial value
    redis.setValue(key, Variant(initialValue));
    EXPECT_EQ(initialValue, redis.getValue(key).asString());

    // Begin transaction
    EXPECT_NO_THROW(redis.beginTransaction());

    // Queue command
    redis.setValue(key, Variant(newValue));

    // Rollback transaction
    EXPECT_NO_THROW(redis.rollbackTransaction());

    // Verify value was NOT changed
    EXPECT_EQ(initialValue, redis.getValue(key).asString());

    // Clean up
    (void) redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, transactionMultipleOperations)
{
    const string key1 = "txn_multi_key1";
    const string key2 = "txn_multi_key2";
    const string key3 = "txn_multi_incr";

    // Clean up any previous test data
    (void) redis.deleteKeys({key1, key2, key3});

    // Begin transaction
    redis.beginTransaction();

    // Queue multiple different operations
    redis.setValue(key1, Variant("value1"));
    redis.setValue(key2, Variant(42));
    (void) redis.incrementKey(key3);
    (void) redis.incrementKey(key3);
    (void) redis.incrementKey(key3);

    // Commit transaction
    auto results = redis.commitTransaction();

    // Verify all operations executed
    EXPECT_EQ("value1", redis.getValue(key1).asString());
    EXPECT_EQ(42, redis.getValue(key2).asInteger());
    EXPECT_EQ(3, redis.getValue(key3).asInteger());

    // Clean up
    (void) redis.deleteKeys({key1, key2, key3});
}

TEST_F(RedisConnectTests, transactionWithHash)
{
    const string hashKey = "txn_hash";

    // Clean up any previous test data
    (void) redis.deleteKeys({hashKey});

    // Begin transaction
    redis.beginTransaction();

    // Queue hash operations
    redis.setHashValues(hashKey, {{"field1", 100}, {"field2", 200}});

    // Commit transaction
    auto results = redis.commitTransaction();

    // Verify hash was set
    auto values = redis.getHashValues(hashKey, {"field1", "field2"});
    EXPECT_EQ(100, values["field1"].asInteger());
    EXPECT_EQ(200, values["field2"].asInteger());

    // Clean up
    (void) redis.deleteKeys({hashKey});
}

TEST_F(RedisConnectTests, transactionCommitWithoutBegin)
{
    // Try to commit without beginning a transaction
    // Redis will return an error
    EXPECT_THROW((void) redis.commitTransaction(), RedisConnectException);
}

TEST_F(RedisConnectTests, transactionRollbackWithoutBegin)
{
    // Try to rollback without beginning a transaction
    // Redis will return an error
    EXPECT_THROW(redis.rollbackTransaction(), RedisConnectException);
}

TEST_F(RedisConnectTests, threadSafety)
{
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
    (void) redis.deleteKeys(keysToDelete);

    vector<jthread> threads;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([this, threadIndex]
                             {
                                 for (auto i = 0; i < operationsPerThread; ++i)
                                 {
                                     const auto key = format("threadsafe_key_{}_{}", threadIndex, i);
                                     const auto value = format("value_{}_{}", threadIndex, i);

                                     // Test set/get operations
                                     redis.setValue(key, Variant(value));
                                     const auto retrieved = redis.getValue(key).asString();
                                     EXPECT_EQ(value, retrieved);

                                     // Test incr operation
                                     const auto incrKey = format("threadsafe_incr_{}", threadIndex);
                                     (void) redis.incrementKey(incrKey);

                                     // Test hash operations
                                     const auto                        hashKey = format("threadsafe_hash_{}", threadIndex);
                                     const RedisConnect::KeysAndValues hashValues = {{format("field_{}", i), Variant(value)}};
                                     redis.setHashValues(hashKey, hashValues);
                                     const auto keys = redis.getHashKeys(hashKey);
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
        const auto value = redis.getValue(incrKey).asInt64();
        EXPECT_EQ(operationsPerThread, value);
    }
}

} // namespace sptk
