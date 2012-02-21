/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRWLock.cpp  -  description
                             -------------------
    begin                : Thu Apr 28 2005
    copyright            : (C) 2005-2012 by Ilya A. Volynets-Evenbakh
    email                : ilya@total-knowledge.com
    based on             : CWaiter.cpp by Alexey Parshin. All rights reserved.
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
#include <sptk5/CRWLock.h>
#include <time.h>
#include <errno.h>

using namespace sptk;

#define MAX_SEMAPHOR_COUNT  16384

#ifndef _WIN32
#if HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK == 0
int pthread_rwlock_timedwrlock(pthread_rwlock_t * rwlock,
                                      const struct timespec * abs_timeout);
#endif
#if HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK == 0
int pthread_rwlock_timedrdlock(pthread_rwlock_t * rwlock,
                                      const struct timespec * abs_timeout);
#endif
#endif

CRWLock::CRWLock() {
#ifndef _WIN32
    pthread_rwlock_init(&m_rwlock,NULL);
#else

    m_readLock = CreateEvent(0L,true,true,0L);
    SetEvent(m_readLock);
    m_writeLock = CreateMutex(0L,false,0L);
    InitializeCriticalSection(&m_criticalSection);
    m_readerCount = 0;
    m_writerMode = false;
#endif
}

CRWLock::~CRWLock() {
#ifndef _WIN32
    pthread_rwlock_destroy(&m_rwlock);
#else

    CloseHandle(m_readLock);
    CloseHandle(m_writeLock);
    DeleteCriticalSection(&m_criticalSection);
#endif
}

int CRWLock::lockR(int timeout) {
#ifndef _WIN32
    if (timeout > 0) {
        int   secs  = timeout / 1000;
        int   msecs = timeout % 1000;
        struct timespec   abstime = {
                                        time(NULL) + secs, msecs * 1000L
                                    };
        return pthread_rwlock_timedrdlock(&m_rwlock,&abstime);
    } else {
        return pthread_rwlock_rdlock(&m_rwlock) == 0;
    }
#else
    if (timeout < 0)
        timeout = INFINITE;

    // Trying to make sure we don't have an active writer
    if (WaitForSingleObject(m_writeLock, timeout) == WAIT_FAILED)
        return false;

    // Increment the readers counter.
    // Switch m_readLock to non-signaled state if needed,
    // blocking the potential waiters
    EnterCriticalSection(&m_criticalSection);
    if (m_readerCount == 0)
        ResetEvent(m_readLock);
    m_readerCount++;
    m_writerMode = false;
    LeaveCriticalSection(&m_criticalSection);
    return true;
#endif
}

int CRWLock::lockRW(int timeout) {
#ifndef _WIN32
    if (timeout > 0) {
        int   secs  = timeout / 1000;
        int   msecs = timeout % 1000;
        struct timespec   abstime = {
                                        time(NULL) + secs, msecs * 1000L
                                    };
        return pthread_rwlock_timedwrlock(&m_rwlock,&abstime) == 0;
    } else {
        return pthread_rwlock_wrlock(&m_rwlock) == 0;
    }
#else
    if (timeout < 0)
        timeout = INFINITE;

    // Trying to make sure we don't have active readers or writer
    // and to acquire the mutex
    HANDLE locks[] = { m_readLock, m_writeLock };
    if (WaitForMultipleObjects(2, locks, true, timeout) == WAIT_FAILED) {
        return false;
    }

    // Only one writer is possible
    m_readerCount = 1;
    m_writerMode = true;
    return true;
#endif
}

void CRWLock::unlock() {
#ifndef _WIN32
    pthread_rwlock_unlock(&m_rwlock);
#else
    // Decrement the readers counter.
    // Switch m_readLock to signaled state if needed, unblocking all the waiters
    EnterCriticalSection(&m_criticalSection);
    if (m_writerMode)
        ReleaseMutex(m_writeLock);
    if (m_readerCount == 1)
        SetEvent(m_readLock);
    m_readerCount--;
    LeaveCriticalSection(&m_criticalSection);
#endif
}

#ifndef _WIN32
#ifndef HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK
/*!
    \fn static int pthread_rwlock_timedwrlock(pthread_rwlock_t *restrict rwlock, const struct timespec *restrict abs_timeout)
    This is ugly implemetation of timed rwlocking function, that has very low resolution, very high overhead
    (it's essantially is a spinlock with timeout) and generally should not have been written. But for poor OSs that don't
    have timers option implemented in pthreads (Solaris) it'll do it. I'm (iluxa) not using timeouts on RW locks anyways ;-)
 */
int pthread_rwlock_timedwrlock(pthread_rwlock_t * rwlock,
                                      const struct timespec * abs_timeout) {
    time_t tm,wt=abs_timeout->tv_sec;
    int rc;
    while((rc=pthread_rwlock_trywrlock(rwlock))==EBUSY) {
        time(&tm);
        if(tm>wt)
            return rc;
    }
    return rc;
}
#endif
#ifndef HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK
int pthread_rwlock_timedrdlock(pthread_rwlock_t * rwlock,
                                      const struct timespec * abs_timeout) {
    time_t tm,wt=abs_timeout->tv_sec;
    int rc;
    while((rc=pthread_rwlock_tryrdlock(rwlock))==EBUSY) {
        time(&tm);
        if(tm>wt)
            return rc;
    }
    return rc;
}
#endif
#endif
