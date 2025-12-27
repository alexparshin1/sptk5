/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
    ocilib::Resultset resultSet() const
    {
        return m_ociStatement->GetResultset();
    }

    void enumerateParams(QueryParameterList& queryParams) override;

protected:
    void bindParameters();

private:
    std::shared_ptr<Connection>                            m_ociConnection; ///< Connection
    std::shared_ptr<Statement>                             m_ociStatement;  ///< Statement
    String                                                 m_sql;           ///< SQL
    std::vector<std::shared_ptr<OracleOciParameterBuffer>> m_parameterBinding;

    /*
     * Index of output parameters
     */
    std::vector<unsigned> m_outputParamIndex;
};

} // namespace sptk
