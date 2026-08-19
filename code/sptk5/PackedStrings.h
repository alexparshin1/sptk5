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

#include <sptk5/FieldList.h>
#include <sptk5/Strings.h>

namespace sptk {
/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Type definition for pchar
 */
using pchar = char*;

/**
 * Type definition for cpchar
 */
using cpchar = const char*;


/**
 * @brief Packed strings list
 *
 * Special data structure to contain several strings packed into same memory block.
 * The idea was to minimize the memory allocation, and decrease the total required memory.
 * Also, contains special attributes for CListView row support.
 */
class SP_EXPORT CPackedStrings : public Strings
{

public:
    /**
     * Constructor
     * @param cnt int, source strings count
     * @param strings const char *, source strings
     */
    CPackedStrings(int cnt, const char* strings[]);

    /**
     * Constructor
     * @param fields CFieldList, the fields data
     * @param keyField int, the key field number
     */
    CPackedStrings(FieldList& fields, int keyField);

    /**
     * Constructor
     * @param strings           Source strings
     */
    explicit CPackedStrings(const Strings& strings);

    /**
     * Deleted copy constructor
     * @param other             Other object
     */
    CPackedStrings(const CPackedStrings& other) = delete;

    /**
         * Destructor
         */
    ~CPackedStrings() override = default;

    /**
     * Assignment operator
     */
    CPackedStrings& operator=(const CPackedStrings&);

    /**
     * Assignment operator
     */
    CPackedStrings& operator=(const Strings&);

    /**
     * Row height for CListView
     */
    unsigned char height {0};

    /**
     * Row flags for CListView
     */
    unsigned char flags {0};
};
/**
 * @}
 */
} // namespace sptk
