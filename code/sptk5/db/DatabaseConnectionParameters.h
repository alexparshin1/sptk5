/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseConnectionParameters.h - description          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __SPTK_DATABASECONNECTIONPARAMETERS_H__
#define __SPTK_DATABASECONNECTIONPARAMETERS_H__

#include <sptk5/sptk.h>

#include <map>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief Database connection parameters
 *
 * Converts database connection string to string-string map.
 */
class SP_EXPORT DatabaseConnectionParameters: public std::map<std::string,std::string>
{

public:

    /**
     * @brief Constructor
     * @param connectionString std::string, the connection string
     */
    explicit DatabaseConnectionParameters(std::string connectionString);

    /**
     * @brief Returns connection string in format of name=value pairs
     */
    std::string toString(const char* delimiter=" ") const;
};
/**
 * @}
 */
}
#endif
