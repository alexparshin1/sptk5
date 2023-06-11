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

#include <sptk5/BufferStorage.h>
#include <sptk5/VariantStorageClient.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace sptk {

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
    : public BufferStorage
    , public VariantStorageClient
{

public:
    /**
     * Constructor
     * @param size              Pre-allocated buffer size
     */
    explicit Buffer(size_t size = 16)
        : BufferStorage(size)
    {
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
    Buffer(const T* data, size_t sz)
        : BufferStorage(data, sz)
    {
    }

    /**
     * Constructor
     *
     * Creates a buffer from string.
     * The string is copied inside the buffer.
     * The return of the bytes() method will be the input string length.
     * @param str               Input string
     */
    Buffer(std::string_view str);

    /**
     * Copy constructor
     *
     * Creates a buffer from another buffer.
     * @param other             Data buffer
     */
    Buffer(const Buffer& other) = default;

    /**
     * Move constructor
     *
     * Moves a buffer from another buffer.
     * @param other             Data buffer
     */
    Buffer(Buffer&& other) noexcept = default;

    /**
     * Destructor
     */
    ~Buffer() noexcept override = default;

    /**
     * Moves buffer from another buffer
     * @param other             Buffer to move from
     * @returns this object
     */
    Buffer& operator=(Buffer&& other) noexcept = default;

    /**
     * Assigns from Buffer
     * @param other             Buffer to assign from
     * @returns this object
     */
    Buffer& operator=(const Buffer& other) = default;

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
     * Appends the external data of size sz to the current buffer.
     *
     * Allocates memory if needed.
     * @param data              External data buffer
     * @param sz                Required memory size
     */
    void append(const uint8_t* data, size_t sz) override
    {
        BufferStorage::append(data, sz);
    }

    /**
     * Append a value of primitive type or structure to the current buffer.
     *
     * Allocates memory if needed.
     * @param val               Primitive type or structure
     */
    template<class T>
    void append(T val)
    {
        append((char*) &val, sizeof(val));
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
    uint8_t& operator[](size_t index)
    {
        return data()[index];
    }

    /**
     * Access the chars by index, const version
     * @param index             Character index
     */
    const uint8_t& operator[](size_t index) const
    {
        return data()[index];
    }

    /**
     * Compare operator
     * @param other             Other buffer
     * @return                  True if buffer contents are identical
     */
    bool operator==(const Buffer& other) const;

    /**
     * Loads the buffer from file fileName.
     * @param fileName          Name of the input file
     */
    void loadFromFile(const std::filesystem::path& fileName);

    /**
     * Saves the buffer to the file fileName.
     * @param fileName          Name of the output file
     */
    void saveToFile(const std::filesystem::path& fileName) const;

    /**
     * Assigns from String
     * @param str               String to assign from
     * @returns this object
     */
    Buffer& operator=(const String& str);

    /**
     * Assigns from char *
     * @param str const char *, the string to assign from
     * @returns this object
     */
    Buffer& operator=(const char* str);

    /**
     * Convertor to std::string
     */
    explicit operator String() const
    {
        return {(const char*) data(), bytes()};
    }

    static VariantDataType variantDataType()
    {
        return VariantDataType::VAR_BUFFER;
    }

    [[nodiscard]] size_t dataSize() const override
    {
        return size();
    }
};

/**
 * Print buffer to ostream as hexadecimal dump
*/
SP_EXPORT std::ostream& operator<<(std::ostream&, const Buffer& buffer);

/**
 * @}
 */
} // namespace sptk
