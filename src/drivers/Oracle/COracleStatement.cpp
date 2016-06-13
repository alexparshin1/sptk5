/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       COracleStatement.cpp - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/db/COracleConnection.h>

using namespace std;
using namespace sptk;
using namespace oracle::occi;

COracleStatement::COracleStatement(COracleConnection* connection, string sql) :
    CDatabaseStatement<COracleConnection,oracle::occi::Statement>(connection),
    m_createClobStatement(NULL),
    m_createBlobStatement(NULL),
    m_resultSet(NULL)
{
    m_statement = connection->createStatement(sql);
    m_state.columnCount = 0;
    m_state.eof = true;
    m_state.transaction = false;
    m_state.outputParameterCount = 0;
    m_statement->setAutoCommit(!m_state.transaction);
}

COracleStatement::~COracleStatement()
{
    Connection* connection = m_statement->getConnection();
    if (m_createClobStatement)
        connection->terminateStatement(m_createClobStatement);
    if (m_createBlobStatement)
        connection->terminateStatement(m_createBlobStatement);
    if (m_statement)
        connection->terminateStatement(m_statement);
}

void COracleStatement::setClobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    if (m_connection) {
        if (!m_createClobStatement) {
            COracleConnection* oracleConnection = (COracleConnection*)m_connection;
            m_createClobStatement =
                oracleConnection->createStatement("INSERT INTO sptk_lobs(sptk_clob) VALUES (empty_clob()) RETURNING sptk_clob INTO :1");
            m_createClobStatement->registerOutParam(1, OCCICLOB);
        }

        // Create row with empty CLOB, and return its locator for data filling
        m_createClobStatement->executeUpdate();
        Clob clob = m_createClobStatement->getClob(1);

        // Fill CLOB with data
        clob.write(dataSize, data, dataSize);
        m_statement->setClob(parameterIndex, clob);
    }
}

void COracleStatement::setBlobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    if (m_connection) {
        if (!m_createBlobStatement) {
            COracleConnection* oracleConnection = (COracleConnection*)m_connection;
            m_createBlobStatement =
                oracleConnection->createStatement("INSERT INTO sptk_lobs(sptk_blob) VALUES (empty_blob()) RETURNING sptk_blob INTO :1");
            m_createBlobStatement->registerOutParam(1, OCCIBLOB);
        }

        // Create row with empty BLOB, and return its locator for data filling
        m_createBlobStatement->executeUpdate();
        Blob blob = m_createBlobStatement->getBlob(1);

        // Fill BLOB with data
        blob.write(dataSize, data, dataSize);
        m_statement->setBlob(parameterIndex, blob);
    }
}

void COracleStatement::setParameterValues()
{
    CParamVector::iterator
        itor = m_enumeratedParams.begin(),
        iend = m_enumeratedParams.end();
        
    for (int parameterIndex = 1; itor != iend; itor++, parameterIndex++)
    {
        CParam& parameter = *(*itor);
        VariantType& priorDataType = parameter.m_binding.m_dataType;
        if (priorDataType == VAR_NONE)
            priorDataType = parameter.dataType();
        if (parameter.isNull()) {
            if (priorDataType == VAR_NONE)
                priorDataType = VAR_STRING;
            Type nativeType = COracleConnection::VariantTypeToOracleType(parameter.m_binding.m_dataType);
            //Type nativeType = OCCICHAR;
            m_statement->setNull(parameterIndex, nativeType);
        } else
        switch (priorDataType) {

            case VAR_NONE:      ///< Undefined
                throwDatabaseException("Parameter " + parameter.name() + " data type is undefined");

            case VAR_INT:       ///< Integer
                m_statement->setInt(parameterIndex, parameter.asInteger());
                break;

            case VAR_FLOAT:     ///< Floating-point (double)
                m_statement->setDouble(parameterIndex, parameter.asFloat());
                break;

            case VAR_STRING:    ///< String pointer
                m_statement->setString(parameterIndex, parameter.asString());
                break;

            case VAR_TEXT:      ///< String pointer, corresponding to CLOB in database
                setClobParameter(parameterIndex, (unsigned char*) parameter.getString(), parameter.dataSize());
                break;

            case VAR_BUFFER:    ///< Data pointer, corresponding to BLOB in database
                setBlobParameter(parameterIndex, (unsigned char*) parameter.getString(), parameter.dataSize());
                break;

            case VAR_DATE:      ///< DateTime (double)
                {
                    int16_t year, month, day;
                    parameter.asDate().decodeDate(&year, &month, &day);
                    Date dateValue(m_connection->environment(), year, month, day);
                    m_statement->setDate(parameterIndex, dateValue);
                }
                break;

            case VAR_DATE_TIME: ///< DateTime (double)
                {
                    int16_t year, month, day;
                    parameter.asDateTime().decodeDate(&year, &month, &day);
                    int16_t hour, minute, second, msecond;
                    parameter.getDateTime().decodeTime(&hour, &minute, &second, &msecond);
                    Timestamp timestampValue(m_connection->environment(),
                                             year, month, day, hour, minute, second);
                    m_statement->setTimestamp(parameterIndex, timestampValue);
                }
                break;

            case VAR_INT64:     ///< 64bit integer
                m_statement->setInt(parameterIndex, parameter.asInteger());
                break;

            case VAR_BOOL:      ///< Boolean
                m_statement->setInt(parameterIndex, parameter.asInteger());
                break;

            default:
                throwDatabaseException("Unsupported data type for parameter " + parameter.name());
        }
    }
}

void COracleStatement::execBulk(bool inTransaction, bool lastIteration)
{
    // If statement is inside the transaction, it shouldn't be in auto-commit mode
    if (inTransaction != m_state.transaction) {
        m_statement->setAutoCommit(!inTransaction);
        m_state.transaction = inTransaction;
    }

    if (m_resultSet)
        close();

    m_state.eof = true;
    m_state.columnCount = 0;

    if (lastIteration)
        m_statement->execute();
    else
        m_statement->addIteration();
}


void COracleStatement::execute(bool inTransaction)
{
    // If statement is inside the transaction, it shouldn't be in auto-commit mode
    if (inTransaction != m_state.transaction) {
        m_statement->setAutoCommit(!inTransaction);
        m_state.transaction = inTransaction;
    }

    if (m_resultSet)
        close();

    m_state.eof = true;
    m_state.columnCount = 0;

    if (m_statement->execute() == Statement::RESULT_SET_AVAILABLE) {

        m_state.eof = false;

        m_resultSet = m_statement->getResultSet();

        vector<MetaData> resultSetMetaData = m_resultSet->getColumnListMetaData();
        vector<MetaData>::iterator
            itor = resultSetMetaData.begin(),
            iend = resultSetMetaData.end();

        m_state.columnCount = resultSetMetaData.size();

        int columnIndex = 1;
        for (; itor != iend; itor++, columnIndex++) {
            const MetaData& metaData = *itor;
            // If resultSet contains cursor, use that cursor as resultSet
            if (metaData.getInt(MetaData::ATTR_DATA_TYPE) == SQLT_RSET) {
                m_resultSet->next();
                ResultSet* resultSet = m_resultSet->getCursor(columnIndex);
                m_resultSet->cancel();
                m_resultSet = resultSet;
                m_state.columnCount = m_resultSet->getColumnListMetaData().size();
                break;
            }
        }
    }
}

void COracleStatement::close()
{
    if (m_resultSet)
        m_resultSet->cancel();

    m_resultSet = NULL;
}
