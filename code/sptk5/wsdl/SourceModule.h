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

#include <sptk5/Buffer.h>
#include <sptk5/sptk.h>
#include <sstream>

namespace sptk {
/**
 * @addtogroup wsdl WSDL-related Classes.
 * @{
 */

/**
 * @brief Helper module to generate source files.
 */
class SP_EXPORT SourceModule
{
public:
    /**
     * @brief Constructor.
     * @param moduleName        Module name.
     * @param modulePath        Module path.
     */
    SourceModule(String moduleName, String modulePath);

    /**
     * @brief Reset module for output.
     */
    void reset();

    /**
     * @brief Returns header file stream.
     */
    std::ostream& header();

    /**
     * @brief Returns source file stream.
     */
    std::ostream& source();

    /**
     * @brief Write output .h and .cpp files if they don't exist.
     *        or have different content.
     */
    void writeOutputFiles();

private:
    String            m_name;   ///< Module name.
    String            m_path;   ///< Module path.
    std::stringstream m_header; ///< Module .h file content.
    std::stringstream m_source; ///< Module cpp file content.

    /**
     * @brief Write data to file if it doesn't exist, or if file content is different from data.
     * @param fileNameAndExtension  File name.
     * @param data                  Data to write.
     */
    void writeFile(const String& fileNameAndExtension, const Buffer& data);
};

/**
 * @}
 */

} // namespace sptk
