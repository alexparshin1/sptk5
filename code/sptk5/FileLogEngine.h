/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
 * @addtogroup log Log Classes
 * @{
 */

/**
 * A log stored in the regular file.
 *
 * A simplest possible way to implement logging.
 * The log file is created automatically if it doesn't exist.
 * @see CBaseLog for more information about basic log abilities.
 */
class SP_EXPORT FileLogEngine
    : public sptk::LogEngine
{
public:
    /**
     * Constructor
     *
     * Creates a new log object based on the file name.
     * If this file doesn't exist - it will be created.
     * @param fileName          Log file name
     */
    explicit FileLogEngine(const std::filesystem::path& fileName);

    ~FileLogEngine() override;

    /**
     * Stores or sends log message to actual destination
     * @param message           Log message
     */
    void saveMessage(Logger::UMessage&& message) override;

    /**
     * Restarts the log
     *
     * The current log content is cleared. The file is recreated.
     */
    void reset() override;

protected:
    void close() override;

private:
    std::filesystem::path m_fileName; ///< Log file name
    std::ofstream m_fileStream;       ///< Log file stream
};
/**
 * @}
 */
} // namespace sptk
