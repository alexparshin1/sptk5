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
#include <sptk5/cutils>
#include <sptk5/threads/Timer.h>

using namespace std;
using namespace chrono;
using namespace sptk;

TEST(SPTK_Timer, repeat) /* NOLINT */
{
    if (DateTime::Now() > DateTime()) // always true
    {
        const Timer timer;

        int eventSet(0);

        constexpr milliseconds repeatInterval {20};
        constexpr milliseconds sleepInterval {105};

        const auto event = timer.repeat(repeatInterval,
                                        [&eventSet]() {
                                      ++eventSet;
                                  });

        this_thread::sleep_for(sleepInterval);
        event->cancel();

        EXPECT_NEAR(5, eventSet, 2);
    }
}

const int MAX_EVENT_COUNTER = 10;

TEST(SPTK_Timer, fireOnce) /* NOLINT */
{
    mutex counterMutex;
    size_t      counter = 1;
    const Timer timer;

    constexpr milliseconds delayInterval {10};
    timer.fireAt(
        DateTime::Now() + delayInterval,
        [&counter, &counterMutex]() {
            const scoped_lock lock(counterMutex);
            ++counter;
        });

    this_thread::sleep_for(delayInterval * 2);

    const scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, static_cast<size_t>(2));
}

TEST(SPTK_Timer, repeatTwice) /* NOLINT */
{
    mutex counterMutex;
    size_t counter = 0;
    const Timer timer;

    constexpr auto repeatInterval {10ms};
    timer.repeat(
        repeatInterval,
        [&counter, &counterMutex]()
        {
            const scoped_lock lock(counterMutex);
            ++counter;
        },
        2);

    this_thread::sleep_for(repeatInterval * 4);

    const scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, static_cast<size_t>(2));
}

TEST(SPTK_Timer, repeatTwoEventsTwice) /* NOLINT */
{
    mutex counterMutex;
    size_t counter = 0;
    const Timer timer;

    constexpr auto repeatInterval {10ms};
    timer.repeat(
        repeatInterval,
        [&counter, &counterMutex]() {
            const scoped_lock lock(counterMutex);
            ++counter;
        },
        2);
    timer.repeat(
        repeatInterval,
        [&counter, &counterMutex]() {
            const scoped_lock lock(counterMutex);
            ++counter;
        },
        2);

    this_thread::sleep_for(repeatInterval * 4);

    const scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, static_cast<size_t>(4));
}

TEST(SPTK_Timer, repeatMultipleEvents) /* NOLINT */
{
    int totalEvents(0);
    mutex eventCounterMutex;

    if (DateTime::Now() > DateTime()) // always true
    {
        const Timer timer;

        vector<STimerEvent> createdEvents;
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            auto event = timer.repeat(20ms,
                                      [&totalEvents, &eventCounterMutex] {
                                          const scoped_lock lock(eventCounterMutex);
                                          totalEvents++;
                                      });
            createdEvents.push_back(event);
        }

        this_thread::sleep_for(110ms);

        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            const auto event = createdEvents[eventIndex];
            event->cancel();
        }

        this_thread::sleep_for(20ms);

        EXPECT_NEAR(MAX_EVENT_COUNTER * 5, totalEvents, 10);
    }
}

TEST(SPTK_Timer, repeatCancel) /* NOLINT */
{
    atomic_int totalEvents(0);

    const Timer timer;

    vector<STimerEvent> createdEvents;
    const auto event = timer.repeat(10ms,
                              [&totalEvents] {
                                  totalEvents++;
                              });

    this_thread::sleep_for(100ms);
    EXPECT_NEAR(9, totalEvents, 2);
    event->cancel();

    this_thread::sleep_for(1ms);
    totalEvents = 0;

    this_thread::sleep_for(100ms);

    EXPECT_EQ(0, totalEvents);
}

TEST(SPTK_Timer, scheduleEventsPerformance) /* NOLINT */
{
    const Timer timer;
    constexpr size_t maxEvents = 100000;
    vector<STimerEvent> createdEvents;

    StopWatch stopwatch;

    DateTime when("now");
    when = when + hours(1);

    stopwatch.start();
    for (size_t eventIndex = 0; eventIndex < maxEvents; ++eventIndex)
    {
        auto event = timer.fireAt(when,
                                  [] {
                                      // Do nothing
                                  });
        createdEvents.push_back(event);
    }
    stopwatch.stop();

    const auto OneThousand = 1000.0;
    COUT(maxEvents << fixed << setprecision(1) << " events scheduled, " << maxEvents / OneThousand / stopwatch.seconds() << "K events/s");

    stopwatch.start();
    for (const auto& event: createdEvents)
    {
        event->cancel();
    }
    stopwatch.stop();

    COUT(maxEvents << fixed << setprecision(1) << " events canceled, " << maxEvents / OneThousand / stopwatch.seconds() << "K events/s");
}
