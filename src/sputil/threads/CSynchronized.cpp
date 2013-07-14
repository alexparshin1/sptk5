/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSynchronized.cpp  -  description
                             -------------------
    begin                : Sun Jan 22 2012
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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
#ifndef _WIN32
    pthread_mutex_init(&m_synchronized, NULL);
    pthread_cond_init(&m_condition, NULL);
#else
    m_synchronized = CreateEvent(NULL, FALSE, TRUE, NULL);
#endif
}

CSynchronized::~CSynchronized()
{
#ifndef _WIN32
    pthread_mutex_destroy(&m_synchronized);
    pthread_cond_destroy(&m_condition);
#else
    CloseHandle(m_synchronized);
#endif
}

void CSynchronized::throwError(int rc, const char* fileName, int lineNumber) throw (std::exception)
{
    string error("Lock failed");

    if (fileName) {
        error += " at " + CLocation(fileName, lineNumber).toString();
        if (m_location.file())
            error += ", conflicting lock at " + m_location.toString();
    }

    error += ": ";

#ifndef _WIN32
    switch (rc) {
    case EINVAL:        throw CException(error + "Invalid mutex or timeout interval");
    case ETIMEDOUT:     throw CTimeoutException(error + "Lock timeout");
    case EAGAIN:        throw CException(error + "Maximum number of recursive locks for mutex has been exceeded");
    case EDEADLK:       throw CException(error + "The current thread already owns the mutex");
    default:            throw CException(error + "Unknown error");
    }
#else
    switch (rc) {
    case WAIT_TIMEOUT:  throw CTimeoutException(error + "Lock timeout");
    default:            throw CException(error + "Lock interrupted");
    }
#endif
}


void CSynchronized::lock(const char* fileName, int lineNumber)
{
#ifndef _WIN32
    int rc = 0;
    do {
        rc = pthread_mutex_lock(&m_synchronized);
    } while (rc == EINTR);
    if (rc != 0)
        throwError(rc, fileName, lineNumber);
#else
    DWORD rc = WaitForSingleObject(m_synchronized, INFINITE);
    if (rc != WAIT_OBJECT_0)
        throwError(rc, fileName, lineNumber);
#endif
    m_location.set(fileName, lineNumber);
}

void CSynchronized::lock(int timeoutMS, const char* fileName, int lineNumber) throw (exception)
{
#ifndef _WIN32
    int rc = 0;
    if (timeoutMS < 1) {
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
        int step = 50;   // Check every 50 milliseconds
        int elapsed = 0;
        while (elapsed < timeoutMS) {
            int rc = pthread_mutex_trylock(&m_synchronized);
            switch (rc) {
                case 0:
                    return;
                case EBUSY:
                case EINTR:
                    msleep(step);
                    elapsed += step;
                    break;
                default:
                    throwError(rc, fileName, lineNumber);
            }
        }
        throwError(ETIMEDOUT, fileName, lineNumber);
        #endif
    }
    if (rc != 0)
        throwError(rc, fileName, lineNumber);
#else
    DWORD rc = WaitForSingleObject(m_synchronized,timeoutMS);
    if (rc != WAIT_OBJECT_0)
        throwError(rc, fileName, lineNumber);
#endif
    // Storing successfull lock invokation location
    m_location.set(fileName, lineNumber);
}

bool CSynchronized::tryLock()
{
#ifndef _WIN32
#define WAIT_TIMEOUT EBUSY
    int rc = pthread_mutex_trylock(&m_synchronized);
#else
    DWORD rc = WaitForSingleObject(m_synchronized,0);
#endif
    switch (rc) {
    case 0:
        return true; // Got lock
    case WAIT_TIMEOUT:
        return false; // Locked by someone else
    default:
        throwError(rc);
        break;
    }
    return false;
}

void CSynchronized::unlock()
{
#ifndef _WIN32
    pthread_mutex_unlock(&m_synchronized);
#else
    SetEvent(m_synchronized);
#endif
}

// NOTE. TO. HAPPY. SQUIRREL
// IT IS CALLER'S RESPONSIBILITY TO LOCK THE LOCK
// OTHERWISE RACE CONDITIONS ARE INTRODUCED
// NOTE. TO. ILUXA.
// man pthread_cond_wait:
//   The pthread_cond_timedwait() and pthread_cond_wait() functions shall block on a condition variable.
//   They shall be called with mutex locked by the calling thread or undefined behavior results.
// Failing to do so makes code non-portable between *nix and Win32.
int CSynchronized::msleep(int timeoutMS)
{
#ifndef _WIN32
    lock();
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
#else
    DWORD rc = WaitForSingleObject(m_synchronized, timeoutMS);
    if (rc == WAIT_TIMEOUT)
        return -1; // Timeout or error
#endif

    return 0;
}

void CSynchronized::wakeup()
{
#ifndef _WIN32
    pthread_mutex_lock(&m_synchronized);
    pthread_cond_signal(&m_condition);
    pthread_mutex_unlock(&m_synchronized);
#else
    SetEvent(m_synchronized);
#endif
}
