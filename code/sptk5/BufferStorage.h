/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_BUFFER_STORAGE_H__
#define __SPTK_BUFFER_STORAGE_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <string.h>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Memory data buffer storage
 */
class SP_EXPORT BufferStorage
{
public:

    /**
     * Default constructor
     *
     * Creates an empty buffer.
     */
    BufferStorage() = default;

    /**
     * Copy constructor
     *
     * Creates an empty buffer.
     */
    BufferStorage(const BufferStorage& other);

    /**
     * Move constructor
     *
     * Creates an empty buffer.
     */
    BufferStorage(BufferStorage&& other) noexcept;

    /**
     * Copy assignment
     *
     * Creates an empty buffer.
     */
    BufferStorage& operator = (const BufferStorage& other);

    /**
     * Move assignment
     *
     * Creates an empty buffer.
     */
    BufferStorage& operator = (BufferStorage&& other) noexcept;

    /**
     * Constructor
     *
     * Creates an empty buffer.
     * The return of the bytes() method will be 0.
     * @param sz                Buffer size to be pre-allocated
     */
    explicit BufferStorage(size_t sz);

    /**
     * Constructor
     *
     * Creates a buffer from void *data.
     * The data is copied inside the buffer.
     * The return of the bytes() method will be the input data size.
     * @param data              Data buffer
     * @param sz                Data buffer size
     */
    BufferStorage(const void* data, size_t sz);

    /**
     * Destructor
     */
    virtual ~BufferStorage() noexcept
    {
        deallocate();
    }

    /**
     * Returns pointer on the data buffer.
     */
    char* data() const
    {
        return m_buffer;
    }

    /**
     * Returns const char pointer on the data buffer.
     */
    const char* c_str() const
    {
        return m_buffer;
    }

    /**
     * Returns true if number of bytes in buffer is zero.
     */
    bool empty() const
    {
        return m_bytes == 0;
    }

    /**
     * Checks if the current buffer size is enough
     *
     * Allocates memory if needed.
     * @param sz                Required memory size
     */
    virtual void checkSize(size_t sz)
    {
        if (sz >= m_size)
            adjustSize(sz);
    }

    /**
     * Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param sz                Required memory size
     */
    void set(const char* data, size_t sz);

    /**
     * Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     */
    void set(const BufferStorage& data)
    {
        if (data.m_bytes == 0)
            m_bytes = 0;
        else
            set(data.m_buffer, data.m_bytes);
    }

    /**
     * Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data
     */
    void set(const String& data)
    {
        set(data.c_str(), data.length());
    }

    /**
     * Returns the size of memory allocated for the data buffer
     * @returns buffer size
     */
    size_t capacity()  const
    {
        return m_size;
    }

    /**
     * Returns the size of data in the data buffer
     * @returns data size
     */
    size_t length() const
    {
        return m_bytes;
    }

    /**
     * Returns the size of data in the data buffer
     * @returns data size
     */
    size_t bytes() const
    {
        return m_bytes;
    }

    /**
     * Sets the size of the data stored
     *
     * Doesn't check anything so use it this caution.
     * @param b                 New size of the buffer
     */
    void bytes(size_t b)
    {
        if (m_bytes == b)
            return;
        if (b < m_size) {
            m_bytes = b;
            m_buffer[b] = 0;
            return;
        }
        throw Exception("Attempt to set buffer size outside storage");
    }

    /**
     * Appends a single char to the current buffer.
     *
     * Allocates memory if needed.
     * @param ch                Single character
     */
    virtual void append(char ch);

    /**
     * Appends the external data of size sz to the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param sz                Required memory size
     */
    virtual void append(const char* data, size_t sz = 0);

    /**
     * Truncates the current buffer to the size sz.
     *
     * Deallocates unused memory if needed.
     * @param sz                Required data size in bytes
     */
    void reset(size_t sz = 0);

    /**
     * Fills the bytes() characters in buffer with character ch.
     * @param ch                The character to fill the buffer
     * @param count             How many characters are to be filled. If counter is greater than capacity, then buffer is extended.
     */
    void fill(char ch, size_t count);

    /**
     * Remove fragment from buffer's content
     * @param offset            Fragment start offset
     * @param length            Fragment length
     */
    void erase(size_t offset, size_t length);

protected:

    /**
     * Resizes current buffer
     * @param sz                Required memory size
     */
    void adjustSize(size_t sz);

    /**
     * Allocate memory
     * @param size              Number of bytes for new buffer
     */
    void allocate(size_t size)
    {
        m_bytes = 0;
        m_size = size;
        m_buffer = new char[m_size];
        memset(m_buffer, 0, size);
    }

    /**
     * Allocate memory
     * @param size              Number of bytes for new buffer
     */
    void allocate(const void* data, size_t size)
    {
        m_bytes = size;
        m_size = m_bytes + 1;
        m_buffer = new char[m_size];
        if (data != nullptr && size != 0)
            memcpy(m_buffer, data, size);
        m_buffer[size] = 0;
    }

    /**
     * Reallocate memory
     * @param size              Number of bytes for new buffer
     */
    void reallocate(size_t size)
    {
        size_t newSize = size + 1;
        if (newSize == m_size)
            return;

        auto* ptr = new char[newSize];
        if (ptr == nullptr)
            throwException("Out of memory")

        if (m_size < size && size > 0)
            memcpy(ptr, m_buffer, m_size);
        else
            memcpy(ptr, m_buffer, size);

        delete [] m_buffer;

        m_buffer = ptr;
        m_size = newSize;
        if (m_bytes > size)
            m_bytes = size;

        m_buffer[size] = 0;
    }

    /**
     * Free memory
     */
    void deallocate() noexcept
    {
        delete [] m_buffer;
        m_buffer = nullptr;
        m_bytes = 0;
        m_size = 0;
    }

    void init(char* data, size_t size, size_t bytes)
    {
        m_buffer = data;
        m_size = size;
        m_bytes = bytes;
    }

private:

    char*       m_buffer {nullptr};     ///< Actual storage
    size_t      m_size {0};             ///< Alocated storage size
    size_t      m_bytes {0};            ///< Actual size of the data in buffer
};

/**
 * @}
 */
}
#endif
