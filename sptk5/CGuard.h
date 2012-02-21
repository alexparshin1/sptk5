/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CGuard.h  -  description
                             -------------------
    begin                : Tue Apr 10 2007
    copyright            : (C) 2008-2012 by Alexey Parshin. All rights reserved.
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
#ifndef __CGUARD_H__
#define __CGUARD_H__

#include <sptk5/sptk.h>
#include <sptk5/CWaiter.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Guardian object
///
/// Automatically locks the lock on the declaration and unlocks that lock when goes out of scope.
/// That is needed for thread safety.
class SP_EXPORT CGuard {

    CWaiter& m_object;    ///< Controlled lock

public:
    /// @brief Constructor
    ///
    /// Automatically locks the lock object. That is needed for thread safety.
    /// @param object CSQLite3Database&, SQLite3 object to lock.
    CGuard(CWaiter& object) : m_object(object) {
        m_object.lock();
    }

    /// @brief Destructor
    ///
    /// Unlocks the lock object defined in constructor.
    ~CGuard() {
        m_object.unlock();
    }
};

/// @}
}

#endif
