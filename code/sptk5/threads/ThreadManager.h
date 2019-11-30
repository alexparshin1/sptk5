/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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

#ifndef __SPTK_THREADMANAGER_H__
#define __SPTK_THREADMANAGER_H__

#include <sptk5/cthreads>

namespace sptk {

class ThreadManager
{
    class Joiner : public Thread
    {
        SynchronizedQueue<SThread>  m_terminatedThreads;    ///< Terminated threads scheduled for delete
    protected:
        void threadFunction() override;
        void joinTerminatedThreads();
    public:
        explicit Joiner(const String& name);
        ~Joiner() override;
        void push(SThread& thread);
        void stop();
    };

    mutable std::mutex          m_mutex;                ///< Mutex that protects internal data
    std::map<Thread*, SThread>  m_runningThreads;       ///< Running threads
    Joiner                      m_joiner;

    void terminateRunningThreads();

public:
    explicit ThreadManager(const String& name);
    virtual ~ThreadManager();

    void start();
    void stop();

    void registerThread(SThread thread);
    void finalizeThread(Thread* thread);
};

typedef std::shared_ptr<ThreadManager>  SThreadManager;

}

#endif
