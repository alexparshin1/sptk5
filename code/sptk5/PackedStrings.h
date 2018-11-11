/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPackedStrings.h - description                         ║
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

#ifndef __CPACKEDSTRINGS_H__
#define __CPACKEDSTRINGS_H__

#include <sptk5/Strings.h>
#include <sptk5/FieldList.h>

namespace sptk {

    /**
     * @addtogroup gui GUI Classes
     * @{
     */

    /**
     * Type definition for pchar
     */
    typedef char *          pchar;

    /**
     * Type definition for cpchar
     */
    typedef const char *    cpchar;


    /**
     * @brief Packed strings list
     *
     * Special data structure to contain several strings packed into same memory block.
     * The idea was to minimize the memory allocation, and decrease the total required memory.
     * Also, contains special attributes for CListView row support.
     */
    class SP_EXPORT CPackedStrings {
            /**
             * Number of strings
             */
            uint16_t   m_size;

            /**
             * Strings buffer
             */
            void      *m_buffer;

            /**
             * User data
             */
            void      *m_data;


        public:

            /**
             * Constructor
             * @param cnt int, source strings count
             * @param strings const char *, source strings
             */
            CPackedStrings(int cnt,const char *strings[]);

            /**
             * Constructor
             * @param fields CFieldList, the fields data
             * @param keyField int, the key field number
             */
            CPackedStrings(FieldList& fields,int keyField);

            /**
             * Constructor
             * @param strings const Strings&, source strings
             */
            explicit CPackedStrings(const Strings& strings);

            /**
             * Destructor
             */
            ~CPackedStrings();

            /**
             * Strings count
             */
            uint16_t size() const { return *(uint16_t *)m_buffer; }

            /**
             * String access by index
             */
            const char * operator[](uint16_t index) const;

            /**
             * Assignment operator
             */
            CPackedStrings& operator=(const CPackedStrings&);

            /**
             * Assignment operator
             */
            CPackedStrings& operator=(const Strings&);

            /**
             * Sets user_data as void *
             */
            void user_data(void *d) { m_data = d;           }

            /**
             * Returns user_data as void *
             */
            void *user_data() const { return m_data;        }

            /**
             * Sets user_data as integer
             */
            void argument(int32_t arg)  { m_data = (void *) (long) arg; }

            /**
             * Returns user_data as integer
             */
            int32_t argument() const   { return (int32_t)(long) m_data;   }

        public:

            /**
             * Row height for CListView
             */
            unsigned char    height;

            /**
             * Row flags for CListView
             */
            unsigned char    flags;
    };
    /**
     * @}
     */
}
#endif
