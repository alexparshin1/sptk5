/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CSemaphore.cpp  -  description
                             -------------------
    begin                : Sat Feb 25 2012
    copyright            : (C) 1999-2013 by Alexey Parshin
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

#include <sptk5/threads/CSemaphore.h>
#include <sptk5/CSystemException.h>

#include <limits.h>
#include <time.h>

using namespace std;
using namespace sptk;

CSemaphore::CSemaphore(uint32_t startingValue)
{
#if USE_CXX11
    m_value = (int) startingValue;
#else
    int rc = sem_init(&m_semaphore, 0, startingValue);
    if (rc != 0)
        throw CSystemException("Create semaphore");
#endif
}

CSemaphore::~CSemaphore()
{
#if USE_CXX11 == 0
    sem_destroy(&m_semaphore);
#endif
}

void CSemaphore::post() THROWS_EXCEPTIONS
{
#if USE_CXX11
    m_value++;
#else
    int rc = sem_post(&m_semaphore);
    if (rc != 0)
        throw CSystemException("Post semaphore");
#endif
}

bool CSemaphore::wait(uint32_t timeoutMS) THROWS_EXCEPTIONS
{
#if USE_CXX11
    unique_lock<mutex>  lock(m_mutex);
    
    // Wait until semaphore value is greater than 0
    if (!m_condition.wait_for(lock, 
                              chrono::milliseconds(timeoutMS), 
                              [this](){return m_value > 0;}))
    {
        return false;
    }

    m_value--;

    return true;
#else
    /// Get current absolute time
    timespec abstime;
    if (clock_gettime(CLOCK_REALTIME, &abstime) == -1)
        throw CSystemException("clock_gettime()");

    /// Offset current time to the end of the waiting intervale
    abstime.tv_sec += timeoutMS / 1000;
    abstime.tv_nsec += timeoutMS % 1000 * 1000;

    /// Wait for semaphore post
    int rc = 0;
    for (;;) {
        rc = sem_timedwait(&m_semaphore, &abstime);
        if (rc == 0)
            return true;

        switch(errno)        {
            case ETIMEDOUT:
                return false;
            case EINTR:
                break;
            default:
                throw CSystemException("Wait for semaphore");
        }
    }
    return false;
#endif
}
