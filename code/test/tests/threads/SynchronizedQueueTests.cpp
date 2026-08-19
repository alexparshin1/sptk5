/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <gtest/gtest.h>

#include <sptk5/Printer.h>
#include <sptk5/Stopwatch.h>
#include <sptk5/threads/SynchronizedQueue.h>

#include <future>

using namespace std;
using namespace sptk;

namespace sptk {

TEST(SynchronizedQueueTests, tasks)
{
    constexpr size_t               maxNumbers = 100;
    constexpr size_t               maxTasks = 5;
    constexpr chrono::milliseconds timeout(1000);
    SynchronizedQueue<int>         queue;

    int value = 1;
    int expectedSum = 0;
    for (size_t index = 0; index < maxNumbers; ++index, ++value)
    {
        expectedSum += value;
        queue.push_back(value);
    }

    vector<future<int>> tasks;
    for (size_t index = 0; index < maxTasks; ++index)
    {
        auto task = async([&queue, &timeout]()
                          {
                              int sum = 0;
                              int value = 0;
                              while (queue.pop_front(value, timeout))
                              {
                                  sum += value;
                                  this_thread::sleep_for(10ms);
                              }
                              return sum;
                          });

        tasks.push_back(std::move(task));
    }

    COUT("");

    int actualSum = 0;
    for (auto& task: tasks)
    {
        const auto sum = task.get();
        EXPECT_NEAR(1000, sum, 300);
        actualSum += sum;
    }

    EXPECT_EQ(expectedSum, actualSum);
}

TEST(SynchronizedQueueTests, emplaceBack)
{
    struct Item
    {
        int         index;
        std::string name;
    };

    SynchronizedQueue<Item> queue;

    queue.emplace_back(1, "Joe");
    queue.emplace_back(2, "Jane");

    EXPECT_EQ(2, queue.size());

    Item item;
    EXPECT_TRUE(queue.pop_front(item, 100ms));
    EXPECT_EQ(1, item.index);
    EXPECT_EQ("Joe", item.name);

    EXPECT_TRUE(queue.pop_front(item, 100ms));
    EXPECT_EQ(2, item.index);
    EXPECT_EQ("Jane", item.name);
}

TEST(SynchronizedQueueTests, performance)
{
    constexpr size_t               maxNumbers = 10000000;
    constexpr chrono::milliseconds timeout(1000);
    SynchronizedQueue<int>         queue;

    Stopwatch stopWatch;

    stopWatch.start();
    int value = 1;
    int actualSum = 0;
    for (size_t index = 0; index < maxNumbers; ++index, ++value)
    {
        queue.push_back(value);
        actualSum += value;
    }
    stopWatch.stop();
    COUT("Pushed " << maxNumbers << " ints: " << fixed << setprecision(2) << maxNumbers / stopWatch.seconds() / 1E6 << "M ints per second");

    stopWatch.start();
    int receivedSum = 0;
    for (size_t index = 0; index < maxNumbers; ++index)
    {
        if (queue.pop_front(value, timeout))
        {
            receivedSum += value;
        }
    }
    stopWatch.stop();
    COUT("Popped " << maxNumbers << " ints: " << fixed << setprecision(2) << maxNumbers / stopWatch.seconds() / 1E6 << "M ints per second");

    EXPECT_FALSE(queue.pop_front(value, 100ms));
    EXPECT_EQ(actualSum, receivedSum);
}

TEST(SynchronizedQueueTests, performanceBulk)
{
    constexpr size_t               maxNumbers = 100000;
    constexpr chrono::milliseconds timeout(1000);
    SynchronizedQueue<int>         queue;

    Stopwatch stopWatch;

    stopWatch.start();
    auto value = 1;
    int  actualSum = 0;
    for (size_t index = 0; index < maxNumbers; ++index, ++value)
    {
        queue.push_back(value);
        actualSum += value;
    }
    stopWatch.stop();
    COUT("Pushed " << maxNumbers << " ints: " << fixed << setprecision(2) << maxNumbers / stopWatch.seconds() / 1E6 << "M ints per second");

    stopWatch.start();

    constexpr size_t bulkSize = 16;
    int              receivedSum = 0;
    vector<int>      values(bulkSize);
    for (size_t index = 0; index < maxNumbers; index += bulkSize)
    {
        if (queue.pop_front(values, bulkSize, timeout))
        {
            for (auto value: values)
            {
                receivedSum += value;
            }
        }
    }
    stopWatch.stop();
    COUT("Popped " << maxNumbers << " ints: " << fixed << setprecision(2) << maxNumbers / stopWatch.seconds() / 1E6 << "M ints per second");

    EXPECT_FALSE(queue.pop_front(value, 100ms));
    EXPECT_EQ(actualSum, receivedSum);
}

} // namespace sptk