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

#ifndef __SPTK_BUFFER_H__
#define __SPTK_BUFFER_H__

#include <sptk5/BufferStorage.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <memory>

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
class SP_EXPORT Buffer : public BufferStorage
{

public:

    /**
     * Default constructor
     *
     * Creates an empty buffer.
     */
    Buffer() = default;

    /**
     * Constructor
     *
     * Creates an empty buffer.
     * The return of the bytes() method will be 0.
     * @param sz                Buffer size to be pre-allocated
     */
    explicit Buffer(size_t sz);

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
    virtual ~Buffer()
    {
        deallocate();
    }

    /**
     * Appends a single char to the current buffer.
     *
     * Allocates memory if needed.
     * @param ch                Single character
     */
    void append(char ch) override
    {
        BufferStorage::append(ch);
    }

    /**
     * Appends the external data of size sz to the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param sz                Required memory size
     */
    void append(const char* data, size_t sz = 0) override
    {
        BufferStorage::append(data, sz);
    }

    /**
     * Append a value of primitive type or structure to the current buffer.
     *
     * Allocates memory if needed.
     * @param val               Primitive type or structure
     */
    template <class T> void append(T val)
    {
        append((char*)&val, sizeof(val));
    }

    /**
     * Appends the string to the current buffer.
     *
     * Allocates memory if needed.
     * @param str               String to append
     */
    void append(const std::string& str)
    {
        return append(str.c_str(), str.length());
    }

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
     * Access the chars by index
     * @param index             Character index
     */
    char& operator[](size_t index)
    {
        return data()[index];
    }

    /**
     * Access the chars by index, const version
     * @param index             Character index
     */
    const char& operator[](size_t index) const
    {
        return data()[index];
    }

    /**
     * Compare operator
     * @param other             Other buffer
     * @return                  True if buffer contents are identical
     */
    bool operator == (const Buffer& other) const;

    /**
     * Compare operator
     * @param other             Other buffer
     * @return                  True if buffer contents are not matching
     */
    bool operator != (const Buffer& other) const;

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
        return String(data(), bytes());
    }
};

typedef std::shared_ptr<Buffer> SBuffer;

/**
 * Print buffer to ostream as hexadecimal dump
*/
SP_EXPORT std::ostream& operator<<(std::ostream&, const Buffer& buffer);

/**
 * @}
 */
}
#endif
