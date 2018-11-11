/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WorkerThread.h - description                           ║
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

#ifndef __SPTK_WORKERTHREAD_H__
#define __SPTK_WORKERTHREAD_H__

#include <sptk5/threads/Thread.h>
#include <sptk5/threads/ThreadEvent.h>
#include <sptk5/threads/Runable.h>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Worker thread for thread manager
 *
 * Worker threads are created by thread manager.
 * They are designed to read tasks from internal or external
 * queue. Executed tasks are objects derived from Runable.
 * If a thread event object is defined, worker thread may report events
 * such as thread start, task start, etc.
 * Worker thread automatically terminates if it's idle for the period longer
 * than defined maxIdleSec (seconds).
 */
class SP_EXPORT WorkerThread : public Thread
{
    /**
     * Task queue
     */
    SynchronizedQueue<Runable*>&    m_queue;

    /**
     * Optional thread event interface
     */
    ThreadEvent*                    m_threadEvent;

    /**
     * Number of thread idle seconds before thread terminates automatically
     */
    std::chrono::milliseconds       m_maxIdleSeconds;


protected:

    /**
     * @brief Thread function
     */
    void threadFunction();

public:

    /**
     * @brief Constructor
     *
     * If queue is NULL then worker thread uses internal task queue.
     * Otherwise, external (shared) task queue is used.
     * If maxIdleSec is defined and thread is idle (not executing any tasks)
     * for a period longer than maxIdleSec then it terminates automatically.
     * @param queue             Task queue
     * @param threadEvent       Optional thread event interface
     * @param maxIdleTime       Maximum time the thread is idle, seconds
     */
    WorkerThread(SynchronizedQueue<Runable*>& queue,
                 ThreadEvent* threadEvent = nullptr,
                 std::chrono::milliseconds maxIdleTime = std::chrono::seconds(3600));

    /**
     * @brief Destructor
     */
    ~WorkerThread() = default;

    /**
     * @brief Execute runable task
     * @param task              Task to execute in the worker thread
     */
    void execute(Runable* task);
};

/**
 * @}
 */
}

#endif
