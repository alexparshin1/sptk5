/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWaiter.h  -  description
                             -------------------
    begin                : Mon Apr 17 2000
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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
#ifndef __CWAITER_H__
#define __CWAITER_H__

#ifndef _WIN32
   #include <unistd.h>
   #include <pthread.h>
#else
   #include <winsock2.h>
   #include <windows.h>
   #include <process.h>
#endif

#include <sptk5/sptk.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Synchronization object
class SP_EXPORT CWaiter {

protected:

#ifndef _WIN32
    pthread_mutex_t   m_waiter;             ///< Mutex object
    pthread_cond_t    m_condition;          ///< Condition object
#else
    HANDLE            m_waiter;             ///< Event object
#endif

public:

    /// @brief Constructor
    CWaiter();

    /// @brief Destructor
    ~CWaiter();

    /// @brief Tries to lock waiter. Blocks until the lock.
    void lock();

    /// @brief Tries to lock waiter. Returns false if it fails. Doesn't block.
    bool try_lock();

    /// @brief Unlocks the waiter.
    void unlock();

    /// @brief Waits for a signal, timeout in msec
    /// @param timeout int, timeout in milliseconds
    int  waitForSignal(int timeout=-1);

    /// @brief Sends signal to stop waiting.
    ///
    /// Caller needs to aquire lock explicitly before calling this.
    void sendSignalNoLock();

    /// @brief Sends signal to stop waiting
    void sendSignal();

    /// @brief Sends signal to all waiting threads.
    ///
    /// Caller needs to aquire lock explicitly before calling this.
    void broadcastSignalNoLock();

    /// @brief Sends signal to all waiting threads.
    void broadcastSignal();
}
;
/// @}
}

#endif
