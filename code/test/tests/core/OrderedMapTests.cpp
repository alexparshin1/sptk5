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
#include "sptk5/Strings.h"


#include <format>
#include <gtest/gtest.h>
#include <ranges>
#include <sptk5/OrderedMap.h>

using namespace std;
using namespace sptk;

namespace {
Strings allKeys(const OrderedMap<string, int>& orderedMap)
{
    Strings keys;
    for (const auto& key: orderedMap | views::keys)
    {
        keys.push_back(key);
    }
    return keys;
}

vector<int> allValues(const OrderedMap<string, int>& orderedMap)
{
    vector<int> values;
    for (const auto& value: orderedMap | views::values)
    {
        values.push_back(value);
    }
    return values;
}

OrderedMap<string, int> makeOrderedMap(const Strings& keys, const vector<int>& values)
{
    OrderedMap<string, int> orderedMap;
    for (size_t i = 0; i < keys.size(); ++i)
    {
        orderedMap.insert(keys[i], values[i]);
    }
    return orderedMap;
}

void verifyKeysAndValues(const OrderedMap<string, int>& orderedMap, const Strings& keys, const vector<int>& values)
{
    EXPECT_EQ(keys.size(), orderedMap.size());
    for (size_t i = 0; i < keys.size(); ++i)
    {
        EXPECT_EQ(values[i], orderedMap.at(keys[i]));
    }
}
} // namespace
namespace sptk {

TEST(OrderedMapTests,copyCtor)
{
    const Strings expectedKeys {"a", "b", "c", "d"};
    const vector  expectedValues {4, 3, 2, 1};

    const auto orderedMap = makeOrderedMap(expectedKeys, expectedValues);
    OrderedMap orderedMapCopy(orderedMap);

    verifyKeysAndValues(orderedMapCopy, expectedKeys, expectedValues);
    orderedMapCopy.clear();
}

TEST(OrderedMapTests,moveCtor)
{
    const Strings expectedKeys {"a", "b", "c", "d"};
    const vector  expectedValues {4, 3, 2, 1};

    auto       orderedMap = makeOrderedMap(expectedKeys, expectedValues);
    OrderedMap orderedMapCopy(std::move(orderedMap));

    verifyKeysAndValues(orderedMapCopy, expectedKeys, expectedValues);
}

TEST(OrderedMapTests,copyAssignment)
{
    const Strings expectedKeys {"a", "b", "c", "d"};
    const vector  expectedValues {4, 3, 2, 1};

    const auto              orderedMap = makeOrderedMap(expectedKeys, expectedValues);
    OrderedMap<string, int> orderedMapCopy;

    orderedMapCopy = orderedMap;

    verifyKeysAndValues(orderedMapCopy, expectedKeys, expectedValues);
}

TEST(OrderedMapTests,moveAssignment)
{
    const Strings expectedKeys {"a", "b", "c", "d"};
    const vector  expectedValues {4, 3, 2, 1};

    auto       orderedMap = makeOrderedMap(expectedKeys, expectedValues);
    OrderedMap orderedMapCopy(std::move(orderedMap));

    verifyKeysAndValues(orderedMapCopy, expectedKeys, expectedValues);
}

TEST(OrderedMapTests,insertAndAccessByKey)
{
    const Strings expectedKeys {"a", "b", "c", "d"};
    const vector  expectedValues {4, 2, 3, 1};

    const auto orderedMap = makeOrderedMap(expectedKeys, expectedValues);

    verifyKeysAndValues(orderedMap, expectedKeys, expectedValues);
}

TEST(OrderedMapTests,insertAndAccessByKeyAt)
{
    const Strings expectedKeys {"a", "b", "c", "d"};
    const vector  expectedValues {4, 2, 3, 1};

    auto orderedMap = makeOrderedMap(expectedKeys, expectedValues);

    EXPECT_EQ(expectedValues[0], orderedMap.at(expectedKeys[0]));
    EXPECT_EQ(expectedValues[1], orderedMap.at(expectedKeys[1]));
    EXPECT_EQ(expectedValues[2], orderedMap.at(expectedKeys[2]));
    EXPECT_EQ(expectedValues[3], orderedMap.at(expectedKeys[3]));

    EXPECT_ANY_THROW(orderedMap.at("e"));
    orderedMap.at("b") = 10;
    EXPECT_EQ(10, orderedMap.at("b"));
}

TEST(OrderedMapTests,updateKeepsInsertionOrder)
{
    const Strings expectedKeys {"first", "second"};
    const vector  expectedValues {10, 2};

    OrderedMap<string, int> orderedMap;
    orderedMap.insert("first", 1);
    orderedMap.insert("second", 2);
    orderedMap.insert("first", 10);

    ASSERT_EQ(2U, orderedMap.size());
    EXPECT_EQ(10, orderedMap["first"]);

    EXPECT_EQ(expectedKeys, allKeys(orderedMap));
    EXPECT_EQ(expectedValues, allValues(orderedMap));
}

TEST(OrderedMapTests,eraseByKey)
{
    const Strings expectedKeys {"a", "c"};
    const vector  expectedValues {1, 3};

    OrderedMap<string, int> orderedMap;
    orderedMap.insert("a", 1);
    orderedMap.insert("b", 2);
    orderedMap.insert("c", 3);

    EXPECT_TRUE(orderedMap.erase("b"));
    EXPECT_FALSE(orderedMap.contains("b"));
    ASSERT_EQ(2U, orderedMap.size());

    EXPECT_EQ(expectedKeys, allKeys(orderedMap));
    EXPECT_EQ(expectedValues, allValues(orderedMap));
}

TEST(OrderedMapTests,clear)
{
    OrderedMap<string, int> orderedMap;
    orderedMap.insert("x", 10);
    orderedMap.insert("y", 20);

    orderedMap.clear();

    EXPECT_TRUE(orderedMap.empty());
    EXPECT_EQ(0U, orderedMap.size());
}

TEST(OrderedMapTests,frontAndBack)
{
    OrderedMap<string, int> orderedMap;
    orderedMap.insert("x", 10);
    orderedMap.insert("y", 20);

    EXPECT_EQ(2U, orderedMap.size());
    EXPECT_EQ(10, orderedMap.front());
    EXPECT_EQ(20, orderedMap.back());
}

TEST(OrderedMapTests,frontAndBackOnEmptyThrows)
{
    OrderedMap<string, int> orderedMap;

    EXPECT_THROW((void) orderedMap.front(), std::out_of_range);
    EXPECT_THROW((void) orderedMap.back(), std::out_of_range);

    const auto& constMap = orderedMap;
    EXPECT_THROW((void) constMap.front(), std::out_of_range);
    EXPECT_THROW((void) constMap.back(), std::out_of_range);
}

TEST(OrderedMapTests,eraseByIteratorAndEndBounds)
{
    const Strings expectedKeys {"a", "c"};
    const vector  expectedValues {1, 3};

    OrderedMap<string, int> orderedMap;
    orderedMap.insert("a", 1);
    orderedMap.insert("b", 2);
    orderedMap.insert("c", 3);

    auto it = orderedMap.begin();
    ++it;
    EXPECT_TRUE(orderedMap.erase(it));
    EXPECT_EQ(expectedKeys, allKeys(orderedMap));
    EXPECT_EQ(expectedValues, allValues(orderedMap));

    EXPECT_FALSE(orderedMap.erase(orderedMap.end()));
    EXPECT_EQ(expectedKeys, allKeys(orderedMap));
    EXPECT_EQ(expectedValues, allValues(orderedMap));
}

TEST(OrderedMapTests,comparePerformance)
{
    map<string, int, less<>> testData;
    for (auto i = 0; i < 1000000; ++i)
    {
        auto key = format("key{}", i);
        testData.try_emplace(key, i);
    }

    Stopwatch stopWatch;
    stopWatch.start();
    OrderedMap<string, int> orderedMap;
    for (const auto& [key, value]: testData)
    {
        orderedMap.insert(key, value);
    }
    stopWatch.stop();
    COUT("OrderedMap insertion: " << stopWatch.milliseconds() << " ms");

    EXPECT_EQ(testData.size(), orderedMap.size());

    stopWatch.start();
    orderedMap.clear();
    stopWatch.stop();
    COUT("OrderedMap clear:     " << stopWatch.milliseconds() << " ms");

    stopWatch.start();
    map<string, int, less<>> stdMap;
    for (const auto& [key, value]: testData)
    {
        stdMap.emplace(key, value);
    }
    stopWatch.stop();
    COUT("Std map insertion:    " << stopWatch.milliseconds() << " ms");

    stopWatch.start();
    stdMap.clear();
    stopWatch.stop();
    COUT("Std map clear:        " << stopWatch.milliseconds() << " ms");

    stopWatch.start();
    unordered_map<string, int> unorderedMap;
    for (const auto& [key, value]: testData)
    {
        unorderedMap.emplace(key, value);
    }
    stopWatch.stop();
    COUT("Hash map insertion:   " << stopWatch.milliseconds() << " ms");

    stopWatch.start();
    unorderedMap.clear();
    stopWatch.stop();
    COUT("Hash map clear:       " << stopWatch.milliseconds() << " ms");
}

} // namespace sptk_test
