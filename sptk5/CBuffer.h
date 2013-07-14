/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CBuffer.h  -  description
                             -------------------
    begin                : Mon Apr 17 2000
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CBUFFER_H__
#define __CBUFFER_H__

#include <sptk5/sptk.h>

#include <stdlib.h>
#include <string.h>
#include <string>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Memory data buffer
///
/// Generic buffer with a special memory-allocation strategy for effective append() operation
    class SP_EXPORT CBuffer {

            /// @brief Resizes current buffer
            /// @param sz uint32_t, required memory size
            void adjustSize(uint32_t sz);

        protected:

            uint32_t m_size;     ///< Allocated size of the buffer
            uint32_t m_bytes;    ///< Actual size of the data in buffer
            char*  m_buffer;   ///< The buffer itself

        public:

            /// @brief Default constructor
            ///
            /// Creates an empty buffer.
            /// The return of the bytes() method will be 0.
            /// @param sz uint32_t, buffer size to be pre-allocated
            CBuffer(uint32_t sz=16);

            /// @brief Constructor
            ///
            /// Creates a buffer from string.
            /// The string is copied inside the buffer.
            /// The return of the bytes() method will be the input string length.
            /// @param str const char *, input string
            CBuffer(const char *str);

            /// @brief Constructor
            ///
            /// Creates a buffer from string.
            /// The string is copied inside the buffer.
            /// The return of the bytes() method will be the input string length.
            /// @param str const std::string&, input string
            CBuffer(const std::string& str);

            /// @brief Constructor
            ///
            /// Creates a buffer from void *data.
            /// The data is copied inside the buffer.
            /// The return of the bytes() method will be the input data size.
            /// @param data void * data buffer
            /// @param sz uint32_t, data buffer size
            CBuffer(const void *data,uint32_t sz);

            /// @brief Copy constructor
            ///
            /// Creates a buffer from another buffer.
            /// The data is copied inside the buffer.
            /// The return of the bytes() method will be the input data size.
            /// @param buffer CBuffer, data buffer
            CBuffer(const CBuffer& buffer);

            /// @brief Destructor
            ~CBuffer() {
                if (m_buffer)
                    free(m_buffer);
            }

            /// @brief Returns pointer on the data buffer.
            char *data() const {
                return m_buffer;
            }

            /// @brief Returns const char pointer on the data buffer.
            const char *c_str() const {
                return m_buffer;
            }

            /// @brief Returns true if number of bytes in buffer is zero.
            bool empty() const { return m_bytes == 0; }

            /// @brief Checks if the current buffer size is enough
            ///
            /// Allocates memory if needed.
            /// @param sz uint32_t, required memory size
            void checkSize(uint32_t sz) {
                if (sz > m_size)
                    adjustSize(sz);
            }

            /// @brief Copies the external data of size sz into the current buffer.
            ///
            /// Allocates memory if needed.
            /// @param data const char*, external data buffer
            /// @param sz uint32_t, required memory size
            /// @returns success (true) or failure (false)
            void set(const char* data,uint32_t sz);

            /// @brief Copies the external data of size sz into the current buffer.
            ///
            /// Allocates memory if needed.
            /// @param data const CBuffer&, external data buffer
            /// @returns success (true) or failure (false)
            void set(const CBuffer& data) {
                set(data.m_buffer,data.m_bytes);
            }

            /// @brief Appends a single char to the current buffer.
            ///
            /// Allocates memory if needed.
            /// @param ch single character
            void append(char ch);

            /// @brief Appends a single char to the current buffer.
            ///
            /// Allocated memory isn't checked. Application should call checkSize() to make sure the required size is there
            /// @param ch single character
            void appendNoCheck(char ch);

            /// @brief Appends the external data of size sz to the current buffer.
            ///
            /// Allocates memory if needed.
            /// @param data const char *, external data buffer
            /// @param sz uint32_t, required memory size
            void append(const char *data,uint32_t sz=0);

            /// @brief Appends the external data of size sz to the current buffer.
            ///
            /// Allocated memory isn't checked. Application should call checkSize() to make sure the required size is there
            /// @param data const char *, external data buffer
            /// @param sz uint32_t, required memory size
            void appendNoCheck(const char *data,uint32_t sz=0);

            /// @brief Appends the string to the current buffer.
            ///
            /// Allocates memory if needed.
            /// @param str std::string, string to append
            void append(const std::string& str) {
                return append(str.c_str(),(uint32_t)str.length());
            }

            /// @brief Appends the string to the current buffer.
            ///
            /// Allocates memory if needed.
            /// @param buffer const CBuffer&, data to append
            void append(const CBuffer& buffer) {
                return append(buffer.data(),buffer.bytes());
            }

            /// @brief Truncates the current buffer to the size sz.
            ///
            /// Deallocates unused memory if needed.
            /// @param sz uint32_t, the required data size in bytes
            void reset(uint32_t sz=0);

            /// @brief Fills the bytes() characters in buffer with character ch.
            /// @param ch the character to fill the buffer
            void fill(char ch);

            /// @brief Returns the size of memory allocated for the data buffer
            /// @returns buffer size
            uint32_t size()  const {
                return m_size;
            }

            /// @brief Returns the size of data in the data buffer
            /// @returns data size
            uint32_t bytes() const {
                return m_bytes;
            }

            /// @brief Returns the size of data in the data buffer
            /// @returns data size
            uint32_t length() const {
                return m_bytes;
            }

            /// @brief Sets the size of the data stored
            ///
            /// Doesn't check anything so use it this caution.
            /// @param b uint32_t, the new size of the buffer
            void bytes(uint32_t b) {
                m_bytes = b;
            }

            /// @brief Access the chars by index
            /// @param index uint32_t, character index
            char& operator[](uint32_t index) { return m_buffer[index]; }

            /// @brief Access the chars by index, const version
            /// @param index uint32_t, character index
            const char& operator[](uint32_t index) const { return m_buffer[index]; }

            /// @brief Loads the buffer from file fileName.
            /// @param fileName std::string, the name of the input file
            void loadFromFile(std::string fileName);

            /// @brief Saves the buffer to the file fileName.
            /// @param fileName std::string, the name of the output file
            void saveToFile(std::string fileName) const;

            /// @brief Assigns from CBuffer
            /// @param b const CBuffer&, the buffer to assign from
            /// @returns this object
            CBuffer& operator = (const CBuffer& b);

            /// @brief Assigns from CString
            /// @param str const std::string&, the string to assign from
            /// @returns this object
            CBuffer& operator = (const std::string& str);

            /// @brief Assigns from char *
            /// @param str const char *, the string to assign from
            /// @returns this object
            CBuffer& operator = (const char *str);

            /// @brief Appends another CBuffer
            /// @param b const CBuffer&, the buffer to append
            /// @returns this object
            CBuffer& operator += (const CBuffer& b) { append(b); return *this; }

            /// @brief Appends CString
            /// @param str const std::string&, the string to append
            /// @returns this object
            CBuffer& operator += (const std::string& str) { append(str); return *this; }

            /// @brief Appends const char *
            /// @param str const char *, the string to append
            /// @returns this object
            CBuffer& operator += (const char *str) { append(str); return *this; }

            /// @brief Convertor to std::string
            operator std::string() const { return std::string(m_buffer,m_bytes); }
    };
/// @}
}
#endif
