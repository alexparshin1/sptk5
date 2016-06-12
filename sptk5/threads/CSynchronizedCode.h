/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CSynchronizedCode.h - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SYNCHRONIZED_CODE_H__
#define __SYNCHRONIZED_CODE_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/CSynchronized.h>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// @brief Synchronized code object
///
/// Automatically locks the synchronization object until goes out of scope.
/// This allows thread-safe execution of the code after this object declaration.
class SP_EXPORT CSynchronizedCode
{
    CSynchronized*    m_object;   ///< Controlled lock

public:
    /// @brief Constructor
    ///
    /// Automatically locks the lock object. That is needed for thread safety.
    /// @param object CSynchronized&, Synchronization object to lock.
    CSynchronizedCode(CSynchronized& object) :
        m_object(&object)
    {
        m_object->lock();
    }

    /// @brief Constructor
    ///
    /// Automatically locks the lock object. That is needed for thread safety.
    /// @param object CSynchronized&, Synchronization object to lock.
    CSynchronizedCode(CSynchronized* object) :
        m_object(object)
    {
        m_object->lock();
    }

    /// @brief Constructor
    ///
    /// Automatically locks the lock object. That is needed for thread safety.
    /// If the lock isn't acquired within timeout period, CTimeoutException is thrown.
    /// @param object CSynchronized&, Synchronization object to lock.
    /// @param timeoutMS uint32_t, lock timeout, milliseconds
    /// @param fileName const char*, lock location fileName, default is NULL
    /// @param lineNumber int, lock location line number, default is 0
    CSynchronizedCode(CSynchronized& object, uint32_t timeoutMS, const char* fileName = NULL, int lineNumber = 0) :
        m_object(&object)
    {
        m_object->lock(timeoutMS, fileName, lineNumber);
    }

    /// @brief Destructor
    ///
    /// Unlocks the lock object defined in constructor.
    ~CSynchronizedCode()
    {
        m_object->unlock();
    }
};

/// @brief SYNCHRONIZED_CODE macro definition
///
/// Used similarly to Windows CRITICAL_SECTION. Protects code starting from the SYNCHRONIZED_CODE definition
/// until SYNCHRONIZED_CODE goes out of scope. @see CGuard class for more information.
/// Uses 'this' object as a synchronization object, so it should be derived from CSynchronized.
#define SYNCHRONIZED_CODE sptk::CSynchronizedCode lock(this)

/// @}
}

#endif
