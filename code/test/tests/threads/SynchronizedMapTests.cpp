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

#include "sptk5/threads/SynchronizedMap.h"


#include <gtest/gtest.h>

#include <sptk5/Printer.h>
#include <sptk5/Stopwatch.h>
#include <sptk5/threads/SynchronizedQueue.h>

#include <future>
#include <thread>

using namespace std;
using namespace sptk;
namespace sptk {

TEST(SynchronizedMapTests, keysValues)
{
    SynchronizedMap<int, string> map;

    auto                 maxNumbers = 1000;
    vector<future<void>> tasks;
    for (auto i = 0; i < maxNumbers; ++i)
    {
        auto task = async([&map, i]()
                          {
                              map.insert(i, format("Value {}", i));
                          });
        tasks.push_back(std::move(task));
    }

    for (const auto& task: tasks)
    {
        task.wait();
    }

    for (auto i = 0; i < maxNumbers; ++i)
    {
        string value;
        map.get(i, value);
        EXPECT_EQ(format("Value {}", i), value);
    }
}

TEST(SynchronizedMapTests, insertClear)
{
    constexpr auto maxNumbers = 1000;

    SynchronizedMap<int, string> map;
    for (auto i = 0; i < maxNumbers; ++i)
    {
        map.insert(i, format("Value {}", i));
    }

    EXPECT_EQ(maxNumbers, map.size());

    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(0, map.size());
}

TEST(SynchronizedMapTests, modifyValue)
{
    constexpr auto maxNumbers = 10;

    SynchronizedMap<int, string> map;
    for (auto i = 0; i < maxNumbers; ++i)
    {
        map.insert(i, format("Value {}", i));
    }

    map.modify(2, [](auto& value)
               {
                   value = "Modified, was: " + value;
               });
    string value;
    map.get(2, value);
    EXPECT_EQ("Modified, was: Value 2", value);
}

TEST(SynchronizedMapTests, remove)
{
    constexpr auto maxNumbers = 1000;

    SynchronizedMap<int, string> map;
    for (auto i = 0; i < maxNumbers; ++i)
    {
        map.insert(i, format("Value {}", i));
    }
    for (auto i = 0; i < maxNumbers; ++i)
    {
        EXPECT_TRUE(map.erase(i));
    }
    EXPECT_TRUE(map.empty());
}

TEST(SynchronizedMapTests, forEach)
{
    constexpr auto maxNumbers = 1000;

    SynchronizedMap<int, string> map;
    for (auto i = 0; i < maxNumbers; ++i)
    {
        map.insert(i, format("Value {}", i));
    }

    auto i = 0;
    map.for_each([&i](const auto& key, const auto& value)
                 {
                     EXPECT_EQ(i++, key);
                     EXPECT_EQ("Value " + to_string(key), value);
                     return true;
                 });

    EXPECT_EQ(maxNumbers, i);
}

TEST(SynchronizedMapTests, performance)
{
    constexpr auto maxNumbers = 100000;
    constexpr auto threadCount = 8;

    SynchronizedQueue<int>    queue;
    SynchronizedMap<int, int> map;

    Stopwatch sw;
    sw.start();

    vector<jthread> threads;
    for (auto th = 0; th < threadCount; ++th)
    {
        threads.emplace_back([&queue, &map]
                             {
                                 while (!queue.empty())
                                 {
                                     int i = 0;
                                     if (queue.pop_front(i, 100ms))
                                     {
                                         map.insert(i, i);
                                         map.get(i, i);
                                     }
                                 }
                             });
    }

    for (auto i = 0; i < maxNumbers; ++i)
    {
        queue.push_back(i);
    }

    threads.clear();

    sw.stop();
    COUT("Map test: " << setprecision(2) << sw.milliseconds() << "ms, " << maxNumbers / sw.milliseconds() / 1000.0 << "M/sec");
}

} // namespace sptk
