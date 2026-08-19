/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-08                                             ║
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

#include <sptk5/cutils>
#include <sptk5/db/Transaction.h>

using namespace std;
using namespace sptk;

Transaction::Transaction(const DatabaseConnection& db)
{
    if (db)
    {
        m_db = db->connection();
        if (!m_db.expired())
        {
            return;
        }
    }
    throw DatabaseException("Database connection is not valid");
}

Transaction::~Transaction()
{
    if (m_active)
    {
        try
        {
            auto db = getConnection(true);
            db->rollbackTransaction();
        }
        catch (const exception& e)
        {
            CERR(e.what());
        }
    }
}

void Transaction::begin()
{
    auto db = getConnection(false);
    db->beginTransaction();
    m_active = true;
}

void Transaction::commit()
{
    auto db = getConnection(true);
    db->commitTransaction();
    m_active = false;
}

void Transaction::rollback()
{
    auto db = getConnection(true);
    db->rollbackTransaction();
    m_active = false;
}

SPoolDatabaseConnection Transaction::getConnection(const bool expectedActive) const
{
    if (auto db = m_db.lock())
    {
        if (m_active == expectedActive)
        {
            return db;
        }
        if (m_active)
        {
            throw DatabaseException("Transaction is already active");
        }
        throw DatabaseException("Transaction is not active");
    }
    throw DatabaseException("Database connection is not valid");
}
