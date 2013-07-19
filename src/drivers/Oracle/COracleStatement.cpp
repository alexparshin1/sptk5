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

#include <sptk5/db/COracleConnection.h>

using namespace std;
using namespace sptk;

COracleStatement::COracleStatement(COracleConnection* connection, string sql) :
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
        CParam* parameter = *itor;
        if (parameter->isOutput())
            m_state.outputParameterCount++;
    }
}

void COracleStatement::setParameterValues()
{
    int parameterIndex = 1;
    CParamVector::iterator
        itor = m_enumeratedParams.begin(),
        iend = m_enumeratedParams.end();
    for (; itor != iend; itor++)
    {
        CParam& parameter = *(*itor);
        if (parameter.isNull())
            m_oracleStatement->setNull(parameterIndex, oracle::occi::OCCI_SQLT_STR);
        else
        switch (parameter.dataType()) {

            case VAR_NONE:      ///< Undefined
                throwDatabaseException("Parameter " + parameter.name() + " data type is undefined");

            case VAR_INT:       ///< Integer
                m_oracleStatement->setInt(parameterIndex, parameter.getInteger());
                break;

            case VAR_FLOAT:     ///< Floating-point (double)
            case VAR_MONEY:     ///< Floating-point (double) money
                m_oracleStatement->setDouble(parameterIndex, parameter.getFloat());
                break;

            case VAR_STRING:    ///< String pointer
                m_oracleStatement->setString(parameterIndex, parameter.getString());
                break;

            case VAR_TEXT:      ///< String pointer, corresponding to BLOBS in database
                {
                    ub2   length = parameter.dataSize();
                    m_oracleStatement->setDataBuffer(parameterIndex, parameter.dataBuffer(),
                                                     oracle::occi::OCCI_SQLT_CLOB,
                                                     parameter.bufferSize(), &length);
                }
                break;

            case VAR_BUFFER:    ///< Data pointer, corresponding to BLOBS in database
                {
                    ub2   length = parameter.dataSize();
                    m_oracleStatement->setDataBuffer(parameterIndex, parameter.dataBuffer(),
                                                     oracle::occi::OCCI_SQLT_BLOB,
                                                     parameter.bufferSize(), &length);
                }
                break;

            case VAR_DATE:      ///< CDateTime (double)
                {
                    int16_t year, month, day;
                    parameter.getDateTime().decodeDate(&year, &month, &day);
                    oracle::occi::Date dateValue(m_connection->environment(), year, month, day);
                    m_oracleStatement->setDate(parameterIndex, dateValue);
                }
                break;

            case VAR_DATE_TIME: ///< CDateTime (double)
                {
                    int16_t year, month, day;
                    parameter.getDateTime().decodeDate(&year, &month, &day);
                    int16_t hour, minute, second, msecond;
                    parameter.getDateTime().decodeTime(&hour, &minute, &second, &msecond);
                    oracle::occi::Timestamp timestampValue(m_connection->environment(),
                                                           year, month, day,
                                                           hour, minute, second);
                    m_oracleStatement->setTimestamp(parameterIndex, timestampValue);
                }
                break;

            case VAR_INT64:     ///< 64bit integer
                m_oracleStatement->setInt(parameterIndex, parameter.getInteger());
                break;

            case VAR_BOOL:      ///< Boolean
                m_oracleStatement->setInt(parameterIndex, parameter.getInteger());
                break;

            case VAR_IMAGE_PTR: ///< Image pointer
            case VAR_IMAGE_NDX: ///< Image index in object-specific table of image pointers
                throwDatabaseException("Unsupported data type for parameter " + parameter.name());
        }
        parameterIndex++;
    }
}
/*
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
*/