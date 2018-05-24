/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SynchronizedCode.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_SYNCHRONIZED_CODE_H__
#define __SPTK_SYNCHRONIZED_CODE_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/Synchronized.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Synchronized code object
 *
 * Automatically locks the synchronization object until goes out of scope.
 * This allows thread-safe execution of the code after this object declaration.
 */
class SP_EXPORT SynchronizedCode
{
    /**
     * Controlled lock
     */
    Synchronized*    m_object;


public:
    /**
     * @brief Constructor
     *
     * Automatically locks the lock object. That is needed for thread safety.
     * @param object Synchronization object to lock.
     */
    explicit SynchronizedCode(Synchronized& object)
    : m_object(&object)
    {
        m_object->lock();
    }

    /**
     * @brief Constructor
     *
     * Automatically locks the lock object. That is needed for thread safety.
     * @param object            Synchronization object to lock.
     */
    explicit SynchronizedCode(Synchronized* object)
    : m_object(object)
    {
        m_object->lock();
    }

    /**
     * @brief Constructor
     *
     * Automatically locks the lock object. That is needed for thread safety.
     * If the lock isn't acquired within timeout period, CTimeoutException is thrown.
     * @param object            Synchronization object to lock.
     * @param timeout           Lock timeout
     * @param fileName          Lock location fileName, default is NULL
     * @param lineNumber        Lock location line number, default is 0
     */
    SynchronizedCode(Synchronized& object, std::chrono::milliseconds timeout, const char* fileName = NULL, int lineNumber = 0)
    : m_object(&object)
    {
        m_object->lock(timeout, fileName, lineNumber);
    }

    /**
     * @brief Constructor
     *
     * Automatically locks the lock object. That is needed for thread safety.
     * If the lock isn't acquired within timeout period, CTimeoutException is thrown.
     * @param object Synchronization object to lock.
     * @param timeout           Lock timeout, milliseconds
     * @param fileName          Lock location fileName, default is NULL
     * @param lineNumber        Lock location line number, default is 0
     */
    SynchronizedCode(Synchronized* object, std::chrono::milliseconds timeout, const char* fileName = NULL, int lineNumber = 0)
    : m_object(object)
    {
        m_object->lock(timeout, fileName, lineNumber);
    }

    /**
     * @brief Destructor
     *
     * Unlocks the lock object defined in constructor.
     */
    ~SynchronizedCode()
    {
        m_object->unlock();
    }
};

/**
 * @brief SYNCHRONIZED_CODE macro definition
 *
 * Used similarly to Windows CRITICAL_SECTION. Protects code starting from the SYNCHRONIZED_CODE definition
 * until SYNCHRONIZED_CODE goes out of scope. @see CGuard class for more information.
 * Uses 'this' object as a synchronization object, so it should be derived from Synchronized.
 */
#ifndef SPTK_LOCK_DEBUG
    #define SYNCHRONIZED_CODE sptk::SynchronizedCode lock(this)
#else
    #define SYNCHRONIZED_CODE sptk::SynchronizedCode lock(this, std::chrono::seconds(10), __FILE__, __LINE__)
#endif


/**
 * @}
 */
}

#endif
