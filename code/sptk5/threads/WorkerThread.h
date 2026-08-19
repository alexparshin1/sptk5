/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/threads/Runable.h>
#include <sptk5/threads/SynchronizedQueue.h>
#include <sptk5/threads/Thread.h>

namespace sptk {
/**
 * @addtogroup threads Thread Classes.
 * @{
 */

/**
 * @brief Worker thread for thread manager.
 *
 * Worker threads are created by the thread manager.
 * They are designed to read tasks from internal or external queue. Executed tasks are objects derived from Runable.
 * If a thread event object is defined, the worker thread may report events such as thread start, task start, etc.
 * Worker thread automatically terminates if it's idle for the period longer than defined maxIdleSec (seconds).
 */
class SP_EXPORT WorkerThread
    : public Thread
{
public:
    /**
     * @brief Constructor.
     *
     * If maxIdleSec is defined, and the thread is idle (not executing any tasks)
     * for a period longer than maxIdleSec then it terminates automatically.
     * @param queue             Task queue.
     * @param maxIdleTime       Maximum time the thread is idle, seconds.
     */
    explicit WorkerThread(SynchronizedQueue<URunable>& queue, std::chrono::milliseconds maxIdleTime = std::chrono::seconds(10));

    /**
     * @brief Destructor.
     */
    ~WorkerThread() noexcept override = default;

    /**
     * @brief Execute runable task.
     * @param task              Task to execute in the worker thread.
     */
    void execute(URunable& task) const;

    /**
     * @brief Terminate runable.
     */
    void terminate() override;

protected:
    /**
     * @brief Thread function.
     */
    void threadFunction() override;

private:
    /**
     * @brief Mutex protecting internal data.
     */
    mutable std::mutex m_mutex;

    /**
     * @brief Task queue.
     */
    SynchronizedQueue<URunable>& m_queue;

    /**
     * @brief Number of thread idle seconds before thread terminates automatically.
     */
    std::chrono::milliseconds m_maxIdleSeconds;

    Runable* m_currentRunable {nullptr};

    void setRunable(Runable* runable);
};

/**
 * @}
 */
} // namespace sptk
