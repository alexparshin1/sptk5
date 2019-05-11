/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQStorage.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday May 11 2019                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <smq/storage/SMQStorage.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;
using namespace chrono;

void SMQStorage::store(uint64_t id, SMessage& message)
{
    UniqueLock(m_mutex);
    m_messages[id] = message;
}

SMessage SMQStorage::get(uint64_t id)
{
    SharedLock(m_mutex);
    auto itor = m_messages.find(id);
    if (itor == m_messages.end())
        return SMessage();
    return itor->second;
}

SMessage SMQStorage::erase(uint64_t id)
{
    SMessage message;
    SharedLock(m_mutex);
    auto itor = m_messages.find(id);
    if (itor != m_messages.end())
        message = itor->second;
    return message;
}

#if USE_GTEST

TEST(SPTK_SMQStorage,roundTrip)
{
    SMQStorage  storage;

    size_t      messageSize(128);
    size_t      maxMessages(128*1024);

    Buffer      testData;

    for (size_t i = 0; i < messageSize; i++)
        testData.append(char(i%0xFF));

    for (size_t i = 0; i < maxMessages; i++) {
        SMessage message = make_shared<Message>(Message::MESSAGE, testData);
        storage.store(i, message);
    }

    for (size_t i = 0; i < maxMessages; i++) {
        SMessage message = storage.get(i);
        int dataMatchTestData = memcmp(message->c_str(), testData.c_str(), testData.bytes());
        EXPECT_EQ(dataMatchTestData, 0);
        EXPECT_EQ(testData.bytes(), message->bytes());
    }

    for (size_t i = 0; i < maxMessages; i++) {
        SMessage message = storage.erase(i);
        int dataMatchTestData = memcmp(message->c_str(), testData.c_str(), testData.bytes());
        EXPECT_EQ(dataMatchTestData, 0);
        EXPECT_EQ(testData.bytes(), message->bytes());
    }
}

TEST(SPTK_SMQStorage,roundTripPerformance)
{
    SMQStorage  storage;

    size_t      messageSize(128);
    size_t      maxMessages(128*1024);

    DateTime    started("now");
    Buffer      testData;

    for (size_t i = 0; i < messageSize; i++)
        testData.append(char(i%0xFF));

    for (size_t i = 0; i < maxMessages; i++) {
        SMessage message = make_shared<Message>(Message::MESSAGE, testData);
        storage.store(i, message);
    }

    for (size_t i = 0; i < maxMessages; i++) {
        SMessage message = storage.get(i);
        int dataMatchTestData = memcmp(message->c_str(), testData.c_str(), testData.bytes());
        EXPECT_EQ(dataMatchTestData, 0);
        EXPECT_EQ(testData.bytes(), message->bytes());
    }

    for (size_t i = 0; i < maxMessages; i++) {
        SMessage message = storage.erase(i);
        int dataMatchTestData = memcmp(message->c_str(), testData.c_str(), testData.bytes());
        EXPECT_EQ(dataMatchTestData, 0);
        EXPECT_EQ(testData.bytes(), message->bytes());
    }

    DateTime ended("now");
    double totalSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
    COUT("Round trip of " << maxMessages / 1024 << "K msgs took " << totalSec << " sec, " << maxMessages / totalSec << " msg/sec" << endl)
}

#endif
