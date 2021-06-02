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

#include <sptk5/sptk.h>

namespace sptk {
/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Case-insensitive string compare class.
 *
 * Lower case string compare class is really useful if we need
 * a case-independent string map
 */
class SP_EXPORT CaseInsensitiveCompare
{
public:
    /**
     * Compare method
     * @param s1                First compare string
     * @param s2                Second compare string
     */
    bool operator()(const String& s1, const String& s2) const
    {
#ifdef WIN32
        return strcasecmp(s1.c_str(), s2.c_str()) < 0;
#else
        return strcasecmp(s1.c_str(), s2.c_str()) < 0;
#endif
    }
};

/**
 * @}
 */

}

