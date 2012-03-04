/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSynchronized.h  -  description
                             -------------------
    begin                : Thu Feb 23 2012
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

#ifndef __SYNCHRONIZED_H__
#define __SYNCHRONIZED_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/CLocation.h>

namespace sptk {

/// @addtogroup threads Threads Classes
/// @{

/// @brief Synchronization object
class SP_EXPORT CSynchronized
{
    /// @brief Throws error description for the error code
    /// @param rc int, Error code
    /// @param fileName const char*, File name where lock is invoked
    /// @param lineNumber int, Line number where lock is invoked
    void throwError(int rc, const char* fileName=NULL, int lineNumber=0) throw (std::exception);

protected:

#ifndef _WIN32
    // PThreads native objects
    pthread_mutex_t     m_synchronized;     ///< Mutex object
    pthread_cond_t      m_condition;        ///< Mutex condition object
#else
    HANDLE              m_synchronized;     ///< Win32 event object
#endif
    CLocation           m_location;         ///< Location of latest successfull lock()

public:

    /// @brief Constructor
    CSynchronized();

    /// @brief Destructor
    virtual ~CSynchronized();

    /// @brief Tries to lock synchronization object. Blocks until the lock is successfull.
    /// @param fileName const char*, lock location fileName
    /// @param lineNumber int, lock location line number
    virtual void lock(const char* fileName=NULL, int lineNumber = 0);

    /// @brief Tries to lock synchronization object. Blocks until the lock is obtained, or until timeout occurs.
    ///
    /// Throws CTimeoutException exception if timeout.
    /// Throws CException exception if lock was interrupted.
    /// @param timeoutMS int, lock timeout, milliseconds
    /// @param fileName const char*, lock location fileName, default is NULL
    /// @param lineNumber int, lock location line number, default is 0
    virtual void lock(int timeoutMS, const char* fileName=NULL, int lineNumber = 0) throw (std::exception);

    /// @brief Tries to lock synchronization object.
    /// @return true is lock may be acquired, or false if not.
    virtual bool tryLock();

    /// @brief Unlocks the synchronization object.
    virtual void unlock();

    /// @brief Sleeps until receives wakeup(), or until timeout occurs
    /// @param timeoutMS int, timeout in milliseconds
    /// @return 0 on success or -1 on timeout or error
    virtual int msleep(int timeoutMS=-1);

    /// @brief Wakes up a sleeping object
    virtual void wakeup();
};

/// @}
}

#endif
