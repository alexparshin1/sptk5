/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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

#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/OracleOciStatement.h>

using namespace std;
using namespace sptk;
using namespace ocilib;

OracleOciStatement::OracleOciStatement(OracleOciConnection* connection, const string& sql)
    : DatabaseStatement<OracleOciConnection, ocilib::Statement>(connection)
    , m_ociStatement(make_shared<Statement>(*connection->connection()))
    , m_sql(sql)
{
    statement(m_ociStatement.get());
    state().columnCount = 0;
    state().eof = true;
    state().transaction = false;
    state().outputParameterCount = 0;
}

OracleOciStatement::~OracleOciStatement()
{
    if (statement() != nullptr)
    {
        m_ociStatement.reset();
    }
}

void OracleOciStatement::setClobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    /*
    if (connection())
    {
        if (!m_createClobStatement)
        {
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
     */
}

void OracleOciStatement::setBlobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize)
{
    /*
    if (connection())
    {
        if (!m_createBlobStatement)
        {
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
     */
}

void OracleOciStatement::setParameterValues()
{
    /*
    m_outputParamIndex.clear();

    unsigned parameterIndex = 1;
    for (const auto& parameterPtr: enumeratedParams())
    {
        QueryParameter& parameter = *parameterPtr;
        VariantDataType& paramDataType = parameter.binding().m_dataType;

        paramDataType = parameter.dataType();

        if (!parameter.isOutput() && parameter.isNull())
        {
            const Type nativeType = VariantTypeToOracleOciType(parameter.binding().m_dataType);
            statement()->setNull(parameterIndex, nativeType);
            ++parameterIndex;
            continue;
        }

        switch (paramDataType)
        {
            case VariantDataType::VAR_INT: ///< Integer
                setIntParamValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_FLOAT: ///< Floating-point (double)
                setFloatParamValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_STRING: ///< String pointer
                setStringParamValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_TEXT: ///< String pointer, corresponding to CLOB in database
                setCLOBParameterValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_BUFFER: ///< Data pointer, corresponding to BLOB in database
                setBLOBParameterValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_DATE: ///< DateTime (double)
                setDateParameterValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_DATE_TIME: ///< DateTime (double)
                setDateTimeParameterValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_INT64: ///< 64bit integer
                setInt64ParamValue(parameterIndex, parameter);
                break;

            case VariantDataType::VAR_BOOL: ///< Boolean
                setBooleanParamValue(parameterIndex, parameter);
                break;

            default:
                throw DatabaseException(
                    "Unsupported parameter type(" + to_string((int) parameter.dataType()) + ") for parameter '" +
                    parameter.name() + "'");
        }
        ++parameterIndex;
    }
     */
}

void OracleOciStatement::setIntParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCIINT);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        statement()->setInt(parameterIndex, parameter.asInteger());
    }
     */
}

void OracleOciStatement::setFloatParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCIDOUBLE);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        statement()->setDouble(parameterIndex, parameter.asFloat());
    }
     */
}

void OracleOciStatement::setStringParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCISTRING);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        statement()->setString(parameterIndex, parameter.asString());
    }
     */
}

void OracleOciStatement::setBooleanParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCIINT);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        statement()->setInt(parameterIndex, parameter.asInteger());
    }
     */
}

void OracleOciStatement::setInt64ParamValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCIINT);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        statement()->setInt(parameterIndex, parameter.asInteger());
    }
     */
}

void OracleOciStatement::setDateTimeParameterValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCITIMESTAMP);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        int16_t year {0};
        int16_t month {0};
        int16_t day {0};
        int16_t wday {0};
        int16_t yday {0};
        parameter.asDateTime().decodeDate(&year, &month, &day, &wday, &yday);
        int16_t hour {0};
        int16_t minute {0};
        int16_t second {0};
        int16_t msecond {0};
        parameter.get<DateTime>().decodeTime(&hour, &minute, &second, &msecond);
        const Timestamp timestampValue(connection()->environment(),
                                       year, (unsigned) month, (unsigned) day, (unsigned) hour, (unsigned) minute,
                                       (unsigned) second);
        statement()->setTimestamp(parameterIndex, timestampValue);
    }
     */
}

void OracleOciStatement::setDateParameterValue(unsigned int parameterIndex, const QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCIDATE);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        int16_t year {0};
        int16_t month {0};
        int16_t day {0};
        int16_t wday {0};
        int16_t yday {0};
        parameter.asDate().decodeDate(&year, &month, &day, &wday, &yday);
        const Date dateValue(connection()->environment(), year, (unsigned) month, (unsigned) day);
        statement()->setDate(parameterIndex, dateValue);
    }
     */
}

void OracleOciStatement::setBLOBParameterValue(unsigned int parameterIndex, QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCIBLOB);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        setBlobParameter(parameterIndex, parameter.get<Buffer>().data(), (unsigned) parameter.dataSize());
    }
     */
}

void OracleOciStatement::setCLOBParameterValue(unsigned int parameterIndex, QueryParameter& parameter)
{
    /*
    if (parameter.isOutput())
    {
        statement()->registerOutParam(parameterIndex, OCCICLOB);
        m_outputParamIndex.push_back(parameterIndex);
    }
    else
    {
        setClobParameter(parameterIndex, parameter.get<Buffer>().data(), (unsigned) parameter.dataSize());
    }
     */
}

void OracleOciStatement::execBulk(bool inTransaction, bool lastIteration)
{
    /*
    // If statement is inside the transaction, it shouldn't be in auto-commit mode
    if (inTransaction != state().transaction)
    {
        statement()->setAutoCommit(!inTransaction);
        state().transaction = inTransaction;
    }

    if (m_resultSet)
    {
        close();
    }

    state().eof = true;
    state().columnCount = 0;

    if (lastIteration)
    {
        statement()->execute();
    }
    else
    {
        statement()->addIteration();
    }
     */
}


void OracleOciStatement::execute(bool inTransaction)
{
    /*
    // If statement is inside the transaction, it shouldn't be in auto-commit mode
    if (inTransaction != state().transaction)
    {
        statement()->setAutoCommit(!inTransaction);
        state().transaction = inTransaction;
    }
    */

    state().eof = true;
    state().columnCount = 0;

    statement()->Execute(m_sql);
    auto resultSet = statement()->GetResultset();
    if (resultSet)
    {
        state().eof = false;
        state().columnCount = (unsigned) resultSet.GetColumnCount();
    }
}

void OracleOciStatement::getBLOBOutputParameter(unsigned int index, const SDatabaseField& field) const
{
    /*
    Blob blob = statement()->getBlob(index);
    blob.open(OCCI_LOB_READONLY);
    const unsigned bytes = blob.length();
    field->checkSize(bytes);
    blob.read(bytes, field->get<Buffer>().data(), bytes, 1);
    blob.close();
    field->setDataSize(bytes);
     */
}

void OracleOciStatement::getCLOBOutputParameter(unsigned int index, const SDatabaseField& field) const
{
    /*
    Clob clob = statement()->getClob(index);
    clob.open(OCCI_LOB_READONLY);
    // Attention: clob stored as widechar
    const unsigned clobChars = clob.length();
    const unsigned clobBytes = clobChars * 4;
    field->checkSize(clobBytes);
    const unsigned bytes = clob.read(clobChars, field->get<Buffer>().data(), clobBytes, 1);
    clob.close();
    field->setDataSize(bytes);
     */
}

void OracleOciStatement::getOutputParameters(FieldList& fields)
{
    /*
    for (const unsigned index: m_outputParamIndex)
    {
        SQueryParameter parameter;
        try
        {
            parameter = enumeratedParams()[index - 1];

            auto field = dynamic_pointer_cast<DatabaseField>(fields.findField(parameter->name()));
            if (!field)
            {
                field = make_shared<DatabaseField>(parameter->name(), OCCIANYDATA,
                                                   parameter->dataType(), 256);
                fields.push_back(field);
            }

            switch (parameter->dataType())
            {
                case VariantDataType::VAR_INT:
                case VariantDataType::VAR_INT64:
                    field->setInteger(statement()->getInt(index));
                    break;

                case VariantDataType::VAR_FLOAT:
                    field->setFloat(statement()->getDouble(index));
                    break;

                case VariantDataType::VAR_DATE:
                    getDateOutputParameter(index, field);
                    break;

                case VariantDataType::VAR_DATE_TIME:
                    getDateTimeOutputParameter(index, field);
                    break;

                case VariantDataType::VAR_BUFFER:
                    getBLOBOutputParameter(index, field);
                    break;

                case VariantDataType::VAR_TEXT:
                    getCLOBOutputParameter(index, field);
                    break;

                default:
                    field->setString(statement()->getString(index));
                    break;
            }
        }
        catch (const Exception& e)
        {
            throw DatabaseException("Can't read parameter " + parameter->name() + ": " + string(e.what()));
        }
        catch (const SQLException& e)
        {
            throw DatabaseException("Can't read parameter " + parameter->name() + ": " + string(e.what()));
        }
    }
     */
}

void OracleOciStatement::getDateTimeOutputParameter(unsigned int index, const SDatabaseField& field) const
{
    /*
    int year {0};
    unsigned month {0};
    unsigned day {0};
    unsigned hour {0};
    unsigned min {0};
    unsigned sec {0};

    const Timestamp timestamp = statement()->getTimestamp(index);
    unsigned ms;
    timestamp.getDate(year, month, day);
    timestamp.getTime(hour, min, sec, ms);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec)));
     */
}

void OracleOciStatement::getDateOutputParameter(unsigned int index, const SDatabaseField& field) const
{
    /*
    int year {0};
    unsigned month {0};
    unsigned day {0};
    unsigned hour {0};
    unsigned min {0};
    unsigned sec {0};
    statement()->getDate(index).getDate(year, month, day, hour, min, sec);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)), true);
     */
}

void OracleOciStatement::close()
{
}

void OracleOciStatement::fetch()
{
    auto resultSet = m_ociStatement->GetResultset();
    if (resultSet)
    {
        state().eof = !resultSet.Next();
    }
}

void OracleOciStatement::enumerateParams(QueryParameterList& queryParams)
{
    DatabaseStatement::enumerateParams(queryParams);
}
