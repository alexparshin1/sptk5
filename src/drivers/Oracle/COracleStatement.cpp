/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       COracleStatement.cpp - description                     ║
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

#include <sptk5/db/OracleConnection.h>
#include <sptk5/db/DatabaseField.h>

using namespace std;
using namespace sptk;
using namespace oracle::occi;

OracleStatement::OracleStatement(OracleConnection* connection, const string& sql) :
    DatabaseStatement<OracleConnection,oracle::occi::Statement>(connection),
    m_createClobStatement(nullptr),
    m_createBlobStatement(nullptr),
    m_resultSet(nullptr)
{
    m_statement = connection->createStatement(sql);
    m_state.columnCount = 0;
    m_state.eof = true;
    m_state.transaction = false;
    m_state.outputParameterCount = 0;
    m_statement->setAutoCommit(!m_state.transaction);
}

OracleStatement::~OracleStatement()
{
    if (m_statement != nullptr) {
        Connection* connection = m_statement->getConnection();
        if (m_createClobStatement)
            connection->terminateStatement(m_createClobStatement);
        if (m_createBlobStatement)
            connection->terminateStatement(m_createBlobStatement);
        connection->terminateStatement(m_statement);
    }
}

void OracleStatement::setClobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    if (m_connection) {
        if (!m_createClobStatement) {
            m_createClobStatement =
                    m_connection->createStatement("INSERT INTO sptk_lobs(sptk_clob) VALUES (empty_clob()) RETURNING sptk_clob INTO :1");
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

void OracleStatement::setBlobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    if (m_connection) {
        if (!m_createBlobStatement) {
            m_createBlobStatement =
                    m_connection->createStatement("INSERT INTO sptk_lobs(sptk_blob) VALUES (empty_blob()) RETURNING sptk_blob INTO :1");
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

void OracleStatement::setParameterValues()
{
    m_outputParamIndex.clear();

    auto itor = m_enumeratedParams.begin();
    auto iend = m_enumeratedParams.end();

    for (unsigned parameterIndex = 1; itor != iend; ++itor, parameterIndex++)
    {
        QueryParameter& parameter = *(*itor);
        VariantType& priorDataType = parameter.m_binding.m_dataType;

        if (priorDataType == VAR_NONE)
            priorDataType = parameter.dataType();

        if (!parameter.isOutput() && parameter.isNull()) {
            if (priorDataType == VAR_NONE)
                priorDataType = VAR_STRING;
            Type nativeType = OracleConnection::VariantTypeToOracleType(parameter.m_binding.m_dataType);
            //Type nativeType = OCCICHAR;
            m_statement->setNull(parameterIndex, nativeType);
            continue;
        }

        switch (priorDataType) {

            case VAR_NONE:      ///< Undefined
                throwDatabaseException("Parameter " + parameter.name() + " data type is undefined");

            case VAR_INT:       ///< Integer
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCIINT);
                    m_outputParamIndex.push_back(parameterIndex);
                } else
                    m_statement->setInt(parameterIndex, parameter.asInteger());
                break;

            case VAR_FLOAT:     ///< Floating-point (double)
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCIDOUBLE);
                    m_outputParamIndex.push_back(parameterIndex);
                } else
                    m_statement->setDouble(parameterIndex, parameter.asFloat());
                break;

            case VAR_STRING:    ///< String pointer
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCISTRING);
                    m_outputParamIndex.push_back(parameterIndex);
                } else
                    m_statement->setString(parameterIndex, parameter.asString());
                break;

            case VAR_TEXT:      ///< String pointer, corresponding to CLOB in database
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCICLOB);
                    m_outputParamIndex.push_back(parameterIndex);
                } else
                    setClobParameter(parameterIndex, (unsigned char*) parameter.getString(), (unsigned) parameter.dataSize());
                break;

            case VAR_BUFFER:    ///< Data pointer, corresponding to BLOB in database
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCIBLOB);
                    m_outputParamIndex.push_back(parameterIndex);
                } else
                    setBlobParameter(parameterIndex, (unsigned char*) parameter.getString(), (unsigned) parameter.dataSize());
                break;

            case VAR_DATE:      ///< DateTime (double)
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCIDATE);
                    m_outputParamIndex.push_back(parameterIndex);
                } else {
                    int16_t year, month, day, wday, yday;
                    parameter.asDate().decodeDate(&year, &month, &day, &wday, &yday);
                    Date dateValue(m_connection->environment(), year, (unsigned) month, (unsigned) day);
                    m_statement->setDate(parameterIndex, dateValue);
                }
                break;

            case VAR_DATE_TIME: ///< DateTime (double)
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCITIMESTAMP);
                    m_outputParamIndex.push_back(parameterIndex);
                } else {
                    int16_t year, month, day, wday, yday;
                    parameter.asDateTime().decodeDate(&year, &month, &day, &wday, &yday);
                    int16_t hour, minute, second, msecond;
                    parameter.getDateTime().decodeTime(&hour, &minute, &second, &msecond);
                    Timestamp timestampValue(m_connection->environment(),
                                             year, (unsigned) month, (unsigned) day, (unsigned) hour, (unsigned) minute, (unsigned) second);
                    m_statement->setTimestamp(parameterIndex, timestampValue);
                }
                break;

            case VAR_INT64:     ///< 64bit integer
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCIINT);
                    m_outputParamIndex.push_back(parameterIndex);
                } else
                    m_statement->setInt(parameterIndex, parameter.asInteger());
                break;

            case VAR_BOOL:      ///< Boolean
                if (parameter.isOutput()) {
                    m_statement->registerOutParam(parameterIndex, OCCIINT);
                    m_outputParamIndex.push_back(parameterIndex);
                } else
                    m_statement->setInt(parameterIndex, parameter.asInteger());
                break;

            default:
                throwDatabaseException("Unsupported data type for parameter " + parameter.name());
        }
    }
}

void OracleStatement::execBulk(bool inTransaction, bool lastIteration)
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


void OracleStatement::execute(bool inTransaction)
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
        auto itor = resultSetMetaData.begin(),
             iend = resultSetMetaData.end();

        m_state.columnCount = (unsigned) resultSetMetaData.size();

        unsigned columnIndex = 1;
        for (; itor != iend; ++itor, columnIndex++) {
            const MetaData& metaData = *itor;
            // If resultSet contains cursor, use that cursor as resultSet
            if (metaData.getInt(MetaData::ATTR_DATA_TYPE) == SQLT_RSET) {
                m_resultSet->next();
                ResultSet* resultSet = m_resultSet->getCursor(columnIndex);
                m_resultSet->cancel();
                m_resultSet = resultSet;
                m_state.columnCount = (unsigned) m_resultSet->getColumnListMetaData().size();
                break;
            }
        }
    }
}

void OracleStatement::getOutputParameters(FieldList& fields)
{
    size_t columnIndex = 0;
    for (unsigned index: m_outputParamIndex) {
        QueryParameter* parameter;
        try {
            parameter = m_enumeratedParams[index - 1];

            int         year;
            unsigned    month, day, hour, min, sec;

            DatabaseField* field = new DatabaseField(parameter->name(), (int) columnIndex, OCCIANYDATA, parameter->dataType(), 256);
            fields.push_back(field);

            switch (parameter->dataType())
            {
                case VAR_INT:
                case VAR_INT64:
                    field->setInteger(m_statement->getInt(index));
                    break;

                case VAR_FLOAT:
                    field->setFloat(m_statement->getDouble(index));
                    break;

                case VAR_DATE:
                    {
                        m_statement->getDate(index).getDate(year, month, day, hour, min, sec);
                        field->setDate(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)));
                    }
                    break;

                case VAR_DATE_TIME:
                    {
                        Timestamp timestamp = m_statement->getTimestamp(index);
                        unsigned ms;
                        timestamp.getDate(year, month, day);
                        timestamp.getTime(hour, min, sec, ms);
                        field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec)));
                    }
                    break;

                case VAR_BUFFER:
                    {
                        Blob blob = m_statement->getBlob(index);
                        blob.open(OCCI_LOB_READONLY);
                        unsigned bytes = blob.length();
                        field->checkSize(bytes);
                        blob.read(bytes,
                                  (unsigned char*) field->getBuffer(),
                                  bytes,
                                  1);
                        blob.close();
                        field->setDataSize(bytes);
                    }
                    break;

                case VAR_TEXT:
                    {
                        Clob clob = m_statement->getClob(index);
                        clob.open(OCCI_LOB_READONLY);
                        // Attention: clob stored as widechar
                        unsigned clobChars = clob.length();
                        unsigned clobBytes = clobChars * 4;
                        field->checkSize(clobBytes);
                        unsigned bytes = clob.read(clobChars,
                                                   (unsigned char*) field->getBuffer(),
                                                   clobBytes,
                                                   1);
                        clob.close();
                        field->setDataSize(bytes);
                    }
                    break;

                default:
                    field->setString(m_statement->getString(index));
                    break;
            }

        } catch (exception& e) {
            throw DatabaseException("Can't read parameter " + parameter->name() + ": " + string(e.what()));
        }
        columnIndex++;
    }
}

void OracleStatement::close()
{
    if (m_resultSet)
        m_resultSet->cancel();

    m_resultSet = nullptr;
}
