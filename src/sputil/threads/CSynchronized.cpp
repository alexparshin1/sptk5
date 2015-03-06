/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSynchronized.cpp  -  description
                             -------------------
    begin                : Sun Jan 22 2012
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
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

#include <sptk5/threads/CSynchronized.h>
#include <sptk5/CException.h>

using namespace std;
using namespace sptk;

CSynchronized::CSynchronized() :
    m_location(NULL,0)
{
#if USE_CXX11 == 0
    pthread_mutex_init(&m_synchronized, NULL);
    pthread_cond_init(&m_condition, NULL);
#endif
}

CSynchronized::~CSynchronized()
{
#if USE_CXX11 == 0
    pthread_mutex_destroy(&m_synchronized);
    pthread_cond_destroy(&m_condition);
#endif
}

void CSynchronized::throwError(const char* fileName, int lineNumber) THROWS_EXCEPTIONS
{
    string error("Lock failed");

    if (fileName) {
        error += " at " + CLocation(fileName, lineNumber).toString();
        if (m_location.file())
            error += ", conflicting lock at " + m_location.toString();
    }

    throw CException(error + ": Lock timeout");
}


void CSynchronized::lock(const char* fileName, int lineNumber)
{
    lock(uint32_t(-1), fileName, lineNumber);
}

#if USE_CXX11 == 0
int CSynchronized::msleep(int timeoutMS)
{
    lock();
    return msleepUnlocked(timeoutMS);
}

int CSynchronized::msleepUnlocked(int timeoutMS)
{
    if (timeoutMS > 0) {
        timespec abstime;

        if (clock_gettime(CLOCK_REALTIME, &abstime) == -1)
            throw CException("Error calling clock_gettime()");

        abstime.tv_sec += timeoutMS / 1000;
        abstime.tv_nsec += timeoutMS % 1000 * 1000000;
        if (abstime.tv_nsec >= 1000000000) {
            abstime.tv_nsec = abstime.tv_nsec % 1000000000;
            abstime.tv_sec++;
        }

        int rc = 0;
        do {
            rc = pthread_cond_timedwait(&m_condition, &m_synchronized, &abstime);
        } while (rc == EINTR);
        if (rc == ETIMEDOUT)
            return -1; // Timeout or error
    } else {
        pthread_cond_wait(&m_condition, &m_synchronized);
    }

    return 0;
}

#endif

void CSynchronized::lock(uint32_t timeoutMS, const char* fileName, int lineNumber) THROWS_EXCEPTIONS
{
#if USE_CXX11
    if (timeoutMS == uint32_t(-1))
        m_synchronized.lock();
    else {
        if (!m_synchronized.try_lock_for(chrono::milliseconds(timeoutMS)))
            throwError(fileName, lineNumber);
    }
#else
    int rc = 0;
    if (timeoutMS < 1 || timeoutMS == uint32_t(-1)) {
        do {
            rc = pthread_mutex_lock(&m_synchronized);
        } while (rc == EINTR);
    } else {
        #if HAVE_PTHREAD_MUTEX_TIMED_LOCK
        struct timespec abs_time;
        clock_gettime(CLOCK_REALTIME, &abs_time);
        abs_time.tv_sec += timeoutMS / 1000;
        abs_time.tv_nsec += timeoutMS % 1000 * 1000000;
        if (abs_time.tv_nsec >= 1000000) {
            abs_time.tv_nsec = abs_time.tv_nsec % 1000000;
            abs_time.tv_sec += abs_time.tv_nsec / 1000000;
        }
        do {
            rc = pthread_mutex_timedlock(&m_synchronized, &abs_time);
        } while (rc == EINTR);
        #else
        uint32_t step = 50;   // Check every 50 milliseconds
        uint32_t elapsed = 0;
        while (elapsed < timeoutMS) {
            int rc = pthread_mutex_trylock(&m_synchronized);
            switch (rc) {
                case 0:
                    return;
                case EBUSY:
                case EINTR:
                    msleepUnlocked(step);
                    elapsed += step;
                    break;
                default:
                    throwError(fileName, lineNumber);
            }
        }
        throwError(fileName, lineNumber);
        #endif
    }
    if (rc != 0)
        throwError(fileName, lineNumber);
#endif
    // Storing successfull lock invokation location
    m_location.set(fileName, lineNumber);
}

bool CSynchronized::tryLock()
{
#if USE_CXX11
    return m_synchronized.try_lock();
#else
    int rc = pthread_mutex_trylock(&m_synchronized);
    switch (rc) {
    case 0:
        return true; // Got lock
    case EBUSY:
        return false; // Locked by someone else
    default:
        throwError();
        break;
    }
    return false;
#endif
}

void CSynchronized::unlock()
{
#if USE_CXX11
    m_synchronized.unlock();
#else
    pthread_mutex_unlock(&m_synchronized);
#endif
}
