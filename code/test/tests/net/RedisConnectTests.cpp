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
#include "sptk5/net/RedisConnect.h"
#include <chrono>
#include <cstdlib>
#include <gtest/gtest.h>
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

    EXPECT_NO_THROW(redis.setBinary(key, binaryData));

    const auto retrievedData = redis.getBinary(key);
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

TEST_F(RedisConnectTests, performanceSingleThread)
{
    constexpr auto iterations = 1000;

    RedisConnect redis;
    redis.connect("localhost", 6379);

    Stopwatch watch;
    watch.start();
    for (auto i = 0; i < iterations; ++i)
    {
        auto key = format("key_{}", i);
        redis.set(key, "value1");
    }
    watch.stop();
    cout << format("Set performance: {:0.1f} ms, {:0.1f} K/s\n", watch.milliseconds(), iterations / watch.milliseconds());

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
        threads.push_back(jthread([threadIndex, iterations]
                                  {
                                      RedisConnect redis;
                                      redis.connect("10.1.1.242", 6379);

                                      for (auto i = 0; i < iterations; ++i)
                                      {
                                          auto key = format("session_{}_{}", threadIndex, i);
                                          redis.set(key, "value1-value2-value3");
                                      }

                                      redis.disconnect();
                                  }));
    }

    for (auto& thread: threads)
    {
        thread.join();
    }

    watch.stop();
    cout << format("Set performance: {:0.1f} ms, {:0.1f} K/s\n", watch.milliseconds(), iterations * threadCount / watch.milliseconds());
}

TEST_F(RedisConnectTests, scan)
{
    RedisConnect redis;
    redis.connect("127.0.0.1", 6379);

    ASSERT_TRUE(redis.isConnected());

    redis.set("test-key1", 12345);
    redis.set("test-key2", 1234);

    std::vector<Variant> values;
    redis.scan("test-*", 0, values, 999999);

    Strings keys;
    keys.resize(values.size());
    transform(values.begin(), values.end(), keys.begin(), [](const Variant& value)
              {
                  COUT(format("key: {}", value.asString().c_str()));
                  return value.asString();
              });

    // Expect two keys matched
    ASSERT_EQ(2, values.size());

    EXPECT_NE(-1, keys.indexOf("test-key1"));
    EXPECT_NE(-1, keys.indexOf("test-key2"));

    redis.disconnect();
}

} // namespace sptk
#endif
