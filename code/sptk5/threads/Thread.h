/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Thread.h - description                                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_THREAD_H__
#define __SPTK_THREAD_H__

#include <sptk5/threads/Locks.h>

#include <thread>
#include <atomic>
#include <mutex>
#include "Semaphore.h"

namespace sptk
{

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * Base thread object.
 *
 * Should be used for deriving a user thread
 * by overwriting threadFunction().
 */
class SP_EXPORT Thread
{
    static void threadStart(void* athread);

    /**
     * Thread synchronization object
     */
    SharedMutex         m_mutex;

    /**
     * Thread name
     */
    String              m_name;

    /**
     * Thread object
     */
    std::thread         m_thread;

    /**
     * Flag: is the thread terminated?
     */
    bool                m_terminated;

    /**
     * Pause object
     */
    Semaphore           m_pause;

public:

    /**
     * Thread ID type
     */
    typedef std::thread::id Id;


    /**
     * Constructor
     * @param name CString, name of the thread for future references.
     */
    explicit Thread(const String& name);

    /**
     * Destructor
     */
    virtual ~Thread();

    virtual /**
     * Starts the already created thread
     */
    void run();

    /**
     * Check thread status
     * @return true if thread is running
     */
    bool running() const;

    /**
     * The thread function. Should be overwritten by the derived class.
     */
    virtual void threadFunction() = 0;

    /**
     * Requests to terminate the thread
     */
    virtual void terminate();

    /**
     * This method is executed immediately after thread function exit
     */
    virtual void onThreadExit()
    {
        // Implement in derived class, if needed
    }

    /**
     * Returns true if the thread is terminated
     */
    bool terminated();

    /**
     * Waits until thread joins
     */
    void join();

    /**
     * Returns this thread OS id
     */
    Id id();

    /**
     * Returns the name of the thread
     */
    const String& name() const
    {
        return m_name;
    }

    /**
     * Sleep for interval of time
     * The sleep is automatically interrupted when terminate() is called.
     * @param interval          Interval of time
     */
    virtual bool sleep_for(std::chrono::milliseconds interval);

    /**
     * Sleep until moment of time
     * The pause is automatically interrupted when terminate() is called.
     * @param timestamp         Moment of time
     */
    virtual bool sleep_until(DateTime timestamp);
};
/**
 * @}
 */
}

#endif
