/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>

#include <sptk5/Printer.h>
#include <sptk5/StopWatch.h>
#include <sptk5/threads/SynchronizedQueue.h>

#include <future>

using namespace std;
using namespace sptk;

TEST(SPTK_SynchronizedQueue, tasks)
{
    constexpr size_t maxNumbers = 100;
    constexpr size_t maxTasks = 5;
    constexpr chrono::milliseconds timeout(1000);
    SynchronizedQueue<int> queue;

    vector<future<int>> tasks;
    for (size_t index = 0; index < maxTasks; ++index)
    {
        auto task = async([&queue, &timeout]() {
            int sum = 0;
            while (true)
            {
                if (int value;
                    queue.pop_front(value, timeout))
                {
                    sum += value;
                }
                else
                {
                    break;
                }
                this_thread::sleep_for(10ms);
            }
            return sum;
        });

        tasks.push_back(std::move(task));
    }

    int value = 1;
    int expectedSum = 0;
    for (size_t index = 0; index < maxNumbers; ++index, ++value)
    {
        expectedSum += value;
        queue.push_back(value);
    }

    int actualSum = 0;
    const int expectedSumPerTask = expectedSum / static_cast<int>(maxTasks);
    for (auto& task: tasks)
    {
        task.wait_for(chrono::milliseconds(200));
        const auto sum = task.get();
        actualSum += sum;
        // Expect tasks doing about the same amount of work
        EXPECT_NEAR(expectedSumPerTask, sum, 100);
    }

    EXPECT_EQ(expectedSum, actualSum);
}

TEST(SPTK_SynchronizedQueue, performance)
{
    constexpr size_t maxNumbers = 10000;
    constexpr chrono::milliseconds timeout(1000);
    SynchronizedQueue<int> queue;

    StopWatch stopWatch;

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

    EXPECT_EQ(actualSum, receivedSum);
}

TEST(SPTK_SynchronizedQueue, for_each)
{
    constexpr int maxNumbers = 10;
    SynchronizedQueue<int> queue;

    int actualSum = 0;
    for (int index = 1; index < maxNumbers; ++index)
    {
        queue.push_back(index);
        if (index < 5)
        {
            actualSum += index;
        }
    }

    int receivedSum = 0;
    queue.each([&receivedSum](const int& item) {
        if (item < 5)
        {
            receivedSum += item;
            return true;
        }
        return false;
    });

    EXPECT_EQ(actualSum, receivedSum);
}
