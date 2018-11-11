/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SharedStrings.h - description                          ║
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

#ifndef __SPTK_SHAREDSTRINGS_H__
#define __SPTK_SHAREDSTRINGS_H__

#include <sptk5/Exception.h>
#include <string>
#include <set>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Shared strings table
 *
 * Contains a table of shared strings for the use with different objects
 * such as XML node, ListView, etc
 */
class SP_EXPORT SharedStrings
{
    /**
     * String set type
     */
    typedef std::set<std::string> Set;

    /**
     * Set of shared strings
     */
    Set     m_strings;

public:
    /**
     * @brief Default constructor
     */
    SharedStrings();

    /**
     * @brief Find a shared string
     *
     * Looks for an existing shared string, and returns a const std::string&
     * to it. If a shared string not found, returns nullptr.
     * @param str const char *, a string to find
     */
    const std::string* findString(const char *str) const;
    
    /**
     * @brief Obtain a shared string
     *
     * Looks for an existing shared string, and returns a const std::string&
     * to it. If a shared string not found, it's created with reference count one.
     * For an existing shared string, every call of this method encreases string
     * reference counter by one
     * @param str const char *, a string to share
     */
    const std::string& shareString(const char *str);

    /**
     * @brief Obtain a shared string
     *
     * Looks for an existing shared string, and returns a const char *pointer
     * to it. If a shared string not found, it's created with reference count one.
     * For an existing shared string, every call of this method encreases string
     * reference counter by one
     * @param str const std::string&, a string to share
     */
    const std::string& shareString(const std::string& str) 
    {
        return shareString(str.c_str());
    }
    
    /**
     * @brief Clear shared starings
     */
    void clear();
};

/**
 * @}
 */
}

#endif
