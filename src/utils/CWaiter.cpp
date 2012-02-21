/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWaiter.cpp  -  description
                             -------------------
    begin                : Fri Jul 20 2001
    copyright            : (C) 2001-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CWaiter.h>
#include <sptk5/CException.h>
#include <errno.h>
#include <string.h>

using namespace sptk;

CWaiter::CWaiter() {
#ifndef _WIN32
    pthread_mutex_init(&m_waiter,NULL);
    pthread_cond_init(&m_condition,NULL);
#else
    m_waiter = CreateEvent(NULL, FALSE, TRUE, NULL);
#endif
}

CWaiter::~CWaiter() {
#ifndef _WIN32
    pthread_mutex_destroy(&m_waiter);
    pthread_cond_destroy(&m_condition);
#else
    CloseHandle(m_waiter);
#endif
}

void CWaiter::lock() {
#ifndef _WIN32
    pthread_mutex_lock(&m_waiter);
#else
    WaitForSingleObject(m_waiter,INFINITE);
#endif
}

bool CWaiter::try_lock() {
#ifndef _WIN32
    #define WAIT_TIMEOUT EBUSY
    int res = pthread_mutex_trylock(&m_waiter);
#else
    DWORD res = WaitForSingleObject(m_waiter,0);
#endif
    switch (res) {
    case 0:
	    return true; // Got lock
    case WAIT_TIMEOUT:
	    return false; // Locked by someone else
    default:
	    throw CException("Unable to aquire lock: ");
	}
#ifdef __MSCVER__
	return false;
#endif
}

void CWaiter::unlock() {
#ifndef _WIN32
    pthread_mutex_unlock(&m_waiter);
#else
    SetEvent(m_waiter);
#endif
}

int CWaiter::waitForSignal(int timeout) {
#ifndef _WIN32
    if (timeout > 0) {
        int   secs  = timeout / 1000;
        int   msecs = timeout % 1000;
        struct timespec   abstime = { time(NULL) + secs, msecs * 1000L };
        int rc = pthread_cond_timedwait(&m_condition,&m_waiter,&abstime);
	if(rc != 0)
	    return -1;
    } else 
        pthread_cond_wait(&m_condition,&m_waiter);
    
#else
    WaitForSingleObject(m_waiter,timeout);
#endif

    return 0;
}

void CWaiter::sendSignalNoLock() {
#ifndef _WIN32
    pthread_cond_signal(&m_condition);
#else
    SetEvent(m_waiter);
#endif
}

void CWaiter::sendSignal() {
    lock();
    sendSignalNoLock();
    unlock();
}

void CWaiter::broadcastSignalNoLock() {
#ifndef _WIN32
    pthread_cond_broadcast(&m_condition);
#endif
}

void CWaiter::broadcastSignal() {
    lock();
    broadcastSignalNoLock();
    unlock();
}
