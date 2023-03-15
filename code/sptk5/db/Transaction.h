/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#include "AutoDatabaseConnection.h"
#include <sptk5/Exception.h>
#include <sptk5/db/PoolDatabaseConnection.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief Database Transaction.
 *
 * Allows to begin, commit, and rollback the transaction automatically.
 * If the transaction object is deleted w/o commiting or rolling back
 * the transaction, it rolls back the transaction (if active)
 */
class SP_EXPORT Transaction
{
public:
    /**
     * Constructor
     * @param db DatabaseConnection&, the database to work with
     */
    explicit Transaction(const DatabaseConnection& db);

    /**
     * Destructor
     */
    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    /**
     * Begins the transaction
     */
    void begin();

    /**
     * Commits the transaction
     */
    void commit();

    /**
     * Rolls back the transaction
     */
    void rollback();

    /**
     * Is transaction active?
     */
    bool active() const
    {
        return m_active;
    }

private:
    PoolDatabaseConnection* m_db; ///< Database to work with
    bool m_active {false};        ///< Transaction activity
};
/**
 * @}
 */
} // namespace sptk
