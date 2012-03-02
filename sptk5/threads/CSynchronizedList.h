/***************************************************************************
*                          SIMPLY POWERFUL TOOLKIT (SPTK)
*                          CSynchronizedList.h  -  description
*                             -------------------
*    begin                : Sat Feb 25 2012
*    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
*    email                : alexeyp@gmail.com
***************************************************************************/

/***************************************************************************
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* Neither the name of the <ORGANIZATION> nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
*  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/

#ifndef __CSYNCHRONIZEDLIST_H__
#define __CSYNCHRONIZEDLIST_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/threads/CSemaphore.h>
#include <list>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// @brief Synchronized template list
///
/// Simple thread-safe list
template <class T>
class CSynchronizedList
{
    CSemaphore              m_semaphore;        ///< Semaphore to waiting for an item if list is empty
    std::list<T>*           m_list;             ///< List

protected:

    mutable CSynchronized   m_sync;             ///< Lock to synchronize list operations

public:

    /// @brief List callback function used in each() method.
    ///
    /// Iterates through list until false is returned.
    /// @param item T&, List item
    /// @param data void*, Optional function-specific data
    typedef bool (CallbackFunction)(T& item, void* data);

public:

    /// @brief Default constructor
    CSynchronizedList() :
        m_list(new std::list<T>)
    {}

    /// @brief Destructor
    virtual ~CSynchronizedList()
    {
        delete m_list;
    }

    /// @brief Pushes a data item to the list front
    ///
    /// Automatically posts internal semaphore to indicate
    /// list item availability.
    /// @param data const T&, A data item
    virtual void push_front(const T& data)
    {
        CSynchronizedCode sc(m_sync);
        m_list->push_front(data);
        m_semaphore.post();
    }

    /// @brief Pops a data item from the list front
    ///
    /// If list is empty then waits until timeoutMS milliseconds timeout occurs.
    /// Returns false if timeout occurs.
    /// @param item T&, A list item (output)
    /// @param timeoutMS uint32_t, Operation timeout in milliseconds, default if INFINITE_TIMEOUT
    virtual bool pop_front(T& item, uint32_t timeoutMS=SP_INFINITY)
    {
        if (m_semaphore.wait(timeoutMS)) {
            CSynchronizedCode sc(m_sync);
            if (!m_list->empty()) {
                item = m_list->front();
                m_list->pop_front();
                return true;
            }
        }
        return false;
    }

    /// @brief Pushes a data item to the list back
    ///
    /// Automatically posts internal semaphore to indicate
    /// list item availability.
    /// @param data const T&, A data item
    virtual void push_back(const T& data)
    {
        CSynchronizedCode sc(m_sync);
        m_list->push_back(data);
        m_semaphore.post();
    }

    /// @brief Pops a data item from the list back
    ///
    /// If list is empty then waits until timeoutMS milliseconds timeout occurs.
    /// Returns false if timeout occurs.
    /// @param item T&, A list item (output)
    /// @param timeoutMS uint32_t, Operation timeout in milliseconds, default is INFINITE_TIMEOUT
    virtual bool pop_back(T& item, uint32_t timeoutMS=SP_INFINITY)
    {
        if (m_semaphore.wait(timeoutMS)) {
            CSynchronizedCode sc(m_sync);
            if (!m_list->empty()) {
                item = m_list->back();
                m_list->pop_back();
                return true;
            }
        }
        return false;
    }

    /// @brief Removes all elements with the specific value from the list
    virtual void remove(T& item)
    {
        CSynchronizedCode sc(m_sync);
        m_list->remove(item);
    }

    /// @brief Wakes up list semaphore to interrupt waiting
    ///
    /// Any waiting pop() operation immediately returns false.
    /// @brief Wakes up a sleeping object
    virtual void wakeup()
    {
        m_semaphore.post();
    }

    /// @brief Returns true if the list is empty
    bool empty() const
    {
        CSynchronizedCode sc(m_sync);
        return m_list->empty();
    }

    /// @brief Returns number of items in the list
    uint32_t size() const
    {
        CSynchronizedCode sc(m_sync);
        return m_list->size();
    }

    /// @brief Removes all items from the list
    void clear()
    {
        CSynchronizedCode sc(m_sync);
        m_list->clear();
    }

    /// @brief Calls callbackFunction() for every list until false is returned
    /// @param callbackFunction CallbackFunction*, Callback function that is executed for list items
    /// @param data void*, Function-specific data
    /// @returns true if every list item was processed
    bool each(CallbackFunction* callbackFunction, void* data=NULL)
    {
        CSynchronizedCode sc(m_sync);
        typename std::list<T>::iterator itor;
        for (itor = m_list->begin(); itor != m_list->end(); itor++) {
            if (!callbackFunction(*itor, data))
                return false;
        }
        return true;
    }
};

/// @}

}
#endif
