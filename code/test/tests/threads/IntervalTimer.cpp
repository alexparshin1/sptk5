/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/cutils>
#include <sptk5/threads/IntervalTimer.h>

using namespace std;
using namespace chrono;
using namespace sptk;

TEST(SPTK_IntervalTimer, repeat) /* NOLINT */
{
    if (DateTime::Now() > DateTime()) // always true
    {
        constexpr milliseconds repeatInterval {20};
        constexpr milliseconds sleepInterval {105};

        IntervalTimer timer(repeatInterval);

        int eventSet(0);

        IntervalTimer::Event event = timer.repeat(
            [&eventSet]() {
                ++eventSet;
            });

        this_thread::sleep_for(sleepInterval);
        event->cancel();

        EXPECT_NEAR(5, eventSet, 2);
    }
}

const int MAX_EVENT_COUNTER = 10;
const int MAX_TIMERS = 10;

class IntervalTimerTestData
{
public:
    static mutex eventCounterMutex;
    static vector<size_t> eventCounter;
    static vector<size_t> eventData;
};

mutex IntervalTimerTestData::eventCounterMutex;
vector<size_t> IntervalTimerTestData::eventCounter(MAX_EVENT_COUNTER);
vector<size_t> IntervalTimerTestData::eventData(MAX_EVENT_COUNTER);

static void gtestIntervalTimerCallback2(const uint8_t* theEventData)
{
    scoped_lock lock(IntervalTimerTestData::eventCounterMutex);
    auto eventIndex = size_t(theEventData);
    ++IntervalTimerTestData::eventCounter[eventIndex];
}

TEST(SPTK_IntervalTimer, fireOnce) /* NOLINT */
{
    constexpr milliseconds delayInterval {10};

    mutex counterMutex;
    size_t counter = 1;
    IntervalTimer timer(delayInterval);

    timer.repeat(
        [&counter, &counterMutex]() {
            scoped_lock lock(counterMutex);
            ++counter;
        },
        1);

    this_thread::sleep_for(delayInterval * 2);

    scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_IntervalTimer, repeatTwice) /* NOLINT */
{
    constexpr milliseconds repeatInterval {10};

    mutex counterMutex;
    size_t counter = 0;
    IntervalTimer timer(repeatInterval);

    timer.repeat(
        [&counter, &counterMutex]() {
            scoped_lock lock(counterMutex);
            ++counter;
        },
        2);

    this_thread::sleep_for(repeatInterval * 4);

    scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_IntervalTimer, repeatMultipleEvents) /* NOLINT */
{
    if (DateTime::Now() > DateTime()) // always true
    {
        constexpr milliseconds repeatInterval {20};
        IntervalTimer timer(repeatInterval);

        vector<IntervalTimer::Event> createdEvents;
        constexpr milliseconds testInterval {110};
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            IntervalTimerTestData::eventData[eventIndex] = eventIndex;
            function<void()> callback = bind(gtestIntervalTimerCallback2, (uint8_t*) eventIndex);
            IntervalTimer::Event event = timer.repeat(callback);
            createdEvents.push_back(event);
        }

        this_thread::sleep_for(testInterval);

        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            IntervalTimer::Event event = createdEvents[eventIndex];
            event->cancel();
        }

        this_thread::sleep_for(repeatInterval);

        int totalEvents(0);
        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            scoped_lock lock(IntervalTimerTestData::eventCounterMutex);
            totalEvents += (int) IntervalTimerTestData::eventCounter[eventIndex];
        }

        EXPECT_NEAR(MAX_EVENT_COUNTER * 5, totalEvents, 10);
    }
}

TEST(SPTK_IntervalTimer, repeatMultipleIntervalTimers) /* NOLINT */
{
    constexpr int repeatCount {10};
    constexpr milliseconds repeatInterval {10};
    vector<shared_ptr<IntervalTimer>> timers;
    for (size_t i = 0; i < MAX_TIMERS; ++i)
    {
        timers.emplace_back(make_shared<IntervalTimer>(repeatInterval));
    }

    if (!timers.empty())
    {
        scoped_lock lock(IntervalTimerTestData::eventCounterMutex);
        IntervalTimerTestData::eventCounter.clear();
        IntervalTimerTestData::eventCounter.resize(MAX_EVENT_COUNTER);
    }

    for (auto& timer: timers)
    {
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            function<void()> callback = bind(gtestIntervalTimerCallback2, (uint8_t*) eventIndex);
            timer->repeat(
                callback,
                repeatCount);
        }
    }

    this_thread::sleep_for(repeatInterval * 30);

    int totalEvents(0);
    if (!timers.empty())
    {
        scoped_lock lock(IntervalTimerTestData::eventCounterMutex);
        for (auto counter: IntervalTimerTestData::eventCounter)
        {
            totalEvents += (int) counter;
        }
    }

    EXPECT_EQ(MAX_TIMERS * MAX_EVENT_COUNTER * repeatCount, totalEvents);
}

TEST(SPTK_IntervalTimer, scheduleEventsPerformance) /* NOLINT */
{
    IntervalTimer timer(hours(1));
    constexpr size_t maxEvents = 100000;
    vector<IntervalTimer::Event> createdEvents;

    StopWatch stopwatch;

    stopwatch.start();
    for (size_t eventIndex = 0; eventIndex < maxEvents; ++eventIndex)
    {
        function<void()> callback = bind(gtestIntervalTimerCallback2, (uint8_t*) eventIndex);
        IntervalTimer::Event event = timer.repeat(callback, 1);
        createdEvents.push_back(event);
    }
    stopwatch.stop();

    COUT(maxEvents << " events scheduled, " << maxEvents / stopwatch.seconds() << " events/s" << endl)

    stopwatch.start();
    for (const auto& event: createdEvents)
    {
        event->cancel();
    }
    stopwatch.stop();

    COUT(maxEvents << " events canceled, " << maxEvents / stopwatch.milliseconds() / 1000.0 << " events/s" << endl)
}
