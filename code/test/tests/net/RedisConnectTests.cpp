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
#include <atomic>
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <ranges>
#include <thread>

using namespace sptk;
using namespace std;

namespace {
const string RedisHost {"theater"};

/**
 * @brief Everything these tests create is named with this prefix.
 *
 * The database is shared - other projects run their tests against the same server - so what is
 * swept up afterwards has to be exactly what was put there, and nothing that merely looks like it.
 */
const string TestKeyPattern {"test:*"};

/// How many keys a single sweep asks for. Scanning stops as soon as it has this many, so a small
/// number keeps each pass cheap and simply costs more passes after a test that made thousands.
constexpr size_t SweepLimit {1000};
} // namespace

class RedisConnectTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EXPECT_NO_THROW(m_redis.connect(RedisHost, 6379));
        ASSERT_TRUE(m_redis.isConnected());
    }

    void TearDown() override
    {
        if (m_redis.isConnected())
        {
            removeTestKeys();
            EXPECT_NO_THROW(m_redis.disconnect());
        }
    }

    /**
     * @brief Remove what this test left in the database.
     *
     * After every test rather than at the end of the suite: the performance tests write tens of
     * thousands of keys, and a database carrying them slows down everything else using the same
     * server - including other projects' tests, where it looks like a regression of their own
     * rather than leftovers from these.
     */
    void removeTestKeys()
    {
        try
        {
            for (auto keys = m_redis.scan(TestKeyPattern, SweepLimit); !keys.empty();
                 keys = m_redis.scan(TestKeyPattern, SweepLimit))
            {
                (void) m_redis.deleteKeys(keys);
            }
        }
        catch (const Exception& e)
        {
            // Reported rather than swallowed: keys left behind are the thing this exists to
            // prevent, and a sweep that quietly does nothing is how they come back.
            ADD_FAILURE() << "Could not remove the keys this test created: " << e.what();
        }
    }

    RedisConnect m_redis;
};

namespace sptk {

TEST_F(RedisConnectTests, connectDisconnect)
{
    // Just test connect and disconnect implemented in the test suite.
}

TEST_F(RedisConnectTests, setGet)
{
    const string  key = "test:key";
    const Variant value = "test_value";

    EXPECT_NO_THROW(m_redis.setValue(key, value));
    EXPECT_EQ(m_redis.getValue(key).asString(), value.asString());
}

TEST_F(RedisConnectTests, mset)
{
    const RedisConnect::KeysAndValues testValues = {
        {"test:orange", 120},
        {"test:apple", 230},
        {"test:watermelon", 3500}};

    EXPECT_NO_THROW(m_redis.setValues(testValues));

    vector<string> keys;
    for (const auto& key: views::keys(testValues))
    {
        keys.push_back(key);
    }

    auto results = m_redis.getValues(keys);

    for (const auto& [key, value]: testValues)
    {
        EXPECT_EQ(value.asString(), results[key].asString());
    }
}

TEST_F(RedisConnectTests, setGetInt)
{
    const string  key = "test:int_key";
    const Variant value = static_cast<int64_t>(1234567890123LL);

    EXPECT_NO_THROW(m_redis.setValue(key, value));
    EXPECT_EQ(m_redis.getValue(key).asInt64(), value.asInt64());
}

TEST_F(RedisConnectTests, incr)
{
    const string  key = "test:int_key";
    const Variant value = static_cast<int64_t>(1234);

    m_redis.setValue(key, value);
    EXPECT_EQ(value.asInt64(), m_redis.getValue(key).asInt64());

    EXPECT_EQ(value.asInt64() + 1, m_redis.incrementKey(key));
}

TEST_F(RedisConnectTests, setGetBool)
{
    const string key_t = "test:bool_key_t";
    const string key_f = "test:bool_key_f";

    EXPECT_NO_THROW(m_redis.setValue(key_t, true));
    EXPECT_EQ(m_redis.getValue(key_t).asBool(), true);

    EXPECT_NO_THROW(m_redis.setValue(key_f, false));
    EXPECT_EQ(m_redis.getValue(key_f).asBool(), false);
}

TEST_F(RedisConnectTests, setGetDouble)
{
    const string   key = "test:double_key";
    constexpr auto value = 3.1415926535;

    EXPECT_NO_THROW(m_redis.setValue(key, value));
    // to_string(double) might have different precision than what Redis returns or how it's stored,
    // but we implemented it using to_string(value)
    EXPECT_EQ(m_redis.getValue(key).asFloat(), value);
}

TEST_F(RedisConnectTests, setGetBinary)
{
    const string key = "test:binary_key";
    const char   data[] = {0x00, 0x01, 0x02, 0x03, 0x00, 0x04, 0x05};
    const Buffer binaryData(data, sizeof(data));

    EXPECT_NO_THROW(m_redis.setValue(key, binaryData));

    const auto retrievedData = m_redis.getValue(key).asBuffer();
    EXPECT_EQ(retrievedData, binaryData);
}

TEST_F(RedisConnectTests, getNonExistentKey)
{
    EXPECT_EQ(m_redis.getValue("test:non_existent_key").asString(), "");
}

TEST_F(RedisConnectTests, setOverwrites)
{
    const string key = "test:overwrite_key_different";
    m_redis.setValue(key, "value1");
    EXPECT_EQ(m_redis.getValue(key).asString(), "value1");

    m_redis.setValue(key, "value2");
    EXPECT_EQ(m_redis.getValue(key).asString(), "value2");
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
        auto key = format("test:key_{}", i);
        m_redis.setValue(key, sessionJson);
    }
    watch.stop();
    cout << format("Set performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, performanceMSetSingleThread)
{
    constexpr auto iterations = 1000;

    Stopwatch stopwatch;
    stopwatch.start();
    RedisConnect::KeysAndValues keysAndValues;
    for (auto i = 0; i < iterations; ++i)
    {
        auto key = format("test:key_{}", i);
        keysAndValues[key] = sessionJson;
    }

    m_redis.setValues(keysAndValues);

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
                                 RedisConnect threadRedis;
                                 threadRedis.connect(RedisHost, 6379);

                                 for (auto i = 0; i < iterations; ++i)
                                 {
                                     auto key = format("test:session_{}_{}", threadIndex, i);
                                     threadRedis.setValue(key, sessionJson);
                                 }

                                 threadRedis.disconnect();
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
                                     const auto key = format("test:session_{}_{}", threadIndex, i);
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
    m_redis.setValue("test:mget-key1", 12345);
    m_redis.setValue("test:mget-key2", 1234);

    auto keysAndValues = m_redis.getValues({"test:mget-key1", "test:mget-key2"});

    // Expect two keys matched
    ASSERT_EQ(2, keysAndValues.size());

    EXPECT_EQ(12345, keysAndValues["test:mget-key1"].asInteger());
    EXPECT_EQ(1234, keysAndValues["test:mget-key2"].asInteger());
}

TEST_F(RedisConnectTests, mgetPerformance)
{
    ASSERT_TRUE(m_redis.isConnected());

    constexpr auto iterations = 1000;
    vector<string> keys;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("test:mget-perf-key{}", i));
    }

    for (auto i = 0; i < iterations; ++i)
    {
        m_redis.setValue(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    auto keysAndValues = m_redis.getValues(keys);
    watch.stop();

    cout << format("Mget performance: {:3.1f} ms, {:3.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, scan)
{
    ASSERT_TRUE(m_redis.isConnected());

    m_redis.setValue("test:scan-key1", 12345);
    m_redis.setValue("test:scan-key2", 1234);

    auto    foundKeys = m_redis.scan("test:scan-key*", 2);
    Strings keys;
    keys.reserve(keys.size());
    for (const auto& key: foundKeys)
    {
        keys.push_back(key);
        COUT("[" << key << "]" << " " << key.size() << " bytes");
    }

    // Expect two keys matched
    ASSERT_EQ(2, keys.size());

    EXPECT_NE(-1, keys.indexOf("test:scan-key1"));
    EXPECT_NE(-1, keys.indexOf("test:scan-key2"));
}

TEST_F(RedisConnectTests, getPerformance)
{
    constexpr auto iterations = 1000;
    constexpr auto threadCount = 32;

    vector<string>            keys;
    SynchronizedQueue<string> keysQueue;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("test:s:client{}@node-{}", i, i % 10));
        keysQueue.push_back(keys.back());
    }

    for (auto i = 0; i < iterations; ++i)
    {
        m_redis.setValue(keys[i], i);
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
    constexpr auto iterations = 1000;
    constexpr auto threadCount = 32;
    constexpr auto hashCount = 10;

    vector<string> hashes;
    for (auto i = 0; i < hashCount; ++i)
    {
        hashes.push_back(format("test:n:node-{}", i));
    }

    vector<string>            keys;
    SynchronizedQueue<string> keysQueue;
    for (auto i = 0; i < iterations; ++i)
    {
        const auto& hash = hashes[i % hashCount];
        const auto  key = format("test:s:client{}", i);
        m_redis.setHashValue(hash, key, i);
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
    constexpr auto iterations = 1000;
    vector<string> keys;
    for (auto i = 0; i < iterations; ++i)
    {
        keys.push_back(format("test:scan-perf-key{}", i));
    }

    for (auto i = 0; i < iterations; ++i)
    {
        m_redis.setValue(keys[i], i);
    }

    Stopwatch watch;
    watch.start();
    const auto keysFound = m_redis.scan("test:scan-perf-key*", iterations);
    const auto keysAndValues = m_redis.getValues(keys);
    watch.stop();

    EXPECT_EQ(iterations, keysFound.size());
    EXPECT_EQ(iterations, keysAndValues.size());

    cout << format("Scan performance: {:3.1f} ms, {:3.2f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());
}

TEST_F(RedisConnectTests, remove)
{
    m_redis.setValue("test:remove-key1", 12345);
    m_redis.setValue("test:remove-key2", 1234);

    const auto removeCount = m_redis.deleteKeys({"test:remove-key1", "test:remove-key2", "test:remove-key3"});

    // Expect two keys removed
    ASSERT_EQ(2, removeCount);
}

TEST_F(RedisConnectTests, hgetSetSingleKey)
{
    const string hash = "test:hash";
    const string key = "test";
    Variant      value(3.45678);

    (void) m_redis.deleteKeys({hash});

    EXPECT_NO_THROW(m_redis.setHashValue(hash, key, value));
    auto returnedValue = m_redis.getHashValue(hash, key);
    EXPECT_NEAR(value.asFloat(), returnedValue.asFloat(), 0.001);
}

TEST_F(RedisConnectTests, hset)
{
    const string                      hash = "test:hash";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    EXPECT_NO_THROW(m_redis.setHashValues(hash, testValues));
}

TEST_F(RedisConnectTests, hkeys)
{
    const string                      hash = "test:hash_keys";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"},
        {"field3", "value3"}};

    m_redis.setHashValues(hash, testValues);

    const auto keys = m_redis.getHashKeys(hash);

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
    const string                      hash = "test:hash_mget";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    m_redis.setHashValues(hash, testValues);

    auto keysAndValues = m_redis.getHashValues(hash, {"field1", "field2", "field3"});

    ASSERT_EQ(3, keysAndValues.size());

    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ(12345, keysAndValues["field2"].asInteger());
    EXPECT_NEAR(3.45678, keysAndValues["field3"].asFloat(), 0.00001);
}

TEST_F(RedisConnectTests, hmgetNonExistentField)
{
    const string                      hash = "test:hash_missing";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    m_redis.setHashValues(hash, testValues);

    auto keysAndValues = m_redis.getHashValues(hash, {"field1", "non_existent_field"});

    ASSERT_EQ(2, keysAndValues.size());

    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ("", keysAndValues["non_existent_field"].asString());
}

TEST_F(RedisConnectTests, hgetall)
{
    const string                      hash = "test:hash_hgetall";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345},
        {"field3", 3.45678}};

    (void) m_redis.deleteKeys({hash});
    m_redis.setHashValues(hash, testValues);

    auto keysAndValues = m_redis.getHashValues(hash);

    ASSERT_EQ(3, keysAndValues.size());
    EXPECT_EQ("value1", keysAndValues["field1"].asString());
    EXPECT_EQ(12345, keysAndValues["field2"].asInteger());
    EXPECT_NEAR(3.45678, keysAndValues["field3"].asFloat(), 0.00001);

    (void) m_redis.deleteKeys({hash});
}

TEST_F(RedisConnectTests, hgetallEmpty)
{
    const string hash = "test:hash_hgetall_empty";

    (void) m_redis.deleteKeys({hash});

    const auto keysAndValues = m_redis.getHashValues(hash);
    EXPECT_TRUE(keysAndValues.empty());
}

TEST_F(RedisConnectTests, hdel)
{
    const string                      hash = "test:hash_hdel";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"},
        {"field3", "value3"}};

    (void) m_redis.deleteKeys({hash});
    m_redis.setHashValues(hash, testValues);

    EXPECT_NO_THROW(m_redis.deleteHashKeys(hash, {"field1", "field2"}));

    const auto remainingKeys = m_redis.getHashKeys(hash);
    ASSERT_EQ(1, remainingKeys.size());
    EXPECT_EQ("field3", remainingKeys[0]);

    auto keysAndValues = m_redis.getHashValues(hash, {"field1", "field2", "field3"});
    EXPECT_EQ("", keysAndValues["field1"].asString());
    EXPECT_EQ("", keysAndValues["field2"].asString());
    EXPECT_EQ("value3", keysAndValues["field3"].asString());

    (void) m_redis.deleteKeys({hash});
}

TEST_F(RedisConnectTests, hdelNonExistentField)
{
    const string                      hash = "test:hash_hdel_missing";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    (void) m_redis.deleteKeys({hash});
    m_redis.setHashValues(hash, testValues);

    EXPECT_NO_THROW(m_redis.deleteHashKeys(hash, {"field1", "non_existent_field"}));

    const auto remainingKeys = m_redis.getHashKeys(hash);
    ASSERT_EQ(1, remainingKeys.size());
    EXPECT_EQ("field2", remainingKeys[0]);

    (void) m_redis.deleteKeys({hash});
}

TEST_F(RedisConnectTests, hdelNonExistentHash)
{
    const string hash = "test:hash_hdel_no_hash";

    (void) m_redis.deleteKeys({hash});

    EXPECT_NO_THROW(m_redis.deleteHashKeys(hash, {"field1", "field2"}));
}

TEST_F(RedisConnectTests, hdelAllFields)
{
    const string                      hash = "test:hash_hdel_all";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", "value2"}};

    (void) m_redis.deleteKeys({hash});
    m_redis.setHashValues(hash, testValues);

    EXPECT_NO_THROW(m_redis.deleteHashKeys(hash, {"field1", "field2"}));

    const auto remainingKeys = m_redis.getHashKeys(hash);
    EXPECT_TRUE(remainingKeys.empty());
}

TEST_F(RedisConnectTests, setAddAndMembers)
{
    const string key = "test:set_test_add_members";
    (void) m_redis.deleteKeys({key});

    const size_t added = m_redis.addSetMembers(key, {"alpha", "beta", "gamma"});
    EXPECT_EQ(3u, added);

    const auto          members = m_redis.getSetMembers(key);
    set<string, less<>> membersFound(members.begin(), members.end());

    ASSERT_EQ(3u, membersFound.size());
    EXPECT_TRUE(membersFound.contains("alpha"));
    EXPECT_TRUE(membersFound.contains("beta"));
    EXPECT_TRUE(membersFound.contains("gamma"));

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setAddDuplicates)
{
    const string key = "test:set_test_add_duplicates";
    (void) m_redis.deleteKeys({key});

    EXPECT_EQ(3u, m_redis.addSetMembers(key, {"a", "b", "c"}));
    // Adding existing + one new: only new one counts
    EXPECT_EQ(1u, m_redis.addSetMembers(key, {"a", "b", "d"}));

    const auto members = m_redis.getSetMembers(key);
    EXPECT_EQ(4u, members.size());

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setAddEmptyMembers)
{
    EXPECT_EQ(0u, m_redis.addSetMembers("test:set_test_empty_add", {}));
}

TEST_F(RedisConnectTests, setIsMember)
{
    const string key = "test:set_test_ismember";
    (void) m_redis.deleteKeys({key});

    m_redis.addSetMembers(key, {"apple", "banana"});

    EXPECT_TRUE(m_redis.isSetMember(key, "apple"));
    EXPECT_TRUE(m_redis.isSetMember(key, "banana"));
    EXPECT_FALSE(m_redis.isSetMember(key, "cherry"));
    EXPECT_FALSE(m_redis.isSetMember(key, "test:nonexistent_set_key"));

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setRemove)
{
    const string key = "test:set_test_remove";
    (void) m_redis.deleteKeys({key});

    m_redis.addSetMembers(key, {"x", "y", "z"});

    const size_t removed = m_redis.deleteSetMembers(key, {"x", "z"});
    EXPECT_EQ(2u, removed);

    const auto members = m_redis.getSetMembers(key);
    ASSERT_EQ(1u, members.size());
    EXPECT_EQ("y", members[0]);

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setRemoveNonExistentMember)
{
    const string key = "test:set_test_remove_missing";
    (void) m_redis.deleteKeys({key});

    m_redis.addSetMembers(key, {"p", "q"});

    // Removing a mix of existing and non-existing members
    const size_t removed = m_redis.deleteSetMembers(key, {"p", "no_such_member"});
    EXPECT_EQ(1u, removed);

    const auto members = m_redis.getSetMembers(key);
    ASSERT_EQ(1u, members.size());
    EXPECT_EQ("q", members[0]);

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, setRemoveEmptyMembers)
{
    EXPECT_EQ(0u, m_redis.deleteSetMembers("test:set_test_empty_remove", {}));
}

TEST_F(RedisConnectTests, setMembersOnEmptySet)
{
    const string key = "test:set_test_members_empty";
    (void) m_redis.deleteKeys({key});

    const auto members = m_redis.getSetMembers(key);
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
                                 RedisConnect threadRedis;
                                 threadRedis.connect("theater", 6379);

                                 for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
                                 {
                                     auto hashName = format("test:hash_{}_{}", threadNumber, hashIndex);
                                     for (size_t keyIndex = 0; keyIndex < maxKeysPerHash; ++keyIndex)
                                     {
                                         const RedisConnect::KeysAndValues testValues = {
                                             {format("message_{}", keyIndex), subscriptionJson},
                                         };
                                         threadRedis.setHashValues(hashName, testValues);
                                     }
                                 }

                                 threadRedis.disconnect();
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
                                 RedisConnect threadRedis;
                                 threadRedis.connect("theater", 6379);

                                 for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
                                 {
                                     auto                        hashName = format("test:hash_{}_{}", threadNumber, hashIndex);
                                     RedisConnect::KeysAndValues testValues;
                                     for (size_t keyIndex = 0; keyIndex < maxKeysPerHash; ++keyIndex)
                                     {
                                         testValues[format("message_{}", keyIndex)] = subscriptionJson;
                                     }
                                     threadRedis.setHashValues(hashName, testValues);
                                 }

                                 threadRedis.disconnect();
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
        auto                        hashName = format("test:hash_mget_{}", hashIndex);
        RedisConnect::KeysAndValues testValues;
        for (size_t keyIndex = 0; keyIndex < maxKeysPerHash; ++keyIndex)
        {
            testValues[format("message_{}", keyIndex)] = subscriptionJson;
        }
        m_redis.setHashValues(hashName, testValues);
    }

    Stopwatch stopwatch;
    stopwatch.start();

    for (size_t hashIndex = 0; hashIndex < maxHashes; ++hashIndex)
    {
        auto hashName = format("test:hash_mget_{}", hashIndex);
        auto hashKeys = m_redis.getHashKeys(hashName);
        auto values = m_redis.getHashValues(hashName, hashKeys);
    }

    stopwatch.stop();

    COUT(format("Get {} hashes of {} keys each ({} total keys) for {:3.1f}ms ({:3.1f}K/s)",
                maxHashes, maxKeysPerHash, maxHashes * maxKeysPerHash,
                stopwatch.milliseconds(),
                maxHashes * maxKeysPerHash / stopwatch.milliseconds()));
}

TEST_F(RedisConnectTests, rename)
{
    const string oldKey = "test:rename_test_old";
    const string newKey = "test:rename_test_new";
    const string value = "test_value";

    // Set initial value
    m_redis.setValue(oldKey, Variant(value));
    EXPECT_EQ(value, m_redis.getValue(oldKey).asString());

    // Rename the key
    EXPECT_NO_THROW(m_redis.renameKey(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    EXPECT_EQ("", m_redis.getValue(oldKey).asString());
    EXPECT_EQ(value, m_redis.getValue(newKey).asString());

    // Clean up
    (void) m_redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, renameOverwrite)
{
    const string oldKey = "test:rename_overwrite_old";
    const string newKey = "test:rename_overwrite_new";
    const string value1 = "value1";
    const string value2 = "value2";

    // Set both keys
    m_redis.setValue(oldKey, Variant(value1));
    m_redis.setValue(newKey, Variant(value2));

    // Rename should overwrite newKey
    EXPECT_NO_THROW(m_redis.renameKey(oldKey, newKey));

    // Verify newKey has value1 (from oldKey)
    EXPECT_EQ(value1, m_redis.getValue(newKey).asString());

    // Clean up
    (void) m_redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, renameNonExistentKey)
{
    // Try to rename a non-existent key
    EXPECT_THROW(m_redis.renameKey("test:nonexistent_key_12345", "test:new_key_12345"), RedisConnectException);
}

TEST_F(RedisConnectTests, renameNX)
{
    const string oldKey = "test:renamenx_test_old";
    const string newKey = "test:renamenx_test_new";
    const string value = "test_value";

    // Clean up any previous test data
    (void) m_redis.deleteKeys({oldKey, newKey});

    // Set initial value
    m_redis.setValue(oldKey, Variant(value));

    // RenameNX should succeed when newKey doesn't exist
    EXPECT_TRUE(m_redis.renameKeyIfExists(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    EXPECT_EQ("", m_redis.getValue(oldKey).asString());
    EXPECT_EQ(value, m_redis.getValue(newKey).asString());

    // Clean up
    (void) m_redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, renameNXExistingKey)
{
    const string oldKey = "test:renamenx_existing_old";
    const string newKey = "test:renamenx_existing_new";
    const string value1 = "value1";
    const string value2 = "value2";

    // Set both keys
    m_redis.setValue(oldKey, Variant(value1));
    m_redis.setValue(newKey, Variant(value2));

    // RenameNX should fail when newKey already exists
    EXPECT_FALSE(m_redis.renameKeyIfExists(oldKey, newKey));

    // Verify both keys still have their original values
    EXPECT_EQ(value1, m_redis.getValue(oldKey).asString());
    EXPECT_EQ(value2, m_redis.getValue(newKey).asString());

    // Clean up
    (void) m_redis.deleteKeys({oldKey, newKey});

    m_redis.disconnect();
}

TEST_F(RedisConnectTests, renameHash)
{
    const string oldKey = "test:hash_old";
    const string newKey = "test:hash_new";
    const string value = "test_value";

    // Set initial value
    m_redis.setHashValues(oldKey, {{"test:akey", 1234}});
    auto keysAndValues = m_redis.getHashValues(oldKey, {"test:akey"});
    EXPECT_EQ(1234, keysAndValues["test:akey"].asInteger());

    // Rename the key
    EXPECT_NO_THROW(m_redis.renameKey(oldKey, newKey));

    // Verify old key doesn't exist and new key has the value
    keysAndValues = m_redis.getHashValues(oldKey, {"test:akey"});
    EXPECT_EQ(0, keysAndValues["test:akey"].asInteger());

    keysAndValues = m_redis.getHashValues(newKey, {"test:akey"});
    EXPECT_EQ(1234, keysAndValues["test:akey"].asInteger());

    // Clean up
    (void) m_redis.deleteKeys({newKey});
}

TEST_F(RedisConnectTests, transactionBasic)
{
    const string key1 = "test:txn_key1";
    const string key2 = "test:txn_key2";
    const string value1 = "value1";
    const string value2 = "value2";

    // Clean up any previous test data
    (void) m_redis.deleteKeys({key1, key2});

    // Begin transaction
    EXPECT_NO_THROW(m_redis.beginTransaction());

    // Queue commands
    m_redis.setValue(key1, Variant(value1));
    m_redis.setValue(key2, Variant(value2));

    // Commit transaction
    auto results = m_redis.commitTransaction();

    // Verify results - EXEC returns array of results
    EXPECT_FALSE(results.empty());

    // Verify values were set
    EXPECT_EQ(value1, m_redis.getValue(key1).asString());
    EXPECT_EQ(value2, m_redis.getValue(key2).asString());

    // Clean up
    (void) m_redis.deleteKeys({key1, key2});
}

TEST_F(RedisConnectTests, transactionRollback)
{
    const string key = "test:txn_rollback_key";
    const string initialValue = "initial";
    const string newValue = "new";

    // Set initial value
    m_redis.setValue(key, Variant(initialValue));
    EXPECT_EQ(initialValue, m_redis.getValue(key).asString());

    // Begin transaction
    EXPECT_NO_THROW(m_redis.beginTransaction());

    // Queue command
    m_redis.setValue(key, Variant(newValue));

    // Rollback transaction
    EXPECT_NO_THROW(m_redis.rollbackTransaction());

    // Verify value was NOT changed
    EXPECT_EQ(initialValue, m_redis.getValue(key).asString());

    // Clean up
    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, transactionMultipleOperations)
{
    const string key1 = "test:txn_multi_key1";
    const string key2 = "test:txn_multi_key2";
    const string key3 = "test:txn_multi_incr";

    // Clean up any previous test data
    (void) m_redis.deleteKeys({key1, key2, key3});

    // Begin transaction
    m_redis.beginTransaction();

    // Queue multiple different operations
    m_redis.setValue(key1, Variant("value1"));
    m_redis.setValue(key2, Variant(42));
    (void) m_redis.incrementKey(key3);
    (void) m_redis.incrementKey(key3);
    (void) m_redis.incrementKey(key3);

    // Commit transaction
    auto results = m_redis.commitTransaction();

    // Verify all operations executed
    EXPECT_EQ("value1", m_redis.getValue(key1).asString());
    EXPECT_EQ(42, m_redis.getValue(key2).asInteger());
    EXPECT_EQ(3, m_redis.getValue(key3).asInteger());

    // Clean up
    (void) m_redis.deleteKeys({key1, key2, key3});
}

TEST_F(RedisConnectTests, transactionWithHash)
{
    const string hashKey = "test:txn_hash";

    // Clean up any previous test data
    (void) m_redis.deleteKeys({hashKey});

    // Begin transaction
    m_redis.beginTransaction();

    // Queue hash operations
    m_redis.setHashValues(hashKey, {{"field1", 100}, {"field2", 200}});

    // Commit transaction
    auto results = m_redis.commitTransaction();

    // Verify hash was set
    auto values = m_redis.getHashValues(hashKey, {"field1", "field2"});
    EXPECT_EQ(100, values["field1"].asInteger());
    EXPECT_EQ(200, values["field2"].asInteger());

    // Clean up
    (void) m_redis.deleteKeys({hashKey});
}

TEST_F(RedisConnectTests, transactionCommitWithoutBegin)
{
    // Try to commit without beginning a transaction
    // Redis will return an error
    EXPECT_THROW((void) m_redis.commitTransaction(), RedisConnectException);
}

TEST_F(RedisConnectTests, transactionRollbackWithoutBegin)
{
    // Try to rollback without beginning a transaction
    // Redis will return an error
    EXPECT_THROW(m_redis.rollbackTransaction(), RedisConnectException);
}

TEST_F(RedisConnectTests, asyncGetValue)
{
    const string  key = "test:async_get_key";
    const Variant value = "async_value";
    m_redis.setValue(key, value);

    promise<Variant> resultPromise;
    auto             resultFuture = resultPromise.get_future();

    m_redis.getValueAsync(key, [&resultPromise](const Variant& result)
                          {
                              resultPromise.set_value(result);
                          });

    ASSERT_EQ(future_status::ready, resultFuture.wait_for(5s));
    EXPECT_EQ(value.asString(), resultFuture.get().asString());
}

TEST_F(RedisConnectTests, asyncSetValueCompletion)
{
    const string key = "test:async_set_key";

    promise<void> donePromise;
    auto          doneFuture = donePromise.get_future();

    m_redis.setValueAsync(key, Variant("hello_async"), [&donePromise]
                          {
                              donePromise.set_value();
                          });

    ASSERT_EQ(future_status::ready, doneFuture.wait_for(5s));
    EXPECT_EQ("hello_async", m_redis.getValue(key).asString());

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, asyncIncrementKey)
{
    const string key = "test:async_incr_key";
    (void) m_redis.deleteKeys({key});
    m_redis.setValue(key, static_cast<int64_t>(41L));

    promise<int64_t> resultPromise;
    auto             resultFuture = resultPromise.get_future();

    m_redis.incrementKeyAsync(key, [&resultPromise](const int64_t result)
                              {
                                  resultPromise.set_value(result);
                              });

    ASSERT_EQ(future_status::ready, resultFuture.wait_for(5s));
    EXPECT_EQ(42, resultFuture.get());

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, asyncOperationsAreOrdered)
{
    // Queued operations run sequentially on the worker thread in submission order.
    const string key = "test:async_order_key";
    (void) m_redis.deleteKeys({key});

    m_redis.setValueAsync(key, Variant("first"));
    m_redis.setValueAsync(key, Variant("second"));

    promise<Variant> resultPromise;
    auto             resultFuture = resultPromise.get_future();
    m_redis.getValueAsync(key, [&resultPromise](const Variant& result)
                          {
                              resultPromise.set_value(result);
                          });

    ASSERT_EQ(future_status::ready, resultFuture.wait_for(5s));
    EXPECT_EQ("second", resultFuture.get().asString());

    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, asyncGetHashValues)
{
    const string                      hash = "test:async_hash";
    const RedisConnect::KeysAndValues testValues = {
        {"field1", "value1"},
        {"field2", 12345}};

    (void) m_redis.deleteKeys({hash});
    m_redis.setHashValues(hash, testValues);

    promise<RedisConnect::KeysAndValues> resultPromise;
    auto                                 resultFuture = resultPromise.get_future();

    m_redis.getHashValuesAsync(hash, [&resultPromise](const RedisConnect::KeysAndValues& result)
                               {
                                   resultPromise.set_value(result);
                               });

    ASSERT_EQ(future_status::ready, resultFuture.wait_for(5s));
    auto values = resultFuture.get();
    ASSERT_EQ(2u, values.size());
    EXPECT_EQ("value1", values["field1"].asString());
    EXPECT_EQ(12345, values["field2"].asInteger());

    (void) m_redis.deleteKeys({hash});
}

TEST_F(RedisConnectTests, asyncWaitForCompletion)
{
    constexpr auto count = 200;
    atomic_size_t  completed = 0;

    for (auto i = 0; i < count; ++i)
    {
        const auto key = format("test:async_wait_key_{}", i);
        m_redis.setValueAsync(key, Variant(i), [&completed]
                              {
                                  ++completed;
                              });
    }

    m_redis.waitForAsyncCompletion();

    // After waiting, every queued operation (and its callback) has run.
    EXPECT_EQ(count, completed.load());

    vector<string> keys;
    for (auto i = 0; i < count; ++i)
    {
        keys.push_back(format("test:async_wait_key_{}", i));
    }
    (void) m_redis.deleteKeys(keys);
}

TEST_F(RedisConnectTests, asyncWaitForCompletionEmpty)
{
    // Nothing queued: returns immediately.
    EXPECT_TRUE(m_redis.waitForAsyncCompletion(5s));
}

TEST_F(RedisConnectTests, asyncWaitForCompletionTimeout)
{
    promise<void> startedPromise;
    auto          startedFuture = startedPromise.get_future();
    promise<void> releasePromise;
    auto          releaseFuture = releasePromise.get_future();

    // A first operation that blocks the worker until released, so the queue cannot drain.
    m_redis.setValueAsync("test:async_timeout_key", Variant("v"), [&startedPromise, &releaseFuture]
                          {
                              startedPromise.set_value();
                              releaseFuture.wait();
                          });

    ASSERT_EQ(future_status::ready, startedFuture.wait_for(5s));

    // The operation is still in progress, so waiting with a short timeout must fail.
    EXPECT_FALSE(m_redis.waitForAsyncCompletion(100ms));

    // Release the worker and confirm completion is then observed.
    releasePromise.set_value();
    EXPECT_TRUE(m_redis.waitForAsyncCompletion(5s));

    (void) m_redis.deleteKeys({"test:async_timeout_key"});
}

TEST_F(RedisConnectTests, asyncErrorHandlerInvokedOnPipelinedFailure)
{
    // INCR on a key holding a non-integer fails with a Redis error reply. The pipelined operation's
    // result callback must not fire; the failure must reach the error handler instead.
    const string key = "test:async_error_pipelined";
    m_redis.setValue(key, Variant("not_a_number"));

    promise<string> errorPromise;
    auto            errorFuture = errorPromise.get_future();
    bool            resultCalled = false;

    m_redis.setAsyncErrorHandler([&errorPromise](const Exception& e)
                                 {
                                     errorPromise.set_value(e.what());
                                 });

    m_redis.incrementKeyAsync(key, [&resultCalled](int64_t)
                              {
                                  resultCalled = true;
                              });

    ASSERT_EQ(future_status::ready, errorFuture.wait_for(5s));
    EXPECT_FALSE(errorFuture.get().empty());
    EXPECT_FALSE(resultCalled);

    m_redis.setAsyncErrorHandler({});
    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, asyncErrorHandlerInvokedOnSelfContainedFailure)
{
    // SREM against a key holding a string fails with WRONGTYPE. deleteSetMembersAsync is a
    // self-contained operation, so this exercises the other failure path.
    const string key = "test:async_error_self_contained";
    m_redis.setValue(key, Variant("test:a_string_not_a_set"));

    promise<string> errorPromise;
    auto            errorFuture = errorPromise.get_future();
    bool            resultCalled = false;

    m_redis.setAsyncErrorHandler([&errorPromise](const Exception& e)
                                 {
                                     errorPromise.set_value(e.what());
                                 });

    m_redis.deleteSetMembersAsync(key, {"member"}, [&resultCalled](size_t)
                                  {
                                      resultCalled = true;
                                  });

    ASSERT_EQ(future_status::ready, errorFuture.wait_for(5s));
    EXPECT_FALSE(errorFuture.get().empty());
    EXPECT_FALSE(resultCalled);

    m_redis.setAsyncErrorHandler({});
    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, asyncErrorHandlerNotInvokedOnSuccess)
{
    // A successful operation must deliver its result and never invoke the error handler.
    const string key = "test:async_error_none";
    (void) m_redis.deleteKeys({key});
    m_redis.setValue(key, static_cast<int64_t>(41L));

    atomic_bool      errorCalled {false};
    promise<int64_t> resultPromise;
    auto             resultFuture = resultPromise.get_future();

    m_redis.setAsyncErrorHandler([&errorCalled](const Exception&)
                                 {
                                     errorCalled = true;
                                 });

    m_redis.incrementKeyAsync(key, [&resultPromise](const int64_t result)
                              {
                                  resultPromise.set_value(result);
                              });

    ASSERT_EQ(future_status::ready, resultFuture.wait_for(5s));
    EXPECT_EQ(42, resultFuture.get());
    EXPECT_TRUE(m_redis.waitForAsyncCompletion(5s));
    EXPECT_FALSE(errorCalled.load());

    m_redis.setAsyncErrorHandler({});
    (void) m_redis.deleteKeys({key});
}

TEST_F(RedisConnectTests, threadSafety)
{
    constexpr auto threadCount = 10;
    constexpr auto operationsPerThread = 100;

    // Clean up keys from previous test runs
    vector<string> keysToDelete;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        keysToDelete.push_back(format("test:threadsafe_incr_{}", threadIndex));
        keysToDelete.push_back(format("test:threadsafe_hash_{}", threadIndex));
        for (auto i = 0; i < operationsPerThread; ++i)
        {
            keysToDelete.push_back(format("test:threadsafe_key_{}_{}", threadIndex, i));
        }
    }
    (void) m_redis.deleteKeys(keysToDelete);

    vector<jthread> threads;
    for (auto threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([this, threadIndex]
                             {
                                 for (auto i = 0; i < operationsPerThread; ++i)
                                 {
                                     const auto key = format("test:threadsafe_key_{}_{}", threadIndex, i);
                                     const auto value = format("value_{}_{}", threadIndex, i);

                                     // Test set/get operations
                                     m_redis.setValue(key, Variant(value));
                                     const auto retrieved = m_redis.getValue(key).asString();
                                     EXPECT_EQ(value, retrieved);

                                     // Test incr operation
                                     const auto incrKey = format("test:threadsafe_incr_{}", threadIndex);
                                     (void) m_redis.incrementKey(incrKey);

                                     // Test hash operations
                                     const auto                        hashKey = format("test:threadsafe_hash_{}", threadIndex);
                                     const RedisConnect::KeysAndValues hashValues = {{format("field_{}", i), Variant(value)}};
                                     m_redis.setHashValues(hashKey, hashValues);
                                     const auto keys = m_redis.getHashKeys(hashKey);
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
        const auto incrKey = format("test:threadsafe_incr_{}", threadIndex);
        const auto value = m_redis.getValue(incrKey).asInt64();
        EXPECT_EQ(operationsPerThread, value);
    }
}

TEST_F(RedisConnectTests, valueSetGetPerformanceAsync)
{
    constexpr auto   totalValues = 1000;
    constexpr size_t maxThreads = 4;

    vector<int> valuesSet;
    mutex       valuesMutex;

    SynchronizedQueue<int> sourceValues;
    for (auto value = 0; value < totalValues; ++value)
    {
        sourceValues.push_back(value);
    }

    vector<jthread> threads;

    Stopwatch stopwatch;
    stopwatch.start();

    for (size_t i = 0; i < maxThreads; i++)
    {
        threads.emplace_back(
            [&sourceValues, &valuesSet, &valuesMutex]
            {
                RedisConnect threadRedis;
                threadRedis.connect("theater", 6379);

                auto value = 0;
                while (!sourceValues.empty() && sourceValues.pop_front(value, 10ms))
                {
                    threadRedis.setValueAsync("test:" + to_string(value), Variant(value),
                                              [&threadRedis, value, &valuesMutex, &valuesSet]
                                              {
                                                  threadRedis.getValueAsync("test:" + to_string(value), [value, &valuesMutex, &valuesSet](const Variant& outputValue)
                                                                            {
                                                                                if (outputValue.asInteger() == value)
                                                                                {
                                                                                    scoped_lock lock(valuesMutex);
                                                                                    valuesSet.push_back(value);
                                                                                }
                                                                            });
                                              });
                }
                (void) threadRedis.waitForAsyncCompletion(30s);
                threadRedis.disconnect();
            });
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    stopwatch.stop();

    EXPECT_EQ(totalValues, valuesSet.size());

    COUT(format("Set {} keys for {:3.1f}ms ({:3.1f}K/s)", totalValues, stopwatch.milliseconds(), totalValues / stopwatch.milliseconds()));
}


// INFO answers with a RESP3 verbatim string, a type that was once not understood here. The cost
// was not the missing reply but what it left behind: the payload stayed unread in the socket, so
// the next command parsed those leftover bytes as if they were its own answer, and every command
// after it on that connection was wrong. CONFIG GET follows deliberately - it is the command that
// exposed the fault, by reporting a response type of "t" taken from the middle of INFO's text.
TEST_F(RedisConnectTests, infoFollowedByConfigGet)
{
    vector<Variant> info;
    ASSERT_NO_THROW(m_redis.executeCommand(RedisCommand("INFO", "memory"), info));
    ASSERT_FALSE(info.empty());

    const auto infoText = info.front().asString();
    EXPECT_NE(infoText.find("used_memory:"), string::npos)
        << "INFO memory should carry used_memory";
    // The three-character format marker that precedes a verbatim string belongs to the protocol,
    // not to the value, and must not reach the caller.
    EXPECT_NE(infoText.compare(0, 4, "txt:"), 0) << "The verbatim format marker was not stripped";

    // The real assertion: the connection is still usable, and this reply is this command's.
    RedisCommand getDir("CONFIG", "GET");
    getDir.emplace_back("dir");

    vector<Variant> directory;
    ASSERT_NO_THROW(m_redis.executeCommand(getDir, directory));
    ASSERT_EQ(2U, directory.size()) << "CONFIG GET answers as name and value";
    EXPECT_EQ("dir", directory[0].asString());
    EXPECT_FALSE(directory[1].asString().empty());

    // And an ordinary command after both still behaves, so nothing was left in the stream.
    const string  key = "test:info_then_config_key";
    const Variant value = "still_in_step";
    EXPECT_NO_THROW(m_redis.setValue(key, value));
    EXPECT_EQ(value.asString(), m_redis.getValue(key).asString());
}

} // namespace sptk
