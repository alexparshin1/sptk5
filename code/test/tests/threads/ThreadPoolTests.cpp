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
#include <sptk5/threads/ThreadPool.h>

using namespace std;
using namespace sptk;

class MyTask
    : public Runable
{
public:
    static SynchronizedQueue<int> intQueue;

    MyTask()
        : Runable("MyTask")
    {
    }

    void run() override
    {
        constexpr std::chrono::milliseconds tenMilliseconds(10);
        while (!terminated())
        {
            if (int item = 0;
                intQueue.pop_front(item, tenMilliseconds))
            {
                ++m_count;
            }
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }

    int count() const
    {
        return m_count;
    }

private:
    atomic_int m_count {0};
};

SynchronizedQueue<int> MyTask::intQueue;
namespace sptk {

TEST(ThreadPoolTests,run)
{
    vector<unique_ptr<MyTask>> tasks;
    vector<MyTask*> taskPointers;

    /// Thread manager controls tasks execution.
    constexpr uint32_t maxThreads = 16;
    constexpr std::chrono::milliseconds maxThreadIdleTime(60);
    auto threadPool = make_shared<ThreadPool>(maxThreads, maxThreadIdleTime, "test thread pool", nullptr);

    // Creating several tasks
    constexpr unsigned taskCount = 5;
    for (unsigned i = 0; i < taskCount; ++i)
    {
        auto task = make_unique<MyTask>();
        taskPointers.push_back(task.get());
        tasks.push_back(std::move(task));
    }

    for (auto& task: tasks)
    {
        threadPool->execute(std::move(task));
    }
    tasks.clear();

    constexpr int maxValues = 100;
    for (int value = 0; value < maxValues; ++value)
    {
        MyTask::intQueue.push_back(value);
    }

    constexpr chrono::milliseconds sleepInterval(300);
    this_thread::sleep_for(sleepInterval);

    for (const auto& task: taskPointers)
        EXPECT_NEAR(20, task->count(), 10);

    EXPECT_EQ(static_cast<size_t>(5), threadPool->size());

    threadPool->stop();
    EXPECT_EQ(static_cast<size_t>(0), threadPool->size());

    threadPool.reset();
}

} // namespace sptk_test
