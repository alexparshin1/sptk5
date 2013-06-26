/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CPackedStrings.h  -  description
                             -------------------
    begin                : Tue Mar 21 2000
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CPACKEDSTRINGS_H__
#define __CPACKEDSTRINGS_H__

#include <sptk5/CStrings.h>
#include <sptk5/CFieldList.h>

namespace sptk {

    /// @addtogroup gui GUI Classes
    /// @{

    typedef char *          pchar;  ///< Type definition for pchar
    typedef const char *    cpchar; ///< Type definition for cpchar

    /// @brief Packed strings list
    ///
    /// Special data structure to contain several strings packed into same memory block.
    /// The idea was to minimize the memory allocation, and decrease the total required memory.
    /// Also, contains special attributes for CListView row support.
    class SP_EXPORT CPackedStrings {
            uint16_t   m_size;   ///< Number of strings
            void      *m_buffer; ///< Strings buffer
            void      *m_data;   ///< User data

        public:

            /// Constructor
            /// @param cnt int, source strings count
            /// @param strings const char *, source strings
            CPackedStrings(int cnt,const char *strings[]);

            /// Constructor
            /// @param fields CFieldList, the fields data
            /// @param keyField int, the key field number
            CPackedStrings(CFieldList& fields,int keyField);

            /// Constructor
            /// @param strings const CStrings&, source strings
            CPackedStrings(const CStrings& strings);

            /// Destructor
            ~CPackedStrings();

            /// Strings count
            uint16_t size() const { return *(uint16_t *)m_buffer; }

            /// String access by index
            const char * operator[](uint16_t index) const;

            /// Assignment operator
            CPackedStrings& operator=(const CPackedStrings&);

            /// Assignment operator
            CPackedStrings& operator=(const CStrings&);

            /// Sets user_data as void *
            void user_data(void *d) { m_data = d;           }

            /// Returns user_data as void *
            void *user_data() const { return m_data;        }

            /// Sets user_data as integer
            void argument(int32_t arg)  { m_data = (void *) (long) arg; }

            /// Returns user_data as integer
            int32_t argument() const   { return (int32_t)(long) m_data;   }

        public:

            /// Row height for CListView
            unsigned char    height;

            /// Row flags for CListView
            unsigned char    flags;
    };
    /// @}
}
#endif
