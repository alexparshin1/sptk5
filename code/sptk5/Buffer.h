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

#include <cstdarg>
#include <cstdio>
#include <format>
#include <sptk5/BufferStorage.h>
#include <sptk5/VariantStorageClient.h>

#include <memory>

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
    static constexpr size_t defaultBufferSize = 16;

    /**
     * Constructor
     * @param size              Pre-allocated buffer size
     */
    explicit Buffer(const size_t size = defaultBufferSize)
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
     * @param bufferSize                Data buffer size
     */
    template<typename T>
    Buffer(const T* data, size_t bufferSize)
        : BufferStorage(data, bufferSize)
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
     * Access the chars by index
     * @param index             Character index
     */
    uint8_t& operator[](const size_t index)
    {
        return data()[index];
    }

    /**
     * Access the chars by index, const version
     * @param index             Character index
     */
    const uint8_t& operator[](const size_t index) const
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
     * Convertor to string.
     */
    explicit operator String() const
    {
        return {c_str(), bytes()};
    }

    /**
     * Convertor to std::string_view.
     */
    explicit operator std::string_view() const
    {
        return {c_str(), bytes()};
    }

    static VariantDataType variantDataType()
    {
        return VariantDataType::VAR_BUFFER;
    }

    [[nodiscard]] size_t dataSize() const override
    {
        return size();
    }

    /**
     * @brief Append formatted data to buffer.
     * @param maxLength         The maximum number of chars to append to buffer.
     * @param format            Format string, printf-style.
     * @param ...               Arguments for format string.
     * @return the actual number of chars appended to buffer.
     */
    size_t printf(const size_t maxLength, const char* format, ...)
    {
        checkSize(size() + maxLength);
        va_list args;
        va_start(args, format);
        const auto written = vsnprintf(reinterpret_cast<char*>(data() + size()), maxLength + 1, format, args);
        va_end(args);
        if (written < 0)
        {
            return 0;
        }
        auto added = static_cast<size_t>(written);
        if (added > maxLength)
        {
            added = maxLength;
        }
        bytes(size() + added);
        return added;
    }

    /**
     * @brief Swap buffers.
     * @param other Another buffer.
     */
    void swap(Buffer& other)
    {
        swapInternal(other);
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
