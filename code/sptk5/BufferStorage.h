/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#ifndef _WIN32
#include <bit>
#endif

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
    static constexpr size_t MAX_SIZE_T = static_cast<size_t>(-1);

    /**
     * Default constructor
     *
     * Creates an empty buffer.
     */
    BufferStorage()
    {
        constexpr size_t defaultSize = 16;
        reallocate(defaultSize);
        m_buffer[0] = 0;
    }

    /**
     * Constructor
     *
     * Creates an empty buffer.
     * The return of the bytes() method will be 0.
     * @param sz                Buffer size to be pre-allocated
     */
    explicit BufferStorage(const size_t sz)
    {
        if (sz)
        {
            reallocate(sz + 1);
            m_buffer[0] = 0;
        }
    }

    /**
     * Copy constructor
     * @param bufferStorage     The other object
     */
    BufferStorage(const BufferStorage& bufferStorage)
        : m_size(bufferStorage.m_size)
    {
        reallocate(bufferStorage.size());
        if (m_buffer != nullptr)
        {
            memcpy(m_buffer, bufferStorage.m_buffer, bufferStorage.size());
        }
    }

    /**
     * Move constructor
     * @param bufferStorage     The other object
     */
    BufferStorage(BufferStorage&& bufferStorage) noexcept
        : m_buffer(bufferStorage.m_buffer)
        , m_allocated(bufferStorage.m_allocated)
        , m_size(bufferStorage.m_size)
    {
        bufferStorage.m_buffer = static_cast<uint8_t*>(malloc(1));
        bufferStorage.m_buffer[0] = 0;
        bufferStorage.m_allocated = 1;
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
     * @param other            The other object
     * @return
     */
    BufferStorage& operator=(const BufferStorage& other)
    {
        if (this != &other)
        {
            if (other.m_size > m_allocated)
            {
                reallocate(other.m_size * 3 / 2);
            }
            m_size = other.m_size;
            memcpy(m_buffer, other.m_buffer, m_size);
            m_buffer[m_size] = 0;
        }
        return *this;
    }

    /**
     * @brief Move assignment
     * @param bufferStorage     The other object
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
    BufferStorage(const T* data, const size_t sz)
    {
        allocate(std::bit_cast<const uint8_t*>(data), sz);
    }

    /**
     * Returns the pointer to the data buffer.
     */
    [[nodiscard]] uint8_t* data()
    {
        return m_buffer;
    }

    /**
     * Returns the pointer to the data buffer.
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
        return reinterpret_cast<const char*>(m_buffer);
    }

    /**
     * Returns true if the number of bytes in the buffer is zero.
     */
    [[nodiscard]] virtual bool empty() const
    {
        return m_size == 0;
    }

    /**
     * Checks if the current buffer size is enough
     *
     * Allocates memory if needed.
     * @param sz                Required memory size
     */
    void checkSize(const size_t sz)
    {
        if (sz >= m_allocated) [[unlikely]]
        {
            adjustSize(sz);
        }
    }

    /**
     * @brief Checks if the current buffer size is enough.
     * Synonym for checkSize().
     * Allocates memory if needed.
     * @param sz                Required memory size
     */
    void reserve(const size_t sz)
    {
        if (sz >= m_allocated) [[unlikely]]
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
    void set(const T* data, const size_t sz)
    {
        _set(std::bit_cast<const uint8_t*>(data), sz);
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
    void set(const std::string& data)
    {
        _set(std::bit_cast<const uint8_t*>(data.c_str()), data.length());
    }

    /**
     * Returns the size of memory allocated for the data buffer
     * @returns buffer size
     */
    [[nodiscard]] size_t capacity() const
    {
        return m_allocated;
    }

    /**
     * Returns the size of data in the data buffer
     * @returns data size
     */
    [[nodiscard]] size_t size() const
    {
        return m_size;
    }

    /**
     * Returns the size of data in the data buffer
     * @returns data size
     */
    [[nodiscard]] virtual size_t bytes() const
    {
        return m_size;
    }

    /**
     * Sets the size of the data stored
     * @param newSize                 New size of the buffer
     */
    virtual void bytes(const size_t newSize)
    {
        if (m_size == newSize)
        {
            return;
        }

        if (newSize > m_allocated)
        {
            reallocate(newSize * 3 / 2);
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
    void append(char chr);

    /**
     * @brief Appends the external data of the size to the current buffer.
     * @param str               Null-terminated string.
     */
    void append(const char* str)
    {
        append(str, strlen(str));
    }

    /**
     * Appends the external data of the size to the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param size              Required memory size in bytes
     */
    template<class T>
        requires std::is_integral_v<T>
    void append(const T* data, const size_t size)
    {
        if (data == nullptr || size == 0)
        {
            return;
        }
        checkSize(m_size + size + 1);
        memcpy(m_buffer + m_size, data, size);
        m_size += size;
        m_buffer[m_size] = 0;
    }

    /**
     * Append a value of the primitive type or structure to the current buffer.
     *
     * Allocates memory if needed.
     * @param val               Primitive type or structure
     */
    template<class T>
        requires std::is_integral_v<T>
    void append(T val)
    {
        append(reinterpret_cast<uint8_t*>(&val), sizeof(val));
    }

    /**
     * Appends the string to the current buffer.
     *
     * Allocates memory if needed.
     * @param str               String to append
     */
    template<class T>
        requires std::is_class_v<T>
    void append(const T& str)
    {
        append(str.c_str(), str.size());
    }

    /**
     * Appends the string to the current buffer.
     *
     * Allocates memory if needed.
     * @param buffer            Data to append
     */
    void append(const BufferStorage& buffer)
    {
        append(buffer.data(), buffer.bytes());
    }

    /**
     * @brief Appends a formatted string to the buffer using std::format-style arguments.
     * @param maxLength         Maximum size of the appended text.
     * @param fmt               Compile-time checked format string
     * @param args              Format arguments
     * @return                  Number of characters appended
     */
    template<typename... Args>
    size_t append(const size_t maxLength, std::format_string<Args...> fmt, Args&&... args)
    {
        checkSize(size() + maxLength);
        const std::format_to_n_result result = std::format_to_n(data() + size(), maxLength, fmt, std::forward<Args>(args)...);
        *result.out = '\0';
        const auto written = std::min(static_cast<size_t>(result.size), maxLength);
        bytes(size() + written);
        return written;
    }

    /**
     * @brief Truncates the current buffer to the size.
     * @param size                Required data size in bytes.
     */
    virtual void reset(size_t size = 0);

    /**
     * Fills the bytes() characters in the buffer with character chr.
     * @param chr               The character to fill the buffer
     * @param count             Number of characters to fill. If the counter is greater than capacity, then the buffer is extended.
     */
    void fill(char chr, size_t count);

    /**
     * Remove fragment from the buffer's content
     * @param offset            Fragment start offset
     * @param length            Fragment length
     */
    virtual void erase(size_t offset, size_t length);

    /**
     * Resizes current buffer
     * @param size                Required memory size
     */
    void adjustSize(const size_t size)
    {
        if (size > m_allocated)
        {
            reallocate(size * 2);
        }
    }

protected:
    /**
     * Allocate memory
     * @param data              Data to copy in
     * @param size              Number of bytes for the new buffer
     */
    void allocate(const uint8_t* data, const size_t size)
    {
        if (m_allocated < size + 1)
        {
            reallocate(size);
        }
        m_size = size;
        if (data != nullptr && size != 0)
        {
            memcpy(m_buffer, data, size);
        }
        m_buffer[size] = 0;
    }

    /**
     * Reallocate memory
     * @param size              Number of bytes for the new buffer
     */
    void reallocate(const size_t size)
    {
        auto* newBuffer = malloc(size + 1);
        if (newBuffer == nullptr) [[unlikely]]
        {
            throwNotEnoughMemory();
        }

        if (m_buffer != nullptr)
        {
            const auto copySize = std::min(size, m_size);
            memcpy(newBuffer, m_buffer, copySize);
            free(m_buffer);
        }
        m_buffer = std::bit_cast<uint8_t*>(newBuffer);
        if (m_size > size)
        {
            m_size = size;
        }
        m_buffer[size] = 0;
        m_allocated = size;
    }

    void init(const uint8_t* data, const size_t size, const size_t bytes)
    {
        allocate(data, size);
        m_size = bytes;
    }

    /**
     * @brief Swap buffers.
     * @param other Another buffer.
     */
    void swapInternal(BufferStorage& other);

private:
    uint8_t* m_buffer {nullptr}; ///< Actual storage
    size_t   m_allocated {0};    ///< Alocated size
    size_t   m_size {0};         ///< Actual size of the data in the buffer

    /**
     * @brief Out-of-line cold path: throw on allocation failure.
     *
     * Kept out of line so the inline reallocate() hot path stays small.
     */
    [[noreturn]] static void throwNotEnoughMemory();


    /**
     * Copies the external data of the size into the current buffer.
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
