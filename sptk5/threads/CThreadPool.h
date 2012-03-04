/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CThreadPool.h  -  description
                             -------------------
    begin                : Sun Feb 26 2012
    copyright            : (C) 2000-2012 by Alexey Parshin
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

#ifndef __CTHREADPOOL_H__
#define __CTHREADPOOL_H__

#include <sptk5/threads/CThread.h>
#include <sptk5/threads/CThreadEvent.h>
#include <sptk5/threads/CRunable.h>
#include <sptk5/threads/CSynchronizedQueue.h>
#include <sptk5/threads/CSynchronizedList.h>
#include <sptk5/threads/CWorkerThread.h>

namespace sptk {

/// @addtogroup threads Threads Classes
/// @{

/// @brief Controls creation and execution of the threads.
///
/// When a thread is requested from the thread pool, it ether
/// creates a new thread or returns one from the thread pool.
/// If a thread is idle for the period longer than defined in constructor,
/// it's automatically terminated.
class SP_EXPORT CThreadPool : public CSynchronized, public CThreadEvent, public CThread
{
    CSynchronizedList<CWorkerThread*>   m_terminatedThreads;    ///< Terminated threads scheduled for delete
    CSynchronizedList<CWorkerThread*>   m_threads;              ///< All threads created by this pool
    uint32_t                            m_threadLimit;          ///< Maximum number of threads in this pool
    CSynchronizedQueue<CRunable*>       m_taskQueue;            ///< Share task queue
    CSemaphore                          m_availableThreads;     ///< Semaphore indicating available threads
    uint32_t                            m_threadIdleSeconds;    ///< Maximum thread idle time before thread in this pool is terminated
    bool                                m_shutdown;             ///< Flag: true during pool shutdown

    /// @brief Creates a new thread and adds it to thread pool
    ///
    /// Create new worker thread
    CWorkerThread* createThread();

protected:

    /// @brief Thread pool control thread function
    ///
    /// Manages terminated threads
    virtual void threadFunction();


    /// @brief Sends terminate() message to all worker threads, and sets shutdown state
    void stop();

public:

    /// @brief Constructor
    /// @param threadLimit uint32_t, Maximum number of threads in this pool
    /// @param threadIdleSeconds int32_t, Maximum period of inactivity (seconds) for thread in the pool before thread is terminated
    CThreadPool(uint32_t threadLimit=100, uint32_t threadIdleSeconds=60);

    /// @brief Destructor
    ///
    /// All worker threads are sent terminate() message,
    /// then thread pool waits while threads are destroyed
    virtual ~CThreadPool();

    /// @brief Tries to lock synchronization object. Blocks until the lock is successfull.
    void execute(CRunable* task);

    /// @brief Thread event callback function
    ///
    /// Receives events that occur in the threads
    /// @param thread CThread*, Thread where event occured
    /// @param eventType CThreadEvent::Type, Thread event type
    virtual void threadEvent(CThread* thread, CThreadEvent::Type eventType);
};

/// @}
}

#endif
