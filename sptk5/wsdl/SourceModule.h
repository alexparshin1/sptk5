/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SourceModule.h - description                           ║
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
    std::string     m_name;

    /**
     * Module path
     */
    std::string     m_path;

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
     * @param moduleName std::string, Module name
     * @param modulePath std::string, Module path
     */
    SourceModule(std::string moduleName, std::string modulePath)
    : m_name(moduleName), m_path(modulePath)
    {
    }

    /**
     * @brief Destructor
     */
    ~SourceModule()
    {
        if (m_header.is_open())
            m_header.close();
        if (m_source.is_open())
            m_source.close();
    }

    /**
     * @brief Opens module output files
     */
    void open()
    {
        if (m_path.empty())
            m_path = ".";
        std::string fileName = m_path + "/" + m_name;
        m_header.open((fileName + ".h").c_str(), std::ofstream::out | std::ofstream::trunc);
        if (!m_header.is_open())
            throw Exception("Can't create file " + fileName + ".h");
        m_source.open((fileName + ".cpp").c_str(), std::ofstream::out | std::ofstream::trunc);
        if (!m_source.is_open())
            throw Exception("Can't create file " + fileName + ".cpp");
    }

    /**
     * @brief Returns header file stream
     */
    std::ofstream& header()
    {
        return m_header;
    }

    /**
     * @brief Returns source file stream
     */
    std::ofstream& source()
    {
        return m_source;
    }
};

/**
 * @}
 */

}
#endif
