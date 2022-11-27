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
#include <sptk5/threads/Timer.h>

using namespace std;
using namespace chrono;
using namespace sptk;

TEST(SPTK_Timer, repeat) /* NOLINT */
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        int eventSet(0);

        constexpr milliseconds repeatInterval {20};
        constexpr milliseconds sleepInterval {105};

        auto event = timer.repeat(repeatInterval,
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

class TimerTestData
{
public:
    static mutex eventCounterMutex;
    static vector<size_t> eventCounter;
    static vector<size_t> eventData;
};

mutex TimerTestData::eventCounterMutex;
vector<size_t> TimerTestData::eventCounter(MAX_EVENT_COUNTER);
vector<size_t> TimerTestData::eventData(MAX_EVENT_COUNTER);

static void gtestTimerCallback2(const uint8_t* theEventData)
{
    scoped_lock lock(TimerTestData::eventCounterMutex);
    auto eventIndex = size_t(theEventData);
    ++TimerTestData::eventCounter[eventIndex];
}

TEST(SPTK_Timer, fireOnce) /* NOLINT */
{
    mutex counterMutex;
    size_t counter = 1;
    Timer timer;

    constexpr milliseconds delayInterval {10};
    timer.fireAt(
        DateTime::Now() + delayInterval,
        [&counter, &counterMutex]() {
            scoped_lock lock(counterMutex);
            ++counter;
        });

    this_thread::sleep_for(delayInterval * 2);

    scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_Timer, repeatTwice) /* NOLINT */
{
    mutex counterMutex;
    size_t counter = 0;
    Timer timer;

    constexpr milliseconds repeatInterval {10};
    timer.repeat(
        repeatInterval,
        [&counter, &counterMutex]() {
            scoped_lock lock(counterMutex);
            ++counter;
        },
        2);

    this_thread::sleep_for(repeatInterval * 4);

    scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_Timer, repeatMultipleEvents) /* NOLINT */
{
    vector<size_t> eventCounter(MAX_EVENT_COUNTER);
    mutex eventCounterMutex;

    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        vector<STimerEvent> createdEvents;
        constexpr milliseconds repeatInterval {20};
        constexpr milliseconds testInterval {110};
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            TimerTestData::eventData[eventIndex] = eventIndex;
            function<void()> callback = bind(gtestTimerCallback2, (uint8_t*) eventIndex);
            auto event = timer.repeat(repeatInterval,
                                      [&eventCounter, &eventCounterMutex, eventIndex]
                                      {
                                          scoped_lock lock(eventCounterMutex);
                                          eventCounter[eventIndex]++;
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

        int totalEvents(0);
        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            scoped_lock lock(eventCounterMutex);
            totalEvents += (int) eventCounter[eventIndex];
        }

        EXPECT_NEAR(MAX_EVENT_COUNTER * 5, totalEvents, 10);
    }
}

TEST(SPTK_Timer, scheduleEventsPerformance) /* NOLINT */
{
    Timer timer;
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
                                  });
        createdEvents.push_back(event);
    }
    stopwatch.stop();

    COUT(maxEvents << fixed << setprecision(1) << " events scheduled, " << maxEvents / 1000.0 / stopwatch.seconds() << "K events/s" << endl)

    stopwatch.start();
    for (const auto& event: createdEvents)
    {
        event->cancel();
    }
    stopwatch.stop();

    COUT(maxEvents << fixed << setprecision(1) << " events canceled, " << maxEvents / 1000.0 / stopwatch.seconds() << "K events/s" << endl)
}
