/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

using namespace std;
using namespace sptk;
using namespace oracle::occi;

OracleStatement::OracleStatement(OracleConnection* connection, const string& sql)
: DatabaseStatement<OracleConnection, oracle::occi::Statement>(connection)
{
    statement(connection->createStatement(sql));
    state().columnCount = 0;
    state().eof = true;
    state().transaction = false;
    state().outputParameterCount = 0;
    statement()->setAutoCommit(!state().transaction);
}

OracleStatement::~OracleStatement()
{
    if (statement() != nullptr) {
        Connection* connection = statement()->getConnection();
        if (m_createClobStatement)
            connection->terminateStatement(m_createClobStatement);
        if (m_createBlobStatement)
            connection->terminateStatement(m_createBlobStatement);
        connection->terminateStatement(statement());
    }
}

void OracleStatement::setClobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    if (connection()) {
        if (!m_createClobStatement) {
            m_createClobStatement =
                    connection()->createStatement(
                            "INSERT INTO sptk_lobs(sptk_clob) VALUES (empty_clob()) RETURNING sptk_clob INTO :1");
            m_createClobStatement->registerOutParam(1, OCCICLOB);
        }

        // Create row with empty CLOB, and return its locator for data filling
        m_createClobStatement->executeUpdate();
        Clob clob = m_createClobStatement->getClob(1);

        // Fill CLOB with data
        clob.write(dataSize, data, dataSize);
        statement()->setClob(parameterIndex, clob);
    }
}

void OracleStatement::setBlobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    if (connection()) {
        if (!m_createBlobStatement) {
            m_createBlobStatement =
                    connection()->createStatement(
                            "INSERT INTO sptk_lobs(sptk_blob) VALUES (empty_blob()) RETURNING sptk_blob INTO :1");
            m_createBlobStatement->registerOutParam(1, OCCIBLOB);
        }

        // Create row with empty BLOB, and return its locator for data filling
        m_createBlobStatement->executeUpdate();
        Blob blob = m_createBlobStatement->getBlob(1);

        // Fill BLOB with data
        blob.write(dataSize, data, dataSize);
        statement()->setBlob(parameterIndex, blob);
    }
}

void OracleStatement::setParameterValues()
{
    m_outputParamIndex.clear();

    unsigned parameterIndex = 1;
    for (auto* parameterPtr: enumeratedParams()) {
        QueryParameter& parameter = *parameterPtr;
        VariantType& priorDataType = parameter.binding().m_dataType;

        if (priorDataType == VAR_NONE)
            priorDataType = parameter.dataType();

        if (!parameter.isOutput() && parameter.isNull()) {
            if (priorDataType == VAR_NONE)
                priorDataType = VAR_STRING;
            Type nativeType = VariantTypeToOracleType(parameter.binding().m_dataType);
            statement()->setNull(parameterIndex, nativeType);
            ++parameterIndex;
            continue;
        }

        switch (priorDataType) {

            case VAR_NONE:      ///< Undefined
            throwDatabaseException("Parameter " + parameter.name() + " data type is undefined")

            case VAR_INT:       ///< Integer
                setIntParamValue(parameterIndex, parameter);
                break;

            case VAR_FLOAT:     ///< Floating-point (double)
                setFloatParamValue(parameterIndex, parameter);
                break;

            case VAR_STRING:    ///< String pointer
                setStringParamValue(parameterIndex, parameter);
                break;

            case VAR_TEXT:      ///< String pointer, corresponding to CLOB in database
                setCLOBParameterValue(parameterIndex, parameter);
                break;

            case VAR_BUFFER:    ///< Data pointer, corresponding to BLOB in database
                setBLOBParameterValue(parameterIndex, parameter);
                break;

            case VAR_DATE:      ///< DateTime (double)
                setDateParameterValue(parameterIndex, parameter);
                break;

            case VAR_DATE_TIME: ///< DateTime (double)
                setDateTimeParameterValue(parameterIndex, parameter);
                break;

            case VAR_INT64:     ///< 64bit integer
                setInt64ParamValue(parameterIndex, parameter);
                break;

            case VAR_BOOL:      ///< Boolean
                setBooleanParamValue(parameterIndex, parameter);
                break;

            default: throwDatabaseException("Unsupported data type for parameter " + parameter.name())
        }
        ++parameterIndex;
    }
}

void OracleStatement::setIntParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCIINT);
        m_outputParamIndex.push_back(parameterIndex);
    } else
        statement()->setInt(parameterIndex, parameter.asInteger());
}

void OracleStatement::setFloatParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCIDOUBLE);
        m_outputParamIndex.push_back(parameterIndex);
    } else
        statement()->setDouble(parameterIndex, parameter.asFloat());
}

void OracleStatement::setStringParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCISTRING);
        m_outputParamIndex.push_back(parameterIndex);
    } else
        statement()->setString(parameterIndex, parameter.asString());
}

void OracleStatement::setBooleanParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCIINT);
        m_outputParamIndex.push_back(parameterIndex);
    } else
        statement()->setInt(parameterIndex, parameter.asInteger());
}

void OracleStatement::setInt64ParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCIINT);
        m_outputParamIndex.push_back(parameterIndex);
    } else
        statement()->setInt(parameterIndex, parameter.asInteger());
}

void OracleStatement::setDateTimeParameterValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCITIMESTAMP);
        m_outputParamIndex.push_back(parameterIndex);
    } else {
        int16_t year;
        int16_t month;
        int16_t day;
        int16_t wday;
        int16_t yday;
        parameter.asDateTime().decodeDate(&year, &month, &day, &wday, &yday);
        int16_t hour;
        int16_t minute;
        int16_t second;
        int16_t msecond;
        parameter.getDateTime().decodeTime(&hour, &minute, &second, &msecond);
        Timestamp timestampValue(connection()->environment(),
                                 year, (unsigned) month, (unsigned) day, (unsigned) hour, (unsigned) minute,
                                 (unsigned) second);
        statement()->setTimestamp(parameterIndex, timestampValue);
    }
}

void OracleStatement::setDateParameterValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCIDATE);
        m_outputParamIndex.push_back(parameterIndex);
    } else {
        int16_t year;
        int16_t month;
        int16_t day;
        int16_t wday;
        int16_t yday;
        parameter.asDate().decodeDate(&year, &month, &day, &wday, &yday);
        Date dateValue(connection()->environment(), year, (unsigned) month, (unsigned) day);
        statement()->setDate(parameterIndex, dateValue);
    }
}

void OracleStatement::setBLOBParameterValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCIBLOB);
        m_outputParamIndex.push_back(parameterIndex);
    } else
        setBlobParameter(parameterIndex, (unsigned char*) parameter.getBuffer(), (unsigned) parameter.dataSize());
}

void OracleStatement::setCLOBParameterValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    if (parameter.isOutput()) {
        statement()->registerOutParam(parameterIndex, OCCICLOB);
        m_outputParamIndex.push_back(parameterIndex);
    } else
        setClobParameter(parameterIndex, (unsigned char*) parameter.getBuffer(), (unsigned) parameter.dataSize());
}

void OracleStatement::execBulk(bool inTransaction, bool lastIteration)
{
    // If statement is inside the transaction, it shouldn't be in auto-commit mode
    if (inTransaction != state().transaction) {
        statement()->setAutoCommit(!inTransaction);
        state().transaction = inTransaction;
    }

    if (m_resultSet)
        close();

    state().eof = true;
    state().columnCount = 0;

    if (lastIteration)
        statement()->execute();
    else
        statement()->addIteration();
}


void OracleStatement::execute(bool inTransaction)
{
    // If statement is inside the transaction, it shouldn't be in auto-commit mode
    if (inTransaction != state().transaction) {
        statement()->setAutoCommit(!inTransaction);
        state().transaction = inTransaction;
    }

    if (m_resultSet)
        close();

    state().eof = true;
    state().columnCount = 0;

    auto rc = statement()->execute();
    if (rc == Statement::RESULT_SET_AVAILABLE) {

        state().eof = false;

        m_resultSet = statement()->getResultSet();

        vector<MetaData> resultSetMetaData = m_resultSet->getColumnListMetaData();

        state().columnCount = (unsigned) resultSetMetaData.size();

        unsigned columnIndex = 1;
        for (const MetaData& metaData: resultSetMetaData) {
            // If resultSet contains cursor, use that cursor as resultSet
            if (metaData.getInt(MetaData::ATTR_DATA_TYPE) == SQLT_RSET) {
                m_resultSet->next();
                ResultSet* resultSet = m_resultSet->getCursor(columnIndex);
                m_resultSet->cancel();
                m_resultSet = resultSet;
                state().columnCount = (unsigned) m_resultSet->getColumnListMetaData().size();
                break;
            }
            ++columnIndex;
        }
    }
}

void OracleStatement::getBLOBOutputParameter(unsigned int index, DatabaseField* field) const
{
    Blob blob = statement()->getBlob(index);
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

void OracleStatement::getCLOBOutputParameter(unsigned int index, DatabaseField* field) const
{
    Clob clob = statement()->getClob(index);
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

void OracleStatement::getOutputParameters(FieldList& fields)
{
    size_t columnIndex = 0;
    for (unsigned index: m_outputParamIndex) {
        const QueryParameter* parameter;
        try {
            parameter = enumeratedParams()[index - 1];

            auto* field = (DatabaseField*) fields.findField(parameter->name());
            if (field == nullptr) {
                field = new DatabaseField(parameter->name(), (int) fields.size(), OCCIANYDATA,
                                          parameter->dataType(), 256);
                fields.push_back(field);
            }

            switch (parameter->dataType()) {
                case VAR_INT:
                case VAR_INT64:
                    field->setInteger(statement()->getInt(index));
                    break;

                case VAR_FLOAT:
                    field->setFloat(statement()->getDouble(index));
                    break;

                case VAR_DATE:
                    getDateOutputParameter(index, field);
                    break;

                case VAR_DATE_TIME:
                    getDateTimeOutputParameter(index, field);
                    break;

                case VAR_BUFFER:
                    getBLOBOutputParameter(index, field);
                    break;

                case VAR_TEXT:
                    getCLOBOutputParameter(index, field);
                    break;

                default:
                    field->setString(statement()->getString(index));
                    break;
            }

        } catch (const Exception& e) {
            throw DatabaseException("Can't read parameter " + parameter->name() + ": " + string(e.what()));
        } catch (const SQLException& e) {
            throw DatabaseException("Can't read parameter " + parameter->name() + ": " + string(e.what()));
        }
        ++columnIndex;
    }
}

void OracleStatement::getDateTimeOutputParameter(unsigned int index, DatabaseField* field) const
{
    int      year;
    unsigned month;
    unsigned day;
    unsigned hour;
    unsigned min;
    unsigned sec;

    Timestamp timestamp = statement()->getTimestamp(index);
    unsigned ms;
    timestamp.getDate(year, month, day);
    timestamp.getTime(hour, min, sec, ms);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec)));
}

void OracleStatement::getDateOutputParameter(unsigned int index, DatabaseField* field) const
{
    int year;
    unsigned month;
    unsigned day;
    unsigned hour;
    unsigned min;
    unsigned sec;
    statement()->getDate(index).getDate(year, month, day, hour, min, sec);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)), true);
}

void OracleStatement::close()
{
    if (m_resultSet)
        m_resultSet->cancel();

    m_resultSet = nullptr;
}
