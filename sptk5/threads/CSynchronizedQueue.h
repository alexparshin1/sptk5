/***************************************************************************
*                          SIMPLY POWERFUL TOOLKIT (SPTK)
*                          CSynchronizedQueue.h  -  description
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

#ifndef __CSYNCHRONIZEDQUEUE_H__
#define __CSYNCHRONIZEDQUEUE_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/threads/CSemaphore.h>
#include <queue>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// @brief Synchronized template queue
///
/// Simple thread-safe queue
template <class T>
class CSynchronizedQueue
{
    CSemaphore              m_semaphore;        ///< Semaphore to waiting for an item if queue is empty
    std::queue<T>*          m_queue;            ///< Queue

protected:

    mutable CSynchronized   m_sync;             ///< Lock to synchronize queue operations

public:

    /// @brief List callback function used in each() method.
    ///
    /// Iterates through queue until false is returned.
    /// @param item T&, List item
    /// @param data void*, Optional function-specific data
    typedef bool (CallbackFunction)(T& item, void* data);

public:

    /// @brief Default constructor
    CSynchronizedQueue() :
        m_queue(new std::queue<T>)
    {}

    /// @brief Destructor
    virtual ~CSynchronizedQueue()
    {
        delete m_queue;
    }

    /// @brief Pushes a data item to the queue
    ///
    /// Automatically posts internal semaphore to indicate
    /// queue item availability.
    /// @param data const T&, A data item
    void push(const T& data)
    {
        CSynchronizedCode sc(m_sync);
        m_queue->push(data);
        m_semaphore.post();
    }

    /// @brief Pops a data item from the queue
    ///
    /// If queue is empty then waits until timeoutMS milliseconds timeout occurs.
    /// Returns false if timeout occurs.
    /// @param item T&, A queue item (output)
    /// @param timeoutMS int32_t, Operation timeout in milliseconds, -1 is forever
    bool pop(T& item, int32_t timeoutMS=-1)
    {
        if (m_semaphore.wait(timeoutMS)) {
            CSynchronizedCode sc(m_sync);
            if (!m_queue->empty()) {
                item = m_queue->front();
                m_queue->pop();
                return true;
            }
        }
        return false;
    }

    /// @brief Wakes up queue semaphore to interrupt waiting
    ///
    /// Any waiting pop() operation immediately returns false.
    /// @brief Wakes up a sleeping object
    virtual void wakeup()
    {
        m_semaphore.post();
    }

    /// @brief Returns true if the queue is empty
    bool empty() const
    {
        CSynchronizedCode sc(m_sync);
        return m_queue->empty();
    }

    /// @brief Returns number of items in the queue
    uint32_t size() const
    {
        CSynchronizedCode sc(m_sync);
        return m_queue->size();
    }

    /// @brief Removes all items from the queue
    void clear()
    {
        CSynchronizedCode sc(m_sync);
        delete m_queue;
        m_queue = new std::queue<T>;
    }

    /// @brief Calls callbackFunction() for every list until false is returned
    ///
    /// Current implementation does the job but isn't too efficient due to
    /// std::queue class limitations.
    /// @param callbackFunction CallbackFunction*, Callback function that is executed for list items
    /// @param data void*, Function-specific data
    /// @returns true if every list item was processed
    bool each(CallbackFunction* callbackFunction, void* data=NULL)
    {
        CSynchronizedCode sc(m_sync);

        std::queue<T> newQueue = new std::queue<T>;

        // Iterating through queue until callback returns false
        bool rc = true;
        while (m_queue->size()) {
            T& item = m_queue->front();
            m_queue->pop();
            newQueue->push(item);
            // When rc switches to false, don't execute callback
            // for the remaining queue items
            if (rc) {
                try {
                    rc = callbackFunction(item, data);
                } catch (std::exception& e) {
                    rc = false;
                }
            }
        }

        delete m_queue;
        m_queue = newQueue;

        return rc;
    }
};
/// @}
}
#endif
