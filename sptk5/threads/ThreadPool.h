/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ThreadPool.h - description                             ║
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

#ifndef __SPTK_THREADPOOL_H__
#define __SPTK_THREADPOOL_H__

#include <sptk5/threads/Thread.h>
#include <sptk5/threads/ThreadEvent.h>
#include <sptk5/threads/Runable.h>
#include <sptk5/threads/SynchronizedQueue.h>
#include <sptk5/threads/SynchronizedList.h>
#include <sptk5/threads/WorkerThread.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Controls creation and execution of the threads.
 *
 * When a thread is requested from the thread pool, it ether
 * creates a new thread or returns one from the thread pool.
 * If a thread is idle for the period longer than defined in constructor,
 * it's automatically terminated.
 */
class SP_EXPORT ThreadPool : public ThreadEvent, public Thread, public std::mutex
{
    /**
     * Terminated threads scheduled for delete
     */
    SynchronizedQueue<WorkerThread*>    m_terminatedThreads;

    /**
     * All threads created by this pool
     */
    SynchronizedList<WorkerThread*>     m_threads;

    /**
     * Maximum number of threads in this pool
     */
    size_t                              m_threadLimit;

    /**
     * Share task queue
     */
    SynchronizedQueue<Runable*>         m_taskQueue;

    /**
     * Semaphore indicating available threads
     */
    Semaphore                           m_availableThreads;

    /**
     * Maximum thread idle time before thread in this pool is terminated
     */
    std::chrono::milliseconds           m_threadIdleTime;

    /**
     * Flag: true during pool shutdown
     */
    std::atomic_bool                    m_shutdown;


    /**
     * @brief Creates a new thread and adds it to thread pool
     *
     * Create new worker thread
     */
    WorkerThread* createThread();

protected:

    /**
     * @brief Thread pool control thread function
     *
     * Manages terminated threads
     */
    void threadFunction() override;

public:

    /**
     * @brief Constructor
     * @param threadLimit       Maximum number of threads in this pool
     * @param threadIdleTime    Maximum period of inactivity (seconds) for thread in the pool before thread is terminated
     * @param threadName        Thread pool own threadName
     * @param autoStart         Start upon creation
     */
    ThreadPool(uint32_t threadLimit=100, std::chrono::milliseconds threadIdleTime=std::chrono::seconds(600), const std::string& threadName="Thread Pool", bool autoStart=true);

    /**
     * @brief Destructor
     *
     * All worker threads are sent terminate() message,
     * then thread pool waits while threads are destroyed
     */
    virtual ~ThreadPool();

    /**
     * @brief Executes task
     * @param task              Task to execute
     */
    virtual void execute(Runable* task);

    /**
     * @brief Thread event callback function
     *
     * Receives events that occur in the threads
     * @param thread            Thread where event occured
     * @param eventType         Thread event type
     * @param runable           Related runable (if any)
     */
    void threadEvent(Thread* thread, ThreadEvent::Type eventType, Runable* runable) override;

    /**
     * @brief Sends terminate() message to all worker threads, and sets shutdown state
     *
     * After thread pool is stopped, it no longer accepts tasks for execution.
     */
    void stop();

    /**
     * @brief Number of active threads in the pool
     */
    size_t size() const;
};

/**
 * @}
 */
}

#endif

