/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SourceModule.h - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_SOURCEMODULE_H__
#define __SPTK_SOURCEMODULE_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <iostream>
#include <fstream>

namespace sptk
{

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * @brief Helper module to generate source files
 */
class SourceModule
{
    /**
     * Module name
     */
    String          m_name;

    /**
     * Module path
     */
    String          m_path;

    /**
     * Module hpp file
     */
    std::ofstream   m_header;

    /**
     * Module cpp file
     */
    std::ofstream   m_source;

public:
    /**
     * @brief Constructor
     * @param moduleName        Module name
     * @param modulePath        Module path
     */
    SourceModule(const String& moduleName, const String& modulePath);

    /**
     * @brief Destructor
     */
    ~SourceModule();

    /**
     * @brief Opens module output files
     */
    void open();

    /**
     * @brief Returns header file stream
     */
    std::ostream& header();

    /**
     * @brief Returns source file stream
     */
    std::ostream& source();
};

/**
 * @}
 */

}
#endif
