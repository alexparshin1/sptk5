/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CThreadPool.cpp  -  description
                             -------------------
    begin                : Sat Feb 25 2012
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

class CWorkerThread : public CThread
{
    CSynchronizedQueue<CRunable*>&  m_queue;
protected:
    void threadFunction()
    {
        while (!terminated()) {
            CRunable* runable = NULL;
            if (m_queue.pop(runable, 1000)) {
                try {
                    runable->execute();
                }
                catch (exception& e) {
                    cerr << "CRunable::run() : " << e.what() << endl;
                }
                catch (...) {
                    cerr << "CRunable::run() : unknown exception" << endl;
                }
            }
        }
    }
public:
    CWorkerThread(CSynchronizedQueue<CRunable*>& queue) :
        CThread("worker"),
        m_queue(queue)
    {
    }
};


class CMonitorThread : public CThread, public CSynchronized
{
    CSynchronizedQueue<CRunable*>&  m_queue;
    std::queue<CWorkerThread*>      m_workerThreads;
protected:
    void threadFunction()
    {
        while (!terminated()) {
        }
    }
public:
    CMonitorThread(CSynchronizedQueue<CRunable*>& queue) :
        CThread("worker"),
        m_queue(queue)
    {
    }

    void startThread()
    {
        SYNCHRONIZED_CODE;
        CWorkerThread* workerThread = new CWorkerThread(m_queue);
        if (workerThread) {
            m_workerThreads.push(workerThread);
            workerThread->run();
        }
    }

    uint32_t threadCount()
    {
        SYNCHRONIZED_CODE;
        return m_workerThreads.size();
    }
};


CThreadPool::CThreadPool(uint32_t maxWorkerThreads) :
    m_monitorThread(new CMonitorThread(m_executeQueue)),
    m_maxWorkerThreads(maxWorkerThreads)
{
}

CThreadPool::~CThreadPool()
{
}

void CThreadPool::execute(CRunable* runable)
{
    SYNCHRONIZED_CODE;
    CMonitorThread* monitorThread = (CMonitorThread*)m_monitorThread;
    if (m_executeQueue.size() && monitorThread->threadCount() < m_maxWorkerThreads)
        monitorThread->startThread();
    m_executeQueue.push(runable);
}
