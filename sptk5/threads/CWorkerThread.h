/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CWorkerThread.h  -  description
                             -------------------
    begin                : Sun Feb 26 2012
    copyright            : (C) 1999-2013 by Alexey Parshin
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

#ifndef __CWORKERTHREAD_H__
#define __CWORKERTHREAD_H__

#include <sptk5/threads/CThread.h>
#include <sptk5/threads/CThreadEvent.h>
#include <sptk5/threads/CRunable.h>
#include <sptk5/threads/CSynchronizedQueue.h>

namespace sptk {

/// @addtogroup threads Threads Classes
/// @{

/// @brief Worker thread for thread manager
///
/// Worker threads are created by thread manager.
/// They are designed to read tasks from internal or external
/// queue. Executed tasks are objects derived from CRunable.
/// If a thread event object is defined, worker thread may report events
/// such as thread start, task start, etc.
/// Worker thread automatically terminates if it's idle for the period longer
/// than defined maxIdleSec (seconds).
class SP_EXPORT CWorkerThread : public CThread
{
    bool                            m_queueOwner;       ///< If true then worker thread owns task queue
    CSynchronizedQueue<CRunable*>*  m_queue;            ///< Task queue
    CThreadEvent*                   m_threadEvent;      ///< Optional thread event interface
    uint32_t                        m_maxIdleSeconds;   ///< Number of thread idle seconds before thread terminates automatically

protected:

    /// @brief Thread function
    void threadFunction();

public:

    /// @brief Constructor
    ///
    /// If queue is NULL then worker thread uses internal task queue.
    /// Otherwise, external (shared) task queue is used.
    /// If maxIdleSec is defined and thread is idle (not executing any tasks)
    /// for a period longer than maxIdleSec then it terminates automatically.
    /// @param queue CSynchronizedQueue<CRunable*>*, Task queue
    /// @param threadEvent CThreadEvent*, Optional thread event interface
    /// @param maxIdleSeconds int32_t, Maximum time the thread is idle, seconds
    CWorkerThread(CSynchronizedQueue<CRunable*>* queue=NULL,
                  CThreadEvent* threadEvent=NULL,
                  uint32_t maxIdleSeconds=SP_INFINITY);

    /// @brief Destructor
    ~CWorkerThread();

    /// @brief Execute runable task
    /// @param task CRunable*, Task to execute in the worker thread
    void execute(CRunable* task);
};

/// @}
}

#endif
