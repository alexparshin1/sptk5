/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Buffer.h - description                                 ║
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

#ifndef __SPTK_BUFFER_H__
#define __SPTK_BUFFER_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>

#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Memory data buffer
 *
 * Generic buffer with a special memory-allocation strategy for effective append() operation
 */
class SP_EXPORT Buffer
{

    /**
     * @brief Resizes current buffer
     * @param sz size_t, required memory size
     */
    void adjustSize(size_t sz);

protected:

    /**
     * Allocated size of the buffer
     */
    size_t    m_capacity {0};

    /**
     * Actual size of the data in buffer
     */
    size_t    m_bytes {0};

    /**
     * The buffer itself
     */
    char*       m_buffer { nullptr };


public:

    /**
     * @brief Default constructor
     *
     * Creates an empty buffer.
     * The return of the bytes() method will be 0.
     * @param sz size_t, buffer size to be pre-allocated
     */
    explicit Buffer(size_t sz = 16);

    /**
     * @brief Constructor
     *
     * Creates a buffer from string.
     * The string is copied inside the buffer.
     * The return of the bytes() method will be the input string length.
     * @param str const char *, input string
     */
    explicit Buffer(const char* str);

    /**
     * @brief Constructor
     *
     * Creates a buffer from string.
     * The string is copied inside the buffer.
     * The return of the bytes() method will be the input string length.
     * @param str const std::string&, input string
     */
    explicit Buffer(const std::string& str);

    /**
     * @brief Constructor
     *
     * Creates a buffer from string.
     * The string is copied inside the buffer.
     * The return of the bytes() method will be the input string length.
     * @param str const std::string&, input string
     */
    explicit Buffer(const String& str);

    /**
     * @brief Constructor
     *
     * Creates a buffer from void *data.
     * The data is copied inside the buffer.
     * The return of the bytes() method will be the input data size.
     * @param data void * data buffer
     * @param sz size_t, data buffer size
     */
    Buffer(const void* data, size_t sz);

    /**
     * @brief Copy constructor
     *
     * Creates a buffer from another buffer.
     * @param other const Buffer&, data buffer
     */
    Buffer(const Buffer& other);

    /**
     * @brief Move constructor
     *
     * Moves a buffer from another buffer.
     * @param other Buffer&&, data buffer
     */
    Buffer(Buffer&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~Buffer()
    {
        if (m_buffer)
            free(m_buffer);
    }

    /**
     * @brief Returns pointer on the data buffer.
     */
    char* data() const
    {
        return m_buffer;
    }

    /**
     * @brief Returns const char pointer on the data buffer.
     */
    const char* c_str() const
    {
        return m_buffer;
    }

    /**
     * @brief Returns true if number of bytes in buffer is zero.
     */
    bool empty() const
    {
        return m_bytes == 0;
    }

    /**
     * @brief Checks if the current buffer size is enough
     *
     * Allocates memory if needed.
     * @param sz size_t, required memory size
     */
    void checkSize(size_t sz)
    {
        if (sz > m_capacity)
            adjustSize(sz);
    }

    /**
     * @brief Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data const char*, external data buffer
     * @param sz size_t, required memory size
     */
    void set(const char* data, size_t sz);

    /**
     * @brief Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data const Buffer&, external data buffer
     */
    void set(const Buffer& data)
    {
        set(data.m_buffer, data.m_bytes);
    }

    /**
     * @brief Copies the external data of size sz into the current buffer.
     *
     * Allocates memory if needed.
     * @param data const std::string&, external data
     */
    void set(const std::string& data)
    {
        set(data.c_str(), data.length());
    }

    /**
     * @brief Appends a single char to the current buffer.
     *
     * Allocates memory if needed.
     * @param ch single character
     */
    void append(char ch);

    /**
     * Append an integer to the current buffer.
     *
     * Allocates memory if needed.
     * @param val uint8_t, Short integer
     */
    void append(uint8_t val);

    /**
     * Append an integer to the current buffer.
     *
     * Allocates memory if needed.
     * @param val uint16_t, Short integer
     */
    void append(uint16_t val);

    /**
     * @brief Appends a single char to the current buffer.
     *
     * Allocated memory isn't checked. Application should call checkSize() to make sure the required size is there
     * @param ch single character
     */
    void appendNoCheck(char ch);

    /**
     * @brief Appends the external data of size sz to the current buffer.
     *
     * Allocates memory if needed.
     * @param data const char *, external data buffer
     * @param sz size_t, required memory size
     */
    void append(const char* data, size_t sz = 0);

    /**
     * @brief Appends the external data of size sz to the current buffer.
     *
     * Allocated memory isn't checked. Application should call checkSize() to make sure the required size is there
     * @param data const char *, external data buffer
     * @param sz size_t, required memory size
     */
    void appendNoCheck(const char* data, size_t sz = 0);

    /**
     * @brief Appends the string to the current buffer.
     *
     * Allocates memory if needed.
     * @param str std::string, string to append
     */
    void append(const std::string& str)
    {
        return append(str.c_str(), str.length());
    }

    /**
     * @brief Appends the string to the current buffer.
     *
     * Allocates memory if needed.
     * @param buffer const Buffer&, data to append
     */
    void append(const Buffer& buffer)
    {
        return append(buffer.data(), buffer.bytes());
    }

    /**
     * @brief Truncates the current buffer to the size sz.
     *
     * Deallocates unused memory if needed.
     * @param sz size_t, the required data size in bytes
     */
    void reset(size_t sz = 0);

    /**
     * @brief Fills the bytes() characters in buffer with character ch.
     * @param ch                The character to fill the buffer
     * @param count             How many characters are to be filled. If counter is greater than capacity, then buffer is extended.
     */
    void fill(char ch, size_t count);

    /**
     * @brief Returns the size of memory allocated for the data buffer
     * @returns buffer size
     */
    size_t capacity()  const
    {
        return m_capacity;
    }

    /**
     * @brief Returns the size of data in the data buffer
     * @returns data size
     */
    size_t bytes() const
    {
        return m_bytes;
    }

    /**
     * @brief Returns the size of data in the data buffer
     * @returns data size
     */
    size_t length() const
    {
        return m_bytes;
    }

    /**
     * @brief Sets the size of the data stored
     *
     * Doesn't check anything so use it this caution.
     * @param b size_t, the new size of the buffer
     */
    void bytes(size_t b)
    {
        m_bytes = b;
    }

    /**
     * @brief Access the chars by index
     * @param index size_t, character index
     */
    char& operator[](size_t index)
    {
        return m_buffer[index];
    }

    /**
     * @brief Access the chars by index, const version
     * @param index size_t, character index
     */
    const char& operator[](size_t index) const
    {
        return m_buffer[index];
    }

    /**
     * @brief Loads the buffer from file fileName.
     * @param fileName std::string, the name of the input file
     */
    void loadFromFile(const String& fileName);

    /**
     * @brief Saves the buffer to the file fileName.
     * @param fileName std::string, the name of the output file
     */
    void saveToFile(const String& fileName) const;

    /**
     * @brief Moves buffer from another buffer
     * @param b Buffer&&, the buffer to move from
     * @returns this object
     */
    Buffer& operator = (Buffer&& b) DOESNT_THROW;

    /**
     * @brief Assigns from Buffer
     * @param b const Buffer&, the buffer to assign from
     * @returns this object
     */
    Buffer& operator = (const Buffer& b);

    /**
     * @brief Assigns from String
     * @param str const std::string&, the string to assign from
     * @returns this object
     */
    Buffer& operator = (const std::string& str);

    /**
     * @brief Assigns from char *
     * @param str const char *, the string to assign from
     * @returns this object
     */
    Buffer& operator = (const char* str);

    /**
     * @brief Appends another Buffer
     * @param b const Buffer&, the buffer to append
     * @returns this object
     */
    Buffer& operator += (const Buffer& b)
    {
        append(b);
        return *this;
    }

    /**
     * @brief Appends String
     * @param str const std::string&, the string to append
     * @returns this object
     */
    Buffer& operator += (const std::string& str)
    {
        append(str);
        return *this;
    }

    /**
     * @brief Appends const char *
     * @param str const char *, the string to append
     * @returns this object
     */
    Buffer& operator += (const char* str)
    {
        append(str);
        return *this;
    }

    /**
     * @brief Convertor to std::string
     */
    operator std::string() const
    {
        return std::string(m_buffer, m_bytes);
    }
};

/**
 * Print buffer to ostream as hexadecimal dump
  */
  std::ostream& operator<<(std::ostream&, const Buffer& buffer);

/**
 * @}
 */
}
#endif
