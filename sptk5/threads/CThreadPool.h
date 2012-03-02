/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CThreadPool.h  -  description
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

#ifndef __CTHREADPOOL_H__
#define __CTHREADPOOL_H__

#include <sptk5/threads/CSynchronizedQueue.h>
#include <sptk5/threads/CThread.h>
#include <sptk5/threads/CRunable.h>
#include <queue>
#include <list>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// @brief Thread pool that executes runable (derived from CRunable) objects
class CThreadPool : public CSynchronized
{
    CSynchronizedQueue<CRunable*>   m_executeQueue;     ///< Queue of the runable objects to be executed
    CThread*                        m_monitorThread;    ///< Monitor thread
    uint32_t                        m_maxWorkerThreads; ///< Maximum number of worker threads
public:
    /// @brief Constructor
    /// @param maxWorkerThreads uint32_t, Maximum number of worker threads
    CThreadPool(uint32_t maxWorkerThreads=100);

    /// @brief Destructor
    virtual ~CThreadPool();

    /// @brief Executes runable
    ///
    /// Runable is placed to the queue for execution in worker threads,
    /// and execute() immediately returns.
    /// @param runable CRunable*, Runable to execute
    void execute(CRunable* runable);
};

/// @}
}

#endif
