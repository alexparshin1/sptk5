/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        Transaction.h - description                           ║
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

#ifndef __SPTK_TRANSACTION_H__
#define __SPTK_TRANSACTION_H__

#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/Exception.h>
#include "AutoDatabaseConnection.h"

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
    /**
     * Transaction activity
     */
    bool                    m_active;

    /**
     * Database to work with
     */
    PoolDatabaseConnection*     m_db;

public:
    /**
     * Constructor
     * @param db DatabaseConnection&, the database to work with
     */
    explicit Transaction(DatabaseConnection db);

    /**
     * Destructor
     */
    ~Transaction();

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
};
/**
 * @}
 */
}
#endif
