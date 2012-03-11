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

#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/CFileLog.h>
#include <sptk5/CStrings.h>

namespace sptk
{

/// @addtogroup threads Thread Classes
/// @{

/// @brief Base thread object.
///
/// Should be used for deriving a user thread
/// by overwriting threadFunction().
class SP_EXPORT CThread
{
protected:
    bool        m_terminated;   ///< Flag: is the thread terminated?
    std::string m_name;         ///< Thread name
    uint64_t    m_id;
#ifndef _WIN32
    pthread_t   m_thread;       ///< Thread handle
#else
    HANDLE      m_thread;       ///< Thread handle
    HANDLE      m_timer;        ///< Thread timer handle
#endif

#ifndef _WIN32
    /// @brief Internally starts a new thread
    static void *threadStart(void *p);
#else
    /// @brief Internally starts a new thread
    static unsigned __stdcall threadStart(void *p);
#endif

protected:

    /// @brief Creates a thread
    void createThread();

    /// @brief Waits until thread joins
    void joinThread();

    /// @brief Destroys the thread
    void destroyThread();

    /// @brief Executes the thread function
    void runThread();

public:

    /// @brief Constructor
    /// @param name CString, name of the thread for future references.
    CThread(std::string name);

    /// @brief Destructor
    virtual ~CThread();

    /// @brief Starts the already created thread
    void run();

    /// @brief The thread function. Should be overwritten by the derived class.
    virtual void threadFunction() = 0;

    /// @brief Requests to terminate the thread
    void terminate();

    /// @brief Returns true if the thread is terminated
    bool terminated() const
    {
        return m_terminated;
    }

    /// @brief Returns true if the thread is running
    bool running() const
    {
        return m_id != 0;
    }

    /// @brief Returns this thread OS id
    uint64_t id() const
    {
        return m_id;
    }

    /// @brief Returns the name of the thread
    const std::string& name() const
    {
        return m_name;
    }

    /// @brief Returns context thread OS id
    static uint64_t contextThreadId();

    /// @brief Pauses the thread
    /// @param msec int, pause time in milliseconds
    static void msleep(int msec);
};
/// @}
}

#endif
