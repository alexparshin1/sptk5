/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CRunable.h  -  description
                             -------------------
    begin                : Fri Mar 1 2012
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

#ifndef __CRUNABLE_H__
#define __CRUNABLE_H__

#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/CFileLog.h>
#include <sptk5/CStrings.h>

namespace sptk
{

/// @addtogroup threads Thread Classes
/// @{

/// @brief Abstract runable object.
///
/// Should be used for deriving a user class for executing by a worker
/// thread in a thread pool. Derived class must override run() method.
class SP_EXPORT CRunable
{
    bool            m_terminated;   ///< Flag: is the task sent terminate request?
    CSynchronized   m_running;      ///< Synchronized object locked while the task running

protected:

    /// @brief Method that is executed by worker thread
    ///
    /// Should be overwritten by derived class.
    virtual void run() throw (std::exception) = 0;

public:

    /// @brief Default Constructor
    CRunable();

    /// @brief Destructor
    virtual ~CRunable();

    /// @brief Executes task' run method
    ///
    /// Task may be executed multiple times, but only one caller
    /// may execute same task at a time.
    void execute() throw (std::exception);

    /// @brief Requests execution termination
    void terminate();

    /// @brief Returns true if terminate request is sent to runable
    bool terminated();

    /// @brief Returns true, if the task is completed
    /// @param timeoutMS uint32_t, Wait timeout, milliseconds
    bool completed(uint32_t timeoutMS=SP_INFINITY) throw (std::exception);
};
/// @}
}

#endif
