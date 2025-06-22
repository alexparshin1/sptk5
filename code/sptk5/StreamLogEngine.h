/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <fstream>
#include <sptk5/LogEngine.h>

namespace sptk {

/**
 * @addtogroup log Log Classes.
 * @{
 */

/**
 * @brief A log stored into external stream.
 * @see CBaseLog for more information about basic log abilities.
 */
class SP_EXPORT StreamLogEngine
    : public sptk::LogEngine
{
public:
    /**
     * @brief Constructor.
     * @param outputStream          Output stream
     */
    explicit StreamLogEngine(std::ostream& outputStream);

    /**
     * @brief Destructor
     */
    ~StreamLogEngine() override;

    /**
     * Stores or sends log message to actual destination
     * @param message           Log message
     */
    void saveMessage(const Logger::Message& message) override;

private:
    std::ostream& m_logStream; ///< Log stream
};
/**
 * @}
 */
} // namespace sptk
