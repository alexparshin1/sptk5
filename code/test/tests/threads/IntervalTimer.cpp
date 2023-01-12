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

        auto event = timer.repeat(
            [&eventSet]() {
                ++eventSet;
            });

        this_thread::sleep_for(sleepInterval);
        event->cancel();

        EXPECT_NEAR(5, eventSet, 2);
    }
}

const int MAX_EVENT_COUNTER = 10;

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
        int totalEvents(0);
        mutex counterMutex;

        vector<STimerEvent> createdEvents;
        constexpr milliseconds testInterval {110};
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            auto event = timer.repeat([&totalEvents, &counterMutex] {
                scoped_lock lock(counterMutex);
                ++totalEvents;
            });
            createdEvents.push_back(event);
        }

        this_thread::sleep_for(testInterval);

        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            auto event = createdEvents[eventIndex];
            event->cancel();
        }

        this_thread::sleep_for(repeatInterval);

        EXPECT_NEAR(MAX_EVENT_COUNTER * 5, totalEvents, 10);
    }
}

TEST(SPTK_IntervalTimer, scheduleEventsPerformance) /* NOLINT */
{
    IntervalTimer timer(hours(1));
    constexpr size_t maxEvents = 100000;
    vector<STimerEvent> createdEvents;

    StopWatch stopwatch;

    stopwatch.start();
    for (size_t eventIndex = 0; eventIndex < maxEvents; ++eventIndex)
    {
        auto event = timer.repeat(
            [] {
                // Empty callback method
            },
            1);
        createdEvents.push_back(event);
    }
    stopwatch.stop();

    const auto OneThousand = 1000.0;
    COUT(maxEvents << fixed << setprecision(1) << " events scheduled, " << maxEvents / OneThousand / stopwatch.seconds() << "K events/s" << endl)

    stopwatch.start();
    for (const auto& event: createdEvents)
    {
        event->cancel();
    }
    stopwatch.stop();

    COUT(maxEvents << fixed << setprecision(1) << " events canceled, " << maxEvents / OneThousand / stopwatch.seconds() << "K events/s" << endl)
}
