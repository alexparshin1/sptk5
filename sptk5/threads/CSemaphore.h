/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CSemaphore.h  -  description
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

#ifndef __CSEMAPHORE_H__
#define __CSEMAPHORE_H__

#ifndef _WIN32
   #include <semaphore.h>
   #include <unistd.h>
   #include <pthread.h>
   #include <inttypes.h>
#else
   #include <winsock2.h>
   #include <windows.h>
   #include <process.h>
#endif

#include <stdexcept>
//#include <sptk5/sptk.h>
//#include <sptk5/CException.h>

namespace sptk {

/// @addtogroup threads Threads Classes
/// @{

/// @brief Generic unnamed semaphore class
class CSemaphore
{

#ifndef WIN32
    typedef sem_t   semaphore_t;
#else
    typedef HANDLE  semaphore_t;
#endif
    semaphore_t m_semaphore;        ///< Semaphore handle

public:
    /// @brief Constructor
    ///
    /// Creates semaphore with starting value (default 0)
    /// @param startingValue uint32_t, starting semaphore value
    CSemaphore(uint32_t startingValue=0);

    /// @brief Destructor
    virtual ~CSemaphore();

    /// @brief Posts the semaphore
    ///
    /// The semaphore value is increased by one.
    void post() throw (std::exception);

    /// @brief Waits until semaphore value is greater than zero, or until timeout occurs
    ///
    /// If semaphore value is greater than zero, decreases semaphore value by one and returns true.
    /// Timeout interval can be -1 (wait forever), or number of milliseconds.
    /// @param timeoutMS int32_t, wait timeout in milliseconds
    /// @return true if semaphore was pasted, or false if timeout occurs
    bool wait(uint32_t timeoutMS=-1) throw (std::exception);
};
/// @}
}

#endif
