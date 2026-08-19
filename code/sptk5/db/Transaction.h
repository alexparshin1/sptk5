/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#include "AutoDatabaseConnection.h"
#include <sptk5/db/PoolDatabaseConnection.h>

namespace sptk {
/**
 * @addtogroup Database Database Support.
 * @{
 */

/**
 * @brief Database Transaction.
 *
 * Allows operations that begin, commit, and rollback the transaction automatically.
 * If the transaction object is deleted w/o commiting or rolling back
 * the transaction, it rolls back the transaction (if active).
 */
class SP_EXPORT Transaction
{
public:
    /**
     * @brief Constructor.
     * @param db DatabaseConnection&, the database to work with.
     */
    explicit Transaction(const DatabaseConnection& db);

    /**
     * @brief Destructor.
     */
    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    /**
     * @brief Begins the transaction.
     */
    void begin();

    /**
     * @brief Commits the transaction.
     */
    void commit();

    /**
     * @brief Rolls back the transaction.
     */
    void rollback();

    /**
     * @brief Is the transaction active?
     */
    [[nodiscard]] bool active() const
    {
        return m_active;
    }

private:
    WPoolDatabaseConnection m_db;             ///< Database to work with.
    bool                    m_active {false}; ///< Transaction activity.

    /**
     * @brief Gets the database connection.
     * @param expectedActive Expected transaction activity state.
     * @return Database connection.
     */
    SPoolDatabaseConnection getConnection(bool expectedActive) const;
};
/**
 * @}
 */
} // namespace sptk
