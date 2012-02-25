/***************************************************************************
 SIMPLY POWERFUL TOOLKIT (SPTK)
 CRunable.h  -  description
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

#ifndef __CRUNABLE_H__
#define __CRUNABLE_H__

#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/CFileLog.h>
#include <sptk5/CStrings.h>

namespace sptk
{

/// @addtogroup threads Thread Classes
/// @{

/// @brief Base runable object.
///
/// Should be used for deriving a user class for executing by a worker
/// thread in a thread pool. Derived class must override run() method.
class CRunable
{
protected:
    bool m_terminated; ///< Flag: is the thread terminated?

public:

    /// @brief Default Constructor
    CRunable() :
            m_terminated(false)
    {
    }

    /// @brief Destructor
    virtual ~CRunable()
    {
    }

    /// @brief Method that is executed by worker thread
    ///
    /// Should be overwritten by derived class
    virtual void run()
    {
        m_terminated = false;
    }

    /// Requests to terminate the thread
    void terminate();

    /// Returns true if the thread is terminated
    bool terminated()
    {
        return m_terminated;
    }
};
/// @}
}

#endif
