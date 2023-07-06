/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <cstdlib>
#include <cstring>
#include <sptk5/Exception.h>
#include <sptk5/sptk.h>

namespace sptk {

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
    BufferStorage()
    {
        reallocate(16);
    }

    /**
     * Constructor
     *
     * Creates an empty buffer.
     * The return of the bytes() method will be 0.
     * @param sz                Buffer size to be pre-allocated
     */
    explicit BufferStorage(size_t sz)
    {
        reallocate(sz + 1);
    }

    /**
     * Copy constructor
     * @param bufferStorage     Other object
     */
    BufferStorage(const BufferStorage& bufferStorage)
        : m_size(bufferStorage.m_size)
    {
        reallocate(bufferStorage.size());
        memcpy(m_buffer, bufferStorage.m_buffer, bufferStorage.size());
    }

    /**
     * Move constructor
     * @param bufferStorage     Other object
     */
    BufferStorage(BufferStorage&& bufferStorage) noexcept
        : m_buffer(bufferStorage.m_buffer)
        , m_allocated(bufferStorage.m_allocated)
        , m_size(bufferStorage.m_size)
    {
        bufferStorage.m_buffer = nullptr;
        bufferStorage.m_allocated = 0;
        bufferStorage.m_size = 0;
    }

    /**
     * Destructor
     */
    virtual ~BufferStorage()
    {
        free(m_buffer);
    }

    /**
     * @brief Copy assignment
     * @param other            Other object
     * @return
     */
    BufferStorage& operator=(const BufferStorage& other)
    {
        if (this != &other)
        {
            m_size = other.m_size;
            reallocate(m_size);
            memcpy(m_buffer, other.m_buffer, m_size);
        }
        return *this;
    }

    /**
     * @brief Move assignment
     * @param other            Other object
     * @return
     */
    BufferStorage& operator=(BufferStorage&& bufferStorage) noexcept
    {
        if (this != &bufferStorage)
        {
            free(m_buffer);
            m_buffer = bufferStorage.m_buffer;
            m_allocated = bufferStorage.m_allocated;
            m_size = bufferStorage.m_size;
            bufferStorage.m_buffer = nullptr;
            bufferStorage.m_allocated = 0;
            bufferStorage.m_size = 0;
        }
        return *this;
    }

    /**
     * Constructor
     *
     * Creates a buffer from void *data.
     * The data is copied inside the buffer.
     * The return of the bytes() method will be the input data size.
     * @param data              Data buffer
     * @param sz                Data buffer size
     */
    template<typename T>
    BufferStorage(const T* data, size_t sz)
    {
        allocate((const uint8_t*) data, sz);
    }

    /**
     * Returns pointer on the data buffer.
     */
    uint8_t* data()
    {
        return m_buffer;
    }

    /**
     * Returns pointer on the data buffer.
     */
    [[nodiscard]] const uint8_t* data() const
    {
        return m_buffer;
    }

    /**
     * Returns const char pointer on the data buffer.
     */
    [[nodiscard]] const char* c_str() const
    {
        return (const char*) m_buffer;
    }

    /**
     * Returns true if number of bytes in buffer is zero.
     */
    [[nodiscard]] bool empty() const
    {
        return m_size == 0;
    }

    /**
     * Checks if the current buffer size is enough
     *
     * Allocates memory if needed.
     * @param sz                Required memory size
     */
    virtual void checkSize(size_t sz)
    {
        if (sz >= m_allocated)
        {
            adjustSize(sz);
        }
    }

    /**
     * Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param sz                Required memory size
     */
    template<typename T>
    void set(const T* data, size_t sz)
    {
        _set((const uint8_t*) data, sz);
    }

    /**
     * Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     */
    void set(const BufferStorage& data)
    {
        if (data.m_size == 0)
        {
            m_size = 0;
        }
        else
        {
            _set(data.m_buffer, data.m_size);
        }
    }

    /**
     * Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data
     */
    void set(const String& data)
    {
        _set((const uint8_t*) data.c_str(), data.length());
    }

    /**
     * Returns the size of memory allocated for the data buffer
     * @returns buffer size
     */
    size_t capacity() const
    {
        return m_allocated;
    }

    /**
     * Returns the size of data in the data buffer
     * @returns data size
     */
    size_t size() const
    {
        return m_size;
    }

    /**
     * Returns the size of data in the data buffer
     * @returns data size
     */
    size_t bytes() const
    {
        return m_size;
    }

    /**
     * Sets the size of the data stored
     * @param newSize                 New size of the buffer
     */
    void bytes(size_t newSize)
    {
        if (m_size == newSize)
        {
            return;
        }

        if (newSize >= m_allocated)
        {
            reallocate(newSize);
        }

        m_size = newSize;
        m_buffer[newSize] = 0;
    }

    /**
     * Appends a single char to the current buffer.
     *
     * Allocates memory if needed.
     * @param chr                Single character
     */
    virtual void append(char chr);

    /**
     * Appends the external data of size size to the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param size                Required memory size
     */
    virtual void append(const char* data, size_t size);

    /**
     * Appends the external data of size size to the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param size                Required memory size
     */
    virtual void append(const uint8_t* data, size_t size);

    /**
     * Truncates the current buffer to the size size.
     *
     * Deallocates unused memory if needed.
     * @param size                Required data size in bytes
     */
    void reset(size_t size = 0);

    /**
     * Fills the bytes() characters in buffer with character chr.
     * @param chr                The character to fill the buffer
     * @param count             How many characters are to be filled. If counter is greater than capacity, then buffer is extended.
     */
    void fill(char chr, size_t count);

    /**
     * Remove fragment from buffer's content
     * @param offset            Fragment start offset
     * @param length            Fragment length
     */
    void erase(size_t offset, size_t length);

protected:
    /**
     * Resizes current buffer
     * @param size                Required memory size
     */
    void adjustSize(size_t size);

    /**
     * Allocate memory
     * @param size              Number of bytes for new buffer
     */
    void allocate(const uint8_t* data, size_t size)
    {
        reallocate(size);
        m_size = size;
        if (data != nullptr && size != 0)
        {
            memcpy(m_buffer, data, size);
        }
        m_buffer[size] = 0;
    }

    /**
     * Reallocate memory
     * @param size              Number of bytes for new buffer
     */
    void reallocate(size_t size)
    {
        auto* newBuffer = static_cast<uint8_t*>(realloc(m_buffer, size + 1));
        if (newBuffer == nullptr)
        {
            throw Exception("Not enough memory");
        }
        m_buffer = newBuffer;
        if (m_size > size)
        {
            m_size = size;
        }
        m_buffer[size] = 0;
        m_allocated = size;
    }

    void init(const uint8_t* data, size_t size, size_t bytes)
    {
        allocate(data, size);
        m_size = bytes;
    }

private:
    uint8_t* m_buffer {nullptr}; ///< Actual storage
    size_t m_allocated {0};      ///< Alocated size
    size_t m_size {0};           ///< Actual size of the data in buffer


    /**
     * Copies the external data of size size into the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param size                Required memory size
     */
    void _set(const uint8_t* data, size_t size);
};

/**
 * @}
 */
} // namespace sptk
