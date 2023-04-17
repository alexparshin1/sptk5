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

#include <gtest/gtest.h>

#include <sptk5/Printer.h>
#include <sptk5/StopWatch.h>
#include <sptk5/threads/SynchronizedQueue.h>

#include <future>

using namespace std;
using namespace sptk;

TEST(SPTK_SynchronizedQueue, tasks)
{
    const size_t maxNumbers = 100;
    const size_t maxTasks = 10;
    const chrono::milliseconds timeout(1000);
    SynchronizedQueue<int> queue;

    vector<future<int>> tasks;
    for (size_t index = 0; index < maxTasks; ++index)
    {
        auto task = async([&queue, &timeout]() {
            int sum = 0;
            while (true)
            {
                int value;
                if (queue.pop(value, timeout))
                {
                    sum += value;
                }
                else
                {
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            return sum;
        });

        tasks.push_back(std::move(task));
    }

    this_thread::sleep_for(chrono::milliseconds(10));

    int value = 1;
    int expectedSum = 0;
    for (size_t index = 0; index < maxNumbers; ++index, ++value)
    {
        expectedSum += value;
        queue.push(value);
    }

    COUT("Pushed " << maxNumbers << " numbers to the queue" << endl);
    COUT("Expected sum is " << expectedSum << endl);

    int actualSum = 0;
    for (auto& task: tasks)
    {
        task.wait_for(chrono::milliseconds(200));
        auto sum = task.get();
        actualSum += sum;
        COUT("Sum is " << sum << ", actual sum is " << actualSum << endl);
    }

    EXPECT_EQ(expectedSum, actualSum);
}
