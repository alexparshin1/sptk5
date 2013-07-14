/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTransaction.h  -  description
                             -------------------
    begin                : Mon Apr 17 2000
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/db/CTransaction.h>

using namespace std;
using namespace sptk;

CTransaction::CTransaction(CDatabaseConnection& db)
{
    m_active = false;
    m_db = &db;
}

CTransaction::~CTransaction()
{
    if (m_active)
        m_db->rollbackTransaction();
}

void CTransaction::begin()
{
    if (m_active)
        throw CDatabaseException("This transaction is already active");
    m_active = true;
    m_db->beginTransaction();
}

void CTransaction::commit()
{
    if (!m_active)
        throw CDatabaseException("This transaction is not active");
    m_db->commitTransaction();
    m_active = false;
}

void CTransaction::rollback()
{
    if (!m_active)
        throw CDatabaseException("This transaction is not active");
    m_db->rollbackTransaction();
    m_active = false;
}
