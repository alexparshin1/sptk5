/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTransaction.cpp - description                         ║
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

#include <sptk5/db/Transaction.h>

using namespace std;
using namespace sptk;

Transaction::Transaction(DatabaseConnection db)
{
    m_active = false;
    m_db = db->connection();
}

Transaction::~Transaction()
{
    try {
        if (m_active)
            m_db->rollbackTransaction();
    }
    catch (...) {}
}

void Transaction::begin()
{
    if (m_active)
        throw DatabaseException("This transaction is already active");
    try {
        m_db->beginTransaction();
    }
    catch (const DatabaseException& e) {
        if (strstr(e.what(), "connection") == nullptr)
            throw;
        m_db->close();
        m_db->open();
        m_db->beginTransaction();
    }
    m_active = true;
}

void Transaction::commit()
{
    if (!m_active)
        throw DatabaseException("This transaction is not active");
    m_db->commitTransaction();
    m_active = false;
}

void Transaction::rollback()
{
    if (!m_active)
        throw DatabaseException("This transaction is not active");
    m_db->rollbackTransaction();
    m_active = false;
}
