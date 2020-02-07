/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        OracleStatement.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_ORACLESTATEMENT_H__
#define __SPTK_ORACLESTATEMENT_H__

#include <occi.h>

#include <list>
#include <string>
#include <cstdio>

#include <sptk5/db/DatabaseStatement.h>
#include <sptk5/FieldList.h>
#include "DatabaseField.h"

namespace sptk
{

class OracleConnection;

/**
 * Oracle statement
 */
class OracleStatement : public DatabaseStatement<OracleConnection,oracle::occi::Statement>
{
public:

    typedef oracle::occi::Connection    Connection;         ///< Oracle connection type
    typedef oracle::occi::Statement     Statement;          ///< Oracle statement type
    typedef oracle::occi::ResultSet     ResultSet;          ///< Oracle result set type
    typedef oracle::occi::MetaData      MetaData;           ///< Oracle result set metdata type

private:

    Statement*          m_createClobStatement {nullptr};    ///< Statement for creating CLOBs
    Statement*          m_createBlobStatement {nullptr};    ///< Statement for creating BLOBs
    ResultSet*          m_resultSet {nullptr};              ///< Result set (if returned by statement)

    /*
     * Index of output parameters
     */
    std::vector<unsigned> m_outputParamIndex;

    /**
     * @brief Sets character data to a CLOB parameter
     * @param parameterIndex uint32_t, Parameter index
     * @param data unsigned char*, Character data buffer
     * @param dataSize uint32_t, Character data size
     */
    void setClobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize);

    /**
     * @brief Sets binary data to a BLOB parameter
     * @param parameterIndex uint32_t, Parameter index
     * @param data unsigned char*, Binary data buffer
     * @param dataSize uint32_t, Binary data size
     */
    void setBlobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize);

public:
    /**
     * @brief Constructor
     * @param connection Connection*, Oracle connection
     * @param sql std::string, SQL statement
     */
    OracleStatement(OracleConnection* connection, const std::string& sql);

    /**
     * @brief Destructor
     */
    ~OracleStatement() override;

    /**
     * @brief Sets actual parameter values for the statement execution
     */
    void setParameterValues() override;

    /**
     * @brief Executes statement
     * @param inTransaction bool, True if statement is executed from transaction
     */
    void execute(bool inTransaction) override;

    /**
     * @brief Executes statement in bulk mode
     * @param inTransaction bool, True if statement is executed from transaction
     * @param lastIteration bool, True if bulk operation is completed (all iterations added)
     */
    void execBulk(bool inTransaction, bool lastIteration);

    /**
     * @brief Closes statement and releases allocated resources
     */
    void close() override;

    /**
     * @brief Fetches next record
     */
    void fetch() override
    {
        if (m_resultSet)
            state().eof = (m_resultSet->next() == ResultSet::END_OF_FETCH);
    }

    /**
     * @brief Returns result set (if returned by a statement)
     */
    ResultSet* resultSet()
    {
        return m_resultSet;
    }

    void getOutputParameters(FieldList& fields);

private:

    /**
     * Read BLOB field
     * @param index             Column number
     * @param field             Field
     */
    void getBLOBOutputParameter(unsigned int index, DatabaseField* field);

    /**
     * Read CLOB field
     * @param index             Column number
     * @param field             Field
     */
    void getCLOBOutputParameter(unsigned int index, sptk::DatabaseField* field);

    /**
     * Set CLOB parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setCLOBParameterValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set BLOB parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setBLOBParameterValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set Date parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setDateParameterValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set DateTime parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setDateTimeParameterValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set int64 parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setInt64ParamValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set bool parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setBooleanParamValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set string parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setStringParamValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set float parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setFloatParamValue(unsigned int parameterIndex, const QueryParameter& parameter);

    /**
     * Set int parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setIntParamValue(unsigned int parameterIndex, const QueryParameter& parameter);

    void getDateOutputParameter(unsigned int index, DatabaseField* field);

    void getDateTimeOutputParameter(unsigned int index, DatabaseField* field);
};

}

#endif
