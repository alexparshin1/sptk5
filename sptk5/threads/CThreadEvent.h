/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CWorkerThread.h  -  description
                             -------------------
    begin                : Sun Feb 26 2012
    copyright            : (C) 1999-2014 by Alexey Parshin
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

#ifndef __CTHREADEVENT_H__
#define __CTHREADEVENT_H__

#include <sptk5/threads/CThread.h>
#include <sptk5/threads/CRunable.h>
#include <sptk5/threads/CSynchronizedQueue.h>

namespace sptk {

/// @addtogroup threads Threads Classes
/// @{

/// @brief Thread event interface
class SP_EXPORT CThreadEvent
{
public:
    /// @brief Thread event type
    enum Type {
        THREAD_STARTED,     ///< Thread started event
        THREAD_FINISHED,    ///< Thread finished event
        RUNABLE_STARTED,    ///< Runable started
        RUNABLE_FINISHED,   ///< Runable finished
        IDLE_TIMEOUT        ///< Thread was idle longer than defined idle timeout
    };
public:
    /// @brief Thread event callback function
    ///
    /// In order to receive thread events, event receiver
    /// should be derived from this class.
    /// @param thread CThread*, Thread where event occured
    /// @param eventType Type, Thread event type
    virtual void threadEvent(CThread* thread, Type eventType) = 0;

    /// @brief Destructor
    virtual ~CThreadEvent()
    {}
};

/// @}
}

#endif
