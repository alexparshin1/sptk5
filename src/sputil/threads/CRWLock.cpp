/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRWLock.cpp  -  description
                             -------------------
    begin                : Thu Apr 28 2005
    copyright            : (C) 1999-2014 by Ilya A. Volynets-Evenbakh
    email                : ilya@total-knowledge.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/sptk.h>
#include <sptk5/threads/CRWLock.h>
#include <time.h>
#include <errno.h>
#if USE_CXX11
    #include <thread>
#endif

using namespace sptk;
using namespace std;

#if USE_CXX11 == 0
    #define MAX_SEMAPHORE_COUNT  16384
    #if HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK == 0
    int pthread_rwlock_timedwrlock(pthread_rwlock_t * rwlock, const struct timespec * abs_timeout);
    int pthread_rwlock_timedrdlock(pthread_rwlock_t * rwlock, const struct timespec * abs_timeout);
    #endif
#endif

CRWLock::CRWLock()
{
#if USE_CXX11
    m_readerCount = 0;
    m_writerMode = false;
#else
    pthread_rwlock_init(&m_rwlock,NULL);
#endif
}

CRWLock::~CRWLock()
{
#if USE_CXX11 == 0
    pthread_rwlock_destroy(&m_rwlock);
#endif
}

int CRWLock::lockR(int timeoutMS)
{
#if USE_CXX11
    if (timeoutMS < 0)
        timeoutMS = 999999999;

    unique_lock<mutex>  lock(m_writeLock);

    // Wait for no writers
    if (!m_condition.wait_for(lock, 
                              chrono::milliseconds(timeoutMS), 
                              [this](){return m_writerMode == false;}))
    {
        return false;
    }

    m_readerCount++;

    return true;
#else
    if (timeoutMS > 0) {
        int   secs  = timeoutMS / 1000;
        int   msecs = timeoutMS % 1000;
        struct timespec   abstime = {
                                        time(NULL) + secs, msecs * 1000L
                                    };
        return pthread_rwlock_timedrdlock(&m_rwlock,&abstime);
    } else
        return pthread_rwlock_rdlock(&m_rwlock) == 0;
#endif
}

int CRWLock::lockRW(int timeoutMS)
{
#if USE_CXX11
    if (timeoutMS < 0)
        timeoutMS = 999999999;

    unique_lock<mutex>  lock(m_writeLock);
    
    // Wait for no readers or writers
    if (!m_condition.wait_for(lock, 
                              chrono::milliseconds(timeoutMS), 
                              [this](){return m_writerMode == false && m_readerCount == 0;}))
    {
        return false;
    }

    m_writerMode = true;

    return true;
#else
    if (timeoutMS > 0) {
        int   secs  = timeoutMS / 1000;
        int   msecs = timeoutMS % 1000;
        struct timespec   abstime = {
                                        time(NULL) + secs, msecs * 1000L
                                    };
        return pthread_rwlock_timedwrlock(&m_rwlock,&abstime) == 0;
    } else
        return pthread_rwlock_wrlock(&m_rwlock) == 0;
#endif
}

void CRWLock::unlock()
{
#if USE_CXX11
    lock_guard<mutex>   guard(m_writeLock);
    if (m_writerMode)
        m_writerMode = false;
    else
        if (m_readerCount > 0)
            m_readerCount--;
#else
    pthread_rwlock_unlock(&m_rwlock);
#endif
}
