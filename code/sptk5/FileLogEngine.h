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

#include <fstream>
#include <sptk5/LogEngine.h>

namespace sptk {
/**
 * @addtogroup log Log Classes.
 * @{
 */

/**
 * A log stored in the regular file.
 *
 * The simplest possible way to implement logging.
 * The log file is created automatically if it doesn't exist.
 * @see CBaseLog for more information about basic log abilities.
 */
class SP_EXPORT FileLogEngine : public LogEngine
{
public:
    /**
     * @brief Constructor.
     *
     * Creates a new log object based on the file name.
     * If this file doesn't exist, it will be created.
     * @param fileName          Log file name.
     * @param append            If true, appends to the existing file, otherwise overwrites.
     */
    explicit FileLogEngine(const std::filesystem::path& fileName, bool append = false);

    /**
     * @brief Destructor.
     */
    ~FileLogEngine() override;

    /**
     * @brief Stores or sends log message to actual destination.
     * @param message           Log message.
     */
    bool saveMessage(const Logger::Message& message) override;

    /**
     * @brief Flush file data to disk.
     */
    void flush() override;

    /**
     * @brief Restarts the log.
     *
     * The current log content is cleared. The file is recreated.
     */
    void reset() override;

    /**
     * @brief Sets the current log aside and starts an empty one.
     *
     * The file is closed, renamed by appending a timestamp to its name - "xmq_server.log" becomes
     * "xmq_server.log.20260828.1912" - and a new, empty file is opened under the original name.
     * Nothing is deleted: what to do with the files set aside is for the caller to decide.
     *
     * Unlike reset(), which throws away the history, this keeps it. A long-running service writes
     * to one name for ever, and without this nothing ever trims it.
     *
     * Safe to call while other threads are logging: the engine's own lock is held throughout, and
     * a message that arrives during the call is written to whichever file is open when its turn
     * comes. If the rename cannot be done, the original file is reopened and kept in use rather
     * than logging stopping - losing the log is worse than not rotating it.
     *
     * @return the name the old log was given, or an empty path if there was nothing to set aside.
     */
    std::filesystem::path rotate() override;

protected:
    void close() override; ///< Close the file stream.

private:
    std::filesystem::path m_fileName;   ///< Log file name
    std::ofstream         m_fileStream; ///< Log file stream
};
/**
 * @}
 */
} // namespace sptk
