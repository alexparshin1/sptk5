/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/LogEngine.h>
#include <sptk5/threads/Runable.h>
#include <sptk5/threads/SynchronizedList.h>
#include <sptk5/threads/SynchronizedQueue.h>
#include <sptk5/threads/ThreadEvent.h>
#include <sptk5/threads/ThreadManager.h>
#include <sptk5/threads/WorkerThread.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * Controls creation and execution of the threads.
 *
 * When a thread is requested from the thread pool, it ether
 * creates a new thread or returns one from the thread pool.
 * If a thread is idle for the period longer than defined in constructor,
 * it's automatically terminated.
 */
class SP_EXPORT ThreadPool
    : public ThreadEvent
    , public std::mutex
{
public:
    /**
     * Constructor
     * @param threadLimit       Maximum number of threads in this pool
     * @param threadIdleSeconds Maximum period of inactivity (seconds) for thread in the pool before thread is terminated
     * @param threadName        Thread pool own threadName
     */
    ThreadPool(uint32_t threadLimit, std::chrono::milliseconds threadIdleSeconds, const String& threadName,
               LogEngine* logEngine);

    /**
     * Executes task
     * @param task              Task to execute
     */
    virtual void execute(const SRunable& task);

    /**
     * Thread event callback function
     *
     * Receives events that occur in the threads
     * @param thread            Thread where event occured
     * @param eventType         Thread event type
     * @param runable           Related runable (if any)
     */
    void threadEvent(Thread* thread, ThreadEvent::Type eventType, SRunable runable) override;

    /**
     * Sends terminate() message to all worker threads, and sets shutdown state
     *
     * After thread pool is stopped, it no longer accepts tasks for execution.
     */
    virtual void stop();

    /**
     * Number of active threads in the pool
     */
    size_t size() const;

private:
    SThreadManager m_threadManager;             ///< Pool's thread manager
    size_t m_threadLimit;                       ///< Maximum number of threads in this pool
    SynchronizedQueue<SRunable> m_taskQueue;    ///< Shared task queue
    Semaphore m_availableThreads;               ///< Semaphore indicating available threads
    std::chrono::milliseconds m_threadIdleTime; ///< Maximum thread idle time before thread in this pool is terminated
    SLogger m_logger;                           ///< Optional logger

    std::atomic_bool m_shutdown {false}; ///< Flag: true during pool shutdown

    /**
     * Creates a new thread and adds it to thread pool
     *
     * Create new worker thread
     */
    WorkerThread* createThread();

    /**
     * Log thread event
     * @param event             Event info
     * @param workerThread      Related worker thread
     */
    void logThreadEvent(const String& event, const Thread* workerThread) const;
};

/**
 * @}
 */
} // namespace sptk
