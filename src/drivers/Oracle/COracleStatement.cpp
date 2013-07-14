/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          COracleStatement.cpp  -  description
                             -------------------
    begin                : Sun Jul 14 2013
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

#include "COracleStatement.h"

using namespace std;
using namespace sptk;

COracleStatement::COracleStatement(Connection* connection, string sql) :
        m_oracleStatement(connection->createStatement(sql))
{
    m_state.columnCount = 0;
    m_state.eof = true;
    m_state.transaction = false;
    m_state.outputParameterCount = 0;
    m_oracleStatement->setAutoCommit(!m_state.transaction);
}

COracleStatement::~COracleStatement()
{
    Connection* connection = m_oracleStatement->getConnection();
    connection->terminateStatement(m_oracleStatement);
}

void COracleStatement::enumerateParams(CParamList& queryParams)
{
    queryParams.enumerate(m_enumeratedParams);
    m_state.outputParameterCount = 0;

    CParamVector::iterator
        itor = m_enumeratedParams.begin(),
        iend = m_enumeratedParams.end();
    for (; itor != iend; itor++)
    {
        CParam* param = *itor;
        if (param->isOutput())
            m_state.outputParameterCount++;
    }
}

void COracleStatement::execute(bool inTransaction)
{
    // If statement is inside the transaction, it shouldn't be in auto-commit mode
    if (inTransaction != m_state.transaction) {
        m_oracleStatement->setAutoCommit(!inTransaction);
        m_state.transaction = inTransaction;
    }

    if (m_resultSets.size())
        close();

    m_state.eof = true;
    m_state.columnCount = 0;

    if (m_oracleStatement->execute() == Statement::RESULT_SET_AVAILABLE) {

        ResultSet* resultSet = m_oracleStatement->getResultSet();
        m_resultSets.push_front(resultSet);

        vector<MetaData> columnsMetaData = resultSet->getColumnListMetaData();

        m_state.eof = false;
        m_state.columnCount = columnsMetaData.size();

        // Check if m_resultSet contains nested cursors
        for (unsigned column = 0; column < m_state.columnCount; column++) {
            const MetaData& metaData = columnsMetaData[column];
            int columnType = metaData.getInt(MetaData::ATTR_DATA_TYPE);

            if (columnType == SQLT_RSET) {
                // Found nested cursor, ignoring other columns
                resultSet->next();
                resultSet = resultSet->getCursor(column + 1);
                m_resultSets.push_front(resultSet);
                m_state.columnCount = resultSet->getColumnListMetaData().size();
                break;
            }
        }
    }
}

void COracleStatement::close()
{
    list<ResultSet*>::iterator
        itor = m_resultSets.begin(),
        iend = m_resultSets.end();

    for(; itor != iend; itor++)
        (*itor)->cancel();

    m_resultSets.clear();
}
