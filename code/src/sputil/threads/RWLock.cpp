/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       RWLock.cpp - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/sptk.h>
#include <sptk5/threads/RWLock.h>

using namespace sptk;
using namespace std;

RWLock::RWLock()
{
    m_readerCount = 0;
    m_writerMode = false;
}

int RWLock::lockR(chrono::milliseconds timeout)
{
    unique_lock<mutex>  lock(m_writeLock);

    // Wait for no writers
    if (!m_condition.wait_for(lock, 
                              timeout,
                              [this](){return m_writerMode == false;}))
    {
        return false;
    }

    m_readerCount++;

    return true;
}

int RWLock::lockRW(chrono::milliseconds timeout)
{
    unique_lock<mutex>  lock(m_writeLock);

    // Wait for no readers or writers
    if (!m_condition.wait_for(lock, 
                              timeout,
                              [this](){return m_writerMode == false && m_readerCount == 0;}))
    {
        return false;
    }

    m_writerMode = true;

    return true;
}

void RWLock::unlock()
{
    lock_guard<mutex>   guard(m_writeLock);
    if (m_writerMode)
        m_writerMode = false;
    else
        if (m_readerCount > 0)
            m_readerCount--;
}

#if USE_GTEST
#include <gtest/gtest.h>

TEST(SPTK_RWLock, readLock)
{
    RWLock                  lock;
    chrono::milliseconds    timeout(10);

    lock.lockR(timeout);
    EXPECT_TRUE(lock.lockR(timeout));
    EXPECT_FALSE(lock.lockRW(timeout));
}

TEST(SPTK_RWLock, readWriteLock)
{
    RWLock                  lock;
    chrono::milliseconds    timeout(10);

    lock.lockRW(timeout);
    EXPECT_FALSE(lock.lockRW(timeout));
    EXPECT_FALSE(lock.lockRW(timeout));
}

TEST(SPTK_RWLock, unlock)
{
    RWLock                  lock;
    chrono::milliseconds    timeout(10);

    lock.lockRW(timeout);
    EXPECT_FALSE(lock.lockRW(timeout));

    lock.unlock();
    EXPECT_TRUE(lock.lockRW(timeout));
}

#endif
