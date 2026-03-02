/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/threads/SynchronizedQueue.h>
#include <sptk5/threads/Thread.h>

namespace sptk {

/**
 * @brief ThreadManager stores a list of threads and automatically joins and destroys terminated threads
 * that are registered with it.
 */
class SP_EXPORT ThreadManager
    : public Thread
{
public:
    /**
     * @brief Constructor.
     * @param name              Thread manager thread name.
     */
    explicit ThreadManager(const String& name);

    /**
     * @brief Destructor.
     */
    ~ThreadManager() override;

    /**
     * @brief Start thread manager monitoring of the threads.
     */
    void start();

    /**
     * @brief Stop thread manager monitoring of the threads.
     */
    void stop();

    /**
     * @brief Register the thread for monitoring.
     * @param thread            Thread.
     */
    void manage(const SThread& thread);

    /**
     * @brief Destroy the thread if it was monitored.
     * @param thread            Thread.
     */
    void destroyThread(const SThread& thread);

    /**
     * @return Count of currently running monitored threads.
     */
    size_t threadCount() const;

    /**
     * @brief Get the next running thread by index.
     * @return thread or nullptr.
     */
    SThread getNextThread();

protected:
    /**
     * @brief Monitoring thread function.
     */
    void threadFunction() override;

private:
    mutable std::mutex         m_mutex;                  ///< Mutex that protects internal data.
    std::vector<SThread>       m_managedThreads;         ///< Running threads.
    SynchronizedQueue<SThread> m_terminatedThreads;      ///< Terminated threads scheduled for delete.
    size_t                     m_nextThreadIndex {0};    ///< Next thread index.
    std::chrono::milliseconds  m_terminatedQueueTimeout; ///< Terminated thread queue timeout.

    /**
     * @brief Join terminated threads.
     */
    void joinTerminatedThreads();

    /**
     * @brief Terminate all running monitored threads.
     */
    void terminateRunningThreads();
};

using SThreadManager = std::shared_ptr<ThreadManager>;

} // namespace sptk
