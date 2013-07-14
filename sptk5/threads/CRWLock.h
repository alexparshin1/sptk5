/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRWLock.h  -  description
                             -------------------
    begin                : Thu Apr 28 2005
    copyright            : (C) 1999-2013 by Ilya A. Volynets-Evenbakh
    email                : ilya@total-knowledge.com
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

#ifndef __CRWLOCK_H__
#define __CRWLOCK_H__

#include <sptk5/sptk.h>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// Read-write synchronization object
class SP_EXPORT CRWLock
{
protected:

#ifndef WIN32
    pthread_rwlock_t    m_rwlock;   ///< pthreads rwlock object
#else
    HANDLE              m_readLock;
    HANDLE              m_writeLock;
    CRITICAL_SECTION    m_criticalSection;
    int                 m_readerCount;
    bool                m_writerMode;
#endif

public:

    /// Constructor
    CRWLock();

    /// Destructor
    ~CRWLock();

    /// Try to lock the object for reading. Blocks if object is locked for writing, or there are pending write locks.
    /// @param timeout int, timeout in milliseconds
    int lockR(int timeout = SP_INFINITY);

    /// Try to lock the object for writing. Blocks if object is locked for reading or writing.
    /// @param timeout int, timeout in milliseconds
    int lockRW(int timeout = SP_INFINITY);

    /// Releases lock on the object.
    void unlock();
};
/// @}
}

#endif
