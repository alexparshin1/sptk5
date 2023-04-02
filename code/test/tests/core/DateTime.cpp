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

#include <cmath>
#include <ctime>
#include <sptk5/cutils>

#include <gtest/gtest.h>

using namespace std;
using namespace chrono;
using namespace sptk;

constexpr int minutesInHour = 60;
constexpr double millisecondsInSecond = 1000.0;
constexpr size_t maxDateTimeStringLength = 128;

TEST(SPTK_DateTime, ctor1)
{
    const DateTime dateTime("2018-01-01 11:22:33.444+10");
    const chrono::milliseconds msSinceEpoch = duration_cast<chrono::milliseconds>(dateTime.sinceEpoch());
    EXPECT_EQ(1514769753444, msSinceEpoch.count());
}

TEST(SPTK_DateTime, ctor2)
{
    const DateTime dateTime1("2018-01-01 11:22:33");
    const DateTime dateTime2(2018, 1, 1, 11, 22, 33);
    const chrono::milliseconds msSinceEpoch1 = duration_cast<chrono::milliseconds>(dateTime1.sinceEpoch());
    const chrono::milliseconds msSinceEpoch2 = duration_cast<chrono::milliseconds>(dateTime2.sinceEpoch());
    EXPECT_EQ(msSinceEpoch1.count(), msSinceEpoch2.count());
}

TEST(SPTK_DateTime, ctorDate)
{
    const DateTime dateTime1("2018-02-01");
    const DateTime dateTime2(2018, 2, 1);
    const chrono::milliseconds msSinceEpoch1 = duration_cast<chrono::milliseconds>(dateTime1.sinceEpoch());
    const chrono::milliseconds msSinceEpoch2 = duration_cast<chrono::milliseconds>(dateTime2.sinceEpoch());
    EXPECT_EQ(msSinceEpoch1.count(), msSinceEpoch2.count());
    COUT(dateTime1.isoDateTimeString() << endl);
}

TEST(SPTK_DateTime, isoTimeString)
{
    const String input("2018-01-01T11:22:33");
    const DateTime dateTime1(input.c_str());
    COUT((String) dateTime1 << endl);
    const String output(dateTime1.isoDateTimeString(sptk::DateTime::PrintAccuracy::MILLISECONDS));
    EXPECT_TRUE(output.startsWith(input));
}

TEST(SPTK_DateTime, timeZones)
{
    const DateTime dateTime1("2018-01-01 09:22:33.444PM+10:00");
    const DateTime dateTime2("2018-01-01 20:22:33.444+09");
    const chrono::milliseconds msSinceEpoch1 = duration_cast<chrono::milliseconds>(dateTime1.sinceEpoch());
    const chrono::milliseconds msSinceEpoch2 = duration_cast<chrono::milliseconds>(dateTime2.sinceEpoch());
    EXPECT_EQ(1514805753444, msSinceEpoch1.count());
    EXPECT_EQ(1514805753444, msSinceEpoch2.count());
}

TEST(SPTK_DateTime, add)
{
    const DateTime dateTime1("2018-01-01 11:22:33.444+10");
    const DateTime dateTime2 = dateTime1 + chrono::milliseconds(111);
    const chrono::milliseconds msSinceEpoch2 = duration_cast<chrono::milliseconds>(dateTime2.sinceEpoch());
    EXPECT_EQ(1514769753555, msSinceEpoch2.count());
}

TEST(SPTK_DateTime, extractDate)
{
    short year = 0;
    short month = 0;
    short day = 0;
    short wday = 0;
    short yday = 0;

    const DateTime dateTime("2018-08-07 11:22:33.444Z");
    dateTime.decodeDate(&year, &month, &day, &wday, &yday, true);

    EXPECT_EQ(2018, year);
    EXPECT_EQ(8, month);
    EXPECT_EQ(7, day);
    EXPECT_EQ(2, wday);
    EXPECT_EQ(218, yday);
}

TEST(SPTK_DateTime, extractTime)
{
    short hour = 0;
    short minute = 0;
    short second = 0;
    short ms = 0;

    const DateTime dateTime("2018-08-07 11:22:33.444Z");
    dateTime.decodeTime(&hour, &minute, &second, &ms, true);

    EXPECT_EQ(11, hour);
    EXPECT_EQ(22, minute);
    EXPECT_EQ(33, second);
    EXPECT_EQ(444, ms);
}

TEST(SPTK_DateTime, formatDate)
{
    const DateTime dateTime("2018-08-07 11:22:33.444Z");

    auto t = (time_t) dateTime;
    tm tt {};
    localtime_r(&t, &tt);

    array<char, maxDateTimeStringLength> buffer {};
    strftime(buffer.data(), sizeof(buffer) - 1, "%x", &tt);

    EXPECT_STREQ("2018-08-07", dateTime.dateString(DateTime::PF_GMT | DateTime::PF_RFC_DATE).c_str());
    EXPECT_STREQ(buffer.data(), dateTime.dateString(DateTime::PF_GMT).c_str());
}

TEST(SPTK_DateTime, formatTime)
{
    const DateTime dateTime("2018-08-07 11:22:33.444Z");

    auto t = (time_t) dateTime;
    tm tt {};
    gmtime_r(&t, &tt);

    array<char, maxDateTimeStringLength> buffer {};
    strftime(buffer.data(), sizeof(buffer) - 1, "%X", &tt);

    EXPECT_STREQ("11:22:33.444Z", dateTime.timeString(DateTime::PF_GMT | DateTime::PF_TIMEZONE,
                                                      DateTime::PrintAccuracy::MILLISECONDS)
                                      .c_str());
    EXPECT_STREQ("11:22:33", dateTime.timeString(DateTime::PF_GMT).c_str());
}

TEST(SPTK_DateTime, formatDateTime2)
{
    auto tzOffsetMinutes = (int) TimeZone::offset().count();
    stringstream tzOffsetStr;
    tzOffsetStr.fill('0');
    if (tzOffsetMinutes > 0)
    {
        tzOffsetStr << "+" << tzOffsetMinutes / minutesInHour << ":" << setw(2) << tzOffsetMinutes % minutesInHour;
    }
    else if (tzOffsetMinutes < 0)
    {
        tzOffsetMinutes = -tzOffsetMinutes;
        tzOffsetStr << "-" << tzOffsetMinutes / minutesInHour << ":" << setw(2) << tzOffsetMinutes % minutesInHour;
    }
    else
    {
        tzOffsetStr << "Z";
    }

    const String tzOffset(tzOffsetStr.str());
    const DateTime dateTime(("2020-10-10 00:00:00" + tzOffset).c_str());

    auto t = (time_t) dateTime;
    tm tt {};
    localtime_r(&t, &tt);

    array<char, maxDateTimeStringLength> buffer {};
    strftime(buffer.data(), sizeof(buffer) - 1, "%x", &tt);

    EXPECT_STREQ(buffer.data(), dateTime.dateString().c_str());
    EXPECT_STREQ(("2020-10-10 00:00:00" + tzOffset).c_str(), dateTime.isoDateTimeString().replace("T", " ").c_str());
}

TEST(SPTK_DateTime, parsePerformance)
{
    constexpr size_t maxTests = 100000;
    const DateTime started("now");

    DateTime dateTime("2018-08-07 11:22:33.444Z");
    for (size_t i = 0; i < maxTests; ++i)
    {
        dateTime = DateTime("2018-08-07 11:22:33.444Z");
    }

    const DateTime ended("now");
    const double durationSec = double(duration_cast<milliseconds>(ended - started).count()) / millisecondsInSecond;

    COUT("Performed " << size_t(maxTests / millisecondsInSecond / durationSec) << "K parses/sec" << endl);
}

TEST(SPTK_DateTime, timezoneFormats1)
{
    const DateTime dt1("2021-02-03T01:02:03+10");
    const DateTime dt2("2021-02-03T01:02:03+10:00");
    const DateTime dt3("2021-02-03T01:02:03+1000");

    EXPECT_STREQ(dt1.isoDateTimeString().c_str(), dt2.isoDateTimeString().c_str());
    EXPECT_STREQ(dt1.isoDateTimeString().c_str(), dt3.isoDateTimeString().c_str());
}

TEST(SPTK_DateTime, timezoneFormats2)
{
    const DateTime dt1("2021-02-03T01:02:03-10");
    const DateTime dt2("2021-02-03T01:02:03-10:00");
    const DateTime dt3("2021-02-03T01:02:03-1000");

    EXPECT_STREQ(dt1.isoDateTimeString().c_str(), dt2.isoDateTimeString().c_str());
    EXPECT_STREQ(dt1.isoDateTimeString().c_str(), dt3.isoDateTimeString().c_str());
}

TEST(SPTK_DateTime, timezoneFormats3)
{
    const DateTime dt1("2021-02-03T01:02:03Z");
    const DateTime dt2("2021-02-03T01:02:03-00:00");
    const DateTime dt3("2021-02-03T01:02:03-00");

    EXPECT_STREQ(dt1.isoDateTimeString().c_str(), dt2.isoDateTimeString().c_str());
    EXPECT_STREQ(dt1.isoDateTimeString().c_str(), dt3.isoDateTimeString().c_str());
}

TEST(SPTK_DateTime, dateElements)
{
    const DateTime dt("2021-09-20 00:00:00");
    EXPECT_EQ(dt.daysInMonth(), 30);
    EXPECT_EQ(dt.dayOfWeek(), 1);
    EXPECT_STREQ(dt.monthName().c_str(), "September");
    EXPECT_STREQ(dt.dayOfWeekName().c_str(), "Monday");
}

TEST(SPTK_DateTime, compare)
{
    const DateTime dt1("2021-09-20 00:00:00");
    const DateTime dt2("2021-10-20 00:00:00");
    const DateTime dt3("2021-09-20 00:00:00");
    EXPECT_TRUE(dt1 < dt2);
    EXPECT_TRUE(dt1 <= dt2);
    EXPECT_TRUE(dt1 == dt3);
    EXPECT_TRUE(dt2 > dt1);
    EXPECT_TRUE(dt2 >= dt1);
}
