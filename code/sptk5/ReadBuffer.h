/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Buffer.h>

namespace sptk {

/**
 * Generic read buffer.
 *
 * Data is added to the buffer the usual way, using ctors and/or append operations.
 * Any read operations copy data into external buffer, then advance internal read offset.
 */
class ReadBuffer : public Buffer
{
    size_t  m_readOffset {0};   ///< read offset

    /**
     * Shift the buffer content to the beginning of the buffer, if read offset past 3/4 of the content size
     */
    void compact()
    {
        if (m_readOffset >= bytes() * 3 / 4) {
            erase(0, m_readOffset);
            m_readOffset = 0;
        }
    }

public:
    /**
     * Default constructor
     */
    explicit ReadBuffer(size_t size=64) : Buffer(size) {}

    /**
     * Constructor
     * @param data              Data
     * @param size              Data size
     */
    ReadBuffer(const char* data, size_t size) : Buffer(data, size) {}

    /**
     * Read into data of primitive type (int, double, etc).
     * @param data              Data
     * @return true if read was successful
     */
    template <typename T>
    bool read(T& data)
    {
        return read((uint8_t*) &data, sizeof(T));
    }

    /**
     * Read data. Internal read offset is advanced by length.
     * @param data              Data
     * @param size              Data size
     * @return true if read was successful
     */
    bool read(uint8_t* data, size_t size);

    /**
     * Read into string
     * @param data              Data
     * @param length            Data size
     * @return true if read was successful
     */
    bool read(String& data, size_t length);

    /**
     * Read into buffer
     * @param data              Data
     * @param length            Data size
     * @return true if read was successful
     */
    bool read(Buffer& data, size_t length);

    /**
     * The start of un-read data
     * @return
     */
    char* head()
    {
        return data() + m_readOffset;
    }

    /**
     * Get number of bytes, available for read
     * @return number of bytes, available for read
     */
    size_t available() const
    {
        return bytes() - readOffset();
    }

    /**
     * @return true if there are no available bytes to read
     */
    bool eof() const
    {
        return readOffset() >= bytes();
    }

    /**
     * Get internal read offset
     * @return internal read offset
     */
    size_t readOffset() const
    {
        return m_readOffset;
    }
};

}

