/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#pragma once

#include <occi.h>

#include <cstdio>
#include <list>
#include <string>

#include "DatabaseField.h"
#include "OracleOciParameterBuffer.h"
#include <sptk5/FieldList.h>
#include <sptk5/db/DatabaseStatement.h>

namespace sptk {

class OracleOciConnection;

/**
 * OracleOci statement
 */
class OracleOciStatement
    : public DatabaseStatement<OracleOciConnection, ocilib::Statement>
{
    friend class OracleOciConnection;

public:
    using Connection = ocilib::Connection; ///< OracleOci connection type
    using Statement = ocilib::Statement;   ///< OracleOci statement type

    /**
     * Constructor
     * @param connection Connection*, OracleOci connection
     * @param sql std::string, SQL statement
     */
    OracleOciStatement(OracleOciConnection* connection, const std::string& sql);

    /**
     * Deleted copy constructor
     */
    OracleOciStatement(const OracleOciStatement&) = delete;

    /**
     * Move constructor
     */
    OracleOciStatement(OracleOciStatement&&) = default;

    /**
     * Destructor
     */
    ~OracleOciStatement() override;

    /**
     * Deleted copy assignment
     */
    OracleOciStatement& operator=(const OracleOciStatement&) = delete;

    /**
     * Move assignment
     */
    OracleOciStatement& operator=(OracleOciStatement&&) = default;

    /**
     * Sets actual parameter values for the statement execution
     */
    void setParameterValues() override;

    /**
     * Executes statement
     * @param inTransaction bool, True if statement is executed from transaction
     */
    void execute(bool inTransaction) override;

    /**
     * Executes statement in bulk mode
     * @param inTransaction bool, True if statement is executed from transaction
     * @param lastIteration bool, True if bulk operation is completed (all iterations added)
     */
    void execBulk(bool inTransaction, bool lastIteration);

    /**
     * Closes statement and releases allocated resources
     */
    void close() override;

    /**
     * Fetches next record
     */
    void fetch() override;

    /**
     * Returns result set (if returned by a statement)
     */
    ocilib::Resultset resultSet()
    {
        return m_ociStatement->GetResultset();
    }

    void getOutputParameters(FieldList& fields);
    void enumerateParams(QueryParameterList& queryParams) override;

protected:
    void bindParameters();

private:
    std::shared_ptr<Connection> m_ociConnection; ///< Connection
    std::shared_ptr<Statement> m_ociStatement;   ///< Statement
    String m_sql;                                ///< SQL
    Statement* m_createClobStatement {nullptr};  ///< Statement for creating CLOBs
    Statement* m_createBlobStatement {nullptr};  ///< Statement for creating BLOBs
    bool m_prepared {false};                     ///< True if statement is prepared
    std::vector<std::shared_ptr<OracleOciParameterBuffer>> m_parameterBinding;

    /*
     * Index of output parameters
     */
    std::vector<unsigned> m_outputParamIndex;

    /**
     * Sets character data to a CLOB parameter
     * @param parameterIndex uint32_t, Parameter index
     * @param data unsigned char*, Character data buffer
     * @param dataSize uint32_t, Character data size
     */
    void setClobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize);

    /**
     * Sets binary data to a BLOB parameter
     * @param parameterIndex uint32_t, Parameter index
     * @param data unsigned char*, Binary data buffer
     * @param dataSize uint32_t, Binary data size
     */
    void setBlobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize);

    /**
     * Read BLOB field
     * @param index             Column number
     * @param field             Field
     */
    void getBLOBOutputParameter(unsigned int index, const SDatabaseField& field) const;

    /**
     * Read CLOB field
     * @param index             Column number
     * @param field             Field
     */
    void getCLOBOutputParameter(unsigned int index, const SDatabaseField& field) const;

    /**
     * Set CLOB parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setCLOBParameterValue(unsigned int parameterIndex, QueryParameter& parameter);

    /**
     * Set BLOB parameter value
     * @param parameterIndex    Parameter number
     * @param parameter         Query parameter
     */
    void setBLOBParameterValue(unsigned int parameterIndex, QueryParameter& parameter);

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
    void setStringParamValue(const ocilib::ostring& paramMark, unsigned int parameterIndex, const QueryParameter& parameter);

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
    void setIntParamValue(const ocilib::ostring& parameterMark, unsigned int parameterIndex, QueryParameter& parameter);

    void getDateOutputParameter(unsigned int index, const SDatabaseField& field) const;

    void getDateTimeOutputParameter(unsigned int index, const SDatabaseField& field) const;
};

} // namespace sptk
