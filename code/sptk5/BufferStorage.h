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

#include <sptk5/Exception.h>
#include <sptk5/sptk.h>
#include <string.h>

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
        : m_buffer(16)
    {
    }

    /**
     * Constructor
     *
     * Creates an empty buffer.
     * The return of the bytes() method will be 0.
     * @param sz                Buffer size to be pre-allocated
     */
    explicit BufferStorage(size_t sz)
        : m_buffer(sz + 1)
    {
    }

    /**
     * Copy constructor
     * @param bufferStorage     Other object
     */
    BufferStorage(const BufferStorage& bufferStorage) = default;

    /**
     * Move constructor
     * @param bufferStorage     Other object
     */
    BufferStorage(BufferStorage&& bufferStorage) noexcept = default;

    /**
     * Destructor
     */
    virtual ~BufferStorage() = default;

    BufferStorage& operator=(const BufferStorage& bufferStorage) = default;
    BufferStorage& operator=(BufferStorage&& bufferStorage) noexcept = default;

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
        return m_buffer.data();
    }

    /**
     * Returns pointer on the data buffer.
     */
    const uint8_t* data() const
    {
        return m_buffer.data();
    }

    /**
     * Returns const char pointer on the data buffer.
     */
    const char* c_str() const
    {
        return (const char*) m_buffer.data();
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
        if (sz + 1 >= m_buffer.size())
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
        if (data.m_bytes == 0)
        {
            m_bytes = 0;
        }
        else
        {
            _set(data.m_buffer.data(), data.m_bytes);
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
        return m_buffer.size() - 1;
    }

    /**
     * Returns the size of data in the data buffer
     * @returns data size
     */
    size_t size() const
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
     * @param b                 New size of the buffer
     */
    void bytes(size_t b)
    {
        if (m_bytes == b)
        {
            return;
        }

        if (b + 1 > m_buffer.size())
        {
            m_buffer.resize(b + 1);
        }

        m_bytes = b;
        m_buffer[b] = 0;
        return;
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
    void allocate(size_t size)
    {
        m_buffer.resize(size + 1);
        m_bytes = 0;
        m_buffer[size] = 0;
    }

    /**
     * Allocate memory
     * @param size              Number of bytes for new buffer
     */
    void allocate(const uint8_t* data, size_t size)
    {
        m_buffer.resize(size + 1);
        m_bytes = size;
        if (data != nullptr && size != 0)
        {
            memcpy(m_buffer.data(), data, size);
        }
        m_buffer[size] = 0;
    }

    /**
     * Reallocate memory
     * @param size              Number of bytes for new buffer
     */
    void reallocate(size_t size)
    {
        m_buffer.resize(size + 1);
        if (m_bytes > size)
        {
            m_bytes = size;
        }
        m_buffer[size] = 0;
    }

    /**
     * Free memory
     */
    void deallocate() noexcept
    {
        m_buffer.resize(1);
        m_buffer[0] = 0;
        m_bytes = 0;
    }

    void init(const uint8_t* data, size_t size, size_t bytes)
    {
        allocate(data, size);
        m_bytes = bytes;
    }

private:
    std::vector<uint8_t> m_buffer; ///< Actual storage
    size_t m_bytes {0};            ///< Actual size of the data in buffer


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
