/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CLocation.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <sptk5/sptk.h>
#include <sptk5/string_ext.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Location object
 *
 * Stores file name and line number in the file
 */
class SP_EXPORT CLocation
{
    /**
     * File name
     */
    const char*         m_file;

    /**
     * Line number
     */
    int                 m_line;


public:
    /**
     * @brief Constructor
     * @param file const char*, File name
     * @param line int, Line number
     */
    CLocation(const char* file, int line) 
    : m_file(file), m_line(line)
    {}

    /**
     * @brief Modifies location
     * @param file const char*, File name
     * @param line int, Line number
     */
    void set(const char* file, int line)
    {
        m_file = file;
        m_line = line;
    }

    /**
     * @brief Returns location file name
     */
    const char* file() const
    {
        return m_file;
    }

    /**
     * @brief Returns location line number
     */
    int line() const
    {
        return m_line;
    }

    /**
     * @brief Returns string presentation of location
     */
    std::string toString() const
    {
        return std::string(m_file) + "(" + int2string(m_line) + ")";
    }

    /**
     * @brief Returns true if location is empty
     */
    bool empty() const
    {
        return (m_file == NULL) && (m_line == 0);
    }
};

/**
 * @}
 */
}

#endif
