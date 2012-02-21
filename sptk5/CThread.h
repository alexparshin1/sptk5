/***************************************************************************
                           SIMPLY POWERFUL TOOLKIT (SPTK)
                             CThread.h  -  description
                                -------------------
        begin                : Thu Jul 12 2001
        copyright            : (C) 2001-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CTHREAD_H__
#define __CTHREAD_H__

#include <sptk5/CWaiter.h>
#include <sptk5/CFileLog.h>
#include <sptk5/CStrings.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Base thread object.
///
/// Should be used for deriving a user thread
/// by overwriting threadFunction().
class CThread {
protected:
    bool              m_terminated; ///< Flag: is the thread terminated?
    bool              m_running;    ///< Flag: is the thread running?
    bool              m_politeMode; ///< Flag: does the thread uses the 'polite' mode for shutdown?
    std::string       m_name;       ///< Thread name
    CFileLog*         m_log;        ///< Optional thread log
#ifndef _WIN32
    pthread_t         m_thread;     ///< Thread handle
#else

    HANDLE            m_thread;     ///< Thread handle
    HANDLE            m_timer;      ///< Thread timer handle
#endif

#ifndef _WIN32
    /// Internally starts a new thread
    static void *threadStart(void *p);
#else
    /// Internally starts a new thread
    static unsigned __stdcall threadStart(void *p);
#endif
    /// Internal cleanup handler - will call onThreadExit
    static void threadCleanup(void *p);

protected:

    /// Creates a thread
    void createThread();

    /// Destroys the thread
    void destroyThread();

    /// Executes the thread function
    void runThread();

    /// Exit handler, called whenever thread terminates. Does nothing by default
    virtual void onThreadExit() {}

public:

    /// Constructor
    /// @param name CString, name of the thread for future references.
    /// @param politeMode bool, true if we want to shutdown thread waitng for the crrect exit.
    /// @param fileLog CFileLog*, optional logger
    CThread(std::string name,bool politeMode=true,CFileLog* fileLog=0);

    /// Destructor
    virtual ~CThread();

    /// Starts the already created thread
    void run();

    /// The thread function. Should be overwritten by the derived class.
    virtual void threadFunction() = 0;

    /// Requests to terminate the thread
    void terminate();

    /// Returns true if the thread is terminated
    bool terminated() {
        return m_terminated;
    }

    /// Returns true if the thread is running
    bool running() {
        return m_running;
    }

    /// Returns the name of the thread
    const std::string& name() const {
        return m_name;
    }

    /// Pauses the thread
    /// @param msec int, pause time in milliseconds
    static void msleep(int msec);
};
/// @}
}

#endif
