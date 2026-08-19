/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/Buffer.h>

#undef min

namespace sptk {
/**
 * @brief Generic read buffer.
 *
 * Data is added to the buffer the usual way, using ctors and/or append operations.
 * Any read operations copy data into an external buffer, then advance internal read offset.
 */
class SP_EXPORT ReadBuffer final : public Buffer
{
public:
    using Buffer::Buffer;

    /**
     * @brief Read data of a trivially copyable type.
     * @param data              Data.
     * @return true if read was successful.
     */
    template<typename T>
        requires std::is_integral_v<T> || std::is_floating_point_v<T>
    bool read(T& data)
    {
        return read(reinterpret_cast<uint8_t*>(&data), sizeof(T));
    }

    /**
     * @brief Read data. Length advances internal read offset.
     * @param data              Data.
     * @param length            Data size.
     * @return true if read was successful.
     */
    bool read(uint8_t* data, size_t length);

    /**
     * @brief Read into string.
     * @param data              Data.
     * @param length            Data size.
     * @return true if read was successful.
     */
    bool read(String& data, size_t length);

    /**
     * @brief Read into buffer.
     * @param data              Data.
     * @param length            Data size.
     * @return true if read was successful.
     */
    bool read(Buffer& data, size_t length);

    /**
     * Get a number of bytes, available for read.
     * @return number of bytes, available for read.
     */
    [[nodiscard]] size_t available() const
    {
        if (m_readOffset >= bytes())
        {
            return 0;
        }
        return bytes() - m_readOffset;
    }

    /**
     * @return true if there are no available bytes to read.
     */
    [[nodiscard]] bool empty() const override
    {
        return readOffset() >= bytes();
    }

    /**
     * @brief Get internal read offset.
     * @return internal read offset.
     */
    [[nodiscard]] size_t readOffset() const
    {
        return m_readOffset;
    }

    void reset(const size_t size = 0) override
    {
        BufferStorage::reset(size);
        m_readOffset = 0;
    }

    [[nodiscard]] size_t bytes() const override
    {
        return BufferStorage::bytes();
    }

    void bytes(const size_t newSize) override
    {
        BufferStorage::bytes(newSize);
        m_readOffset = std::min(m_readOffset, newSize);
    }

    template<typename T>
    void set(const T* data, size_t size)
    {
        BufferStorage::set(data, size);
        m_readOffset = 0;
    }

    ReadBuffer& operator=(const Buffer& other)
    {
        Buffer::operator=(other);
        m_readOffset = 0;
        return *this;
    }

    ReadBuffer& operator=(const String& str)
    {
        Buffer::operator=(str);
        m_readOffset = 0;
        return *this;
    }

    ReadBuffer& operator=(const char* str)
    {
        Buffer::operator=(str);
        m_readOffset = 0;
        return *this;
    }

    void erase(const size_t offset, const size_t length) override
    {
        const auto oldReadOffset = m_readOffset;
        BufferStorage::erase(offset, length);

        if (oldReadOffset <= offset)
        {
            m_readOffset = oldReadOffset;
        }
        else
        {
            const auto readShift = std::min(length, oldReadOffset - offset);
            m_readOffset = oldReadOffset - readShift;
        }
        m_readOffset = std::min(m_readOffset, bytes());
    }

private:
    size_t m_readOffset {0}; ///< Read offset

    /**
     * @brief Shift the buffer content to the beginning of the buffer, if read offset past 3/4 of the content size.
     */
    void compact()
    {
        constexpr auto divider = 4;
        if (constexpr auto multiplier = 3;
            m_readOffset >= bytes() * multiplier / divider)
        {
            erase(0, m_readOffset);
            m_readOffset = 0;
        }
    }
};

} // namespace sptk
