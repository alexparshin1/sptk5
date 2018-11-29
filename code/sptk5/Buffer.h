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
 * Memory data buffer
 *
 * Generic buffer with a special memory-allocation strategy for effective append() operation
 */
class SP_EXPORT Buffer
{
    /**
     * Actual storage
     */
    std::vector<char>   m_storage;

    /**
     * Actual size of the data in buffer
     */
    size_t              m_bytes {0};

    /**
     * The pointer to beginning of the storage
     */
    char*               m_buffer;

    /**
     * Resizes current buffer
     * @param sz                Required memory size
     */
    void adjustSize(size_t sz);

public:

    /**
     * Default constructor
     *
     * Creates an empty buffer.
     * The return of the bytes() method will be 0.
     * @param sz                Buffer size to be pre-allocated
     */
    explicit Buffer(size_t sz = 16);

    /**
     * Constructor
     *
     * Creates a buffer from string.
     * The string is copied inside the buffer.
     * The return of the bytes() method will be the input string length.
     * @param str               Input string
     */
    explicit Buffer(const char* str);

    /**
     * Constructor
     *
     * Creates a buffer from string.
     * The string is copied inside the buffer.
     * The return of the bytes() method will be the input string length.
     * @param str               Input string
     */
    explicit Buffer(const String& str);

    /**
     * Constructor
     *
     * Creates a buffer from void *data.
     * The data is copied inside the buffer.
     * The return of the bytes() method will be the input data size.
     * @param data              Data buffer
     * @param sz                Data buffer size
     */
    Buffer(const void* data, size_t sz);

    /**
     * Copy constructor
     *
     * Creates a buffer from another buffer.
     * @param other             Data buffer
     */
    Buffer(const Buffer& other);

    /**
     * Move constructor
     *
     * Moves a buffer from another buffer.
     * @param other             Data buffer
     */
    Buffer(Buffer&& other) noexcept;

    /**
     * Destructor
     */
    virtual ~Buffer() = default;

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
        if (sz >= m_storage.size())
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
    void set(const Buffer& data)
    {
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
     * Appends a single char to the current buffer.
     *
     * Allocates memory if needed.
     * @param ch                Single character
     */
    void append(char ch);

    /**
     * Append an integer to the current buffer.
     *
     * Allocates memory if needed.
     * @param val               Short integer
     */
    void append(uint16_t val);

    /**
     * Appends the external data of size sz to the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param sz                Required memory size
     */
    void append(const char* data, size_t sz = 0);

    /**
     * Appends the string to the current buffer.
     *
     * Allocates memory if needed.
     * @param str               String to append
     */
    void append(const String& str)
    {
        return append(str.c_str(), str.length());
    }

    /**
     * Appends the string to the current buffer.
     *
     * Allocates memory if needed.
     * @param buffer            Data to append
     */
    void append(const Buffer& buffer)
    {
        return append(buffer.data(), buffer.bytes());
    }

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
     * Returns the size of memory allocated for the data buffer
     * @returns buffer size
     */
    size_t capacity()  const
    {
        return m_storage.size();
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
        if (b < m_storage.size()) {
            m_bytes = b;
            m_storage[b] = 0;
            return;
        }
        throw Exception("Attempt to set buffer size outside storage");
    }

    /**
     * Access the chars by index
     * @param index             Character index
     */
    char& operator[](size_t index)
    {
        return m_buffer[index];
    }

    /**
     * Access the chars by index, const version
     * @param index             Character index
     */
    const char& operator[](size_t index) const
    {
        return m_buffer[index];
    }

    /**
     * Loads the buffer from file fileName.
     * @param fileName          Name of the input file
     */
    void loadFromFile(const String& fileName);

    /**
     * Saves the buffer to the file fileName.
     * @param fileName          Name of the output file
     */
    void saveToFile(const String& fileName) const;

    /**
     * Moves buffer from another buffer
     * @param other             Buffer to move from
     * @returns this object
     */
    Buffer& operator = (Buffer&& other) DOESNT_THROW;

    /**
     * Assigns from Buffer
     * @param other             Buffer to assign from
     * @returns this object
     */
    Buffer& operator = (const Buffer& other);

    /**
     * Assigns from String
     * @param str               String to assign from
     * @returns this object
     */
    Buffer& operator = (const String& str);

    /**
     * Assigns from char *
     * @param str const char *, the string to assign from
     * @returns this object
     */
    Buffer& operator = (const char* str);

    /**
     * Convertor to std::string
     */
    explicit operator String() const
    {
        return String(m_buffer, m_bytes);
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
