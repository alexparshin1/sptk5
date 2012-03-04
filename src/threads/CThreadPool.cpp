/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CThreadPool.cpp  -  description
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

#include <sptk5/threads/CThreadPool.h>

using namespace std;
using namespace sptk;

CThreadPool::CThreadPool(uint32_t threadLimit, uint32_t threadIdleSeconds) :
    CThread("thread manager"),
    m_threadLimit(threadLimit),
    m_threadIdleSeconds(threadIdleSeconds),
    m_shutdown(false)
{
    createThread();
    run();
}

CThreadPool::~CThreadPool()
{
}

void CThreadPool::threadFunction()
{
    while (!terminated()) {
        CWorkerThread* workerThread = NULL;
        if (m_terminatedThreads.pop_front(workerThread, 1000)) {
            m_threads.remove(workerThread);
            delete workerThread;
        }
    }
}

CWorkerThread* CThreadPool::createThread()
{
    CWorkerThread*  workerThread = new CWorkerThread(&m_taskQueue, this, m_threadIdleSeconds);
    m_threads.push_back(workerThread);
    workerThread->run();
    return workerThread;
}

void CThreadPool::execute(CRunable* task)
{
    SYNCHRONIZED_CODE;
    if (m_shutdown)
        throw CException("Thread manager is stopped");

    if (!m_availableThreads.wait(10)) {
        if (m_threads.size() < m_threadLimit)
            createThread();
    }

    m_taskQueue.push(task);
}

void CThreadPool::threadEvent(CThread* thread, CThreadEvent::Type eventType)
{
    SYNCHRONIZED_CODE;
    switch (eventType) {
    case CThreadEvent::RUNABLE_STARTED:
        break;
    case CThreadEvent::RUNABLE_FINISHED:
        m_availableThreads.post();
        break;
    case CThreadEvent::THREAD_FINISHED:
        m_terminatedThreads.push_back((CWorkerThread*)thread);
        break;
    case CThreadEvent::THREAD_STARTED:
    case CThreadEvent::IDLE_TIMEOUT:
        break;
    }
}

static bool terminateThread(CWorkerThread*& thread, void*)
{
    thread->terminate();
    return true;
}

void CThreadPool::stop()
{
    SYNCHRONIZED_CODE;
    m_shutdown = true;
    m_threads.each(terminateThread);
}
