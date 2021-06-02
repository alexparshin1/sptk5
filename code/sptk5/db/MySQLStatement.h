/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#pragma once

#include <mysql.h>

#include <list>
#include <string>
#include <cstdio>

#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/DatabaseStatement.h>

namespace sptk
{

class MySQLConnection;
class MySQLStatementField;

/**
 * MySQL statement wrapper
 */
class MySQLStatement : public DatabaseStatement<MySQLConnection,MYSQL_STMT>
{
public:
    /**
     * Translates MySQL native type to CVariant type
     * @param mysqlType enum_field_types, MySQL native type
     * @returns CVariant type
     */
    static VariantType mySQLTypeToVariantType(enum_field_types mysqlType);

    /**
     * Translates CVariant type to MySQL native type
     * @param dataType VariantType&, CVariant type
     * @returns MySQL native type
     */
    static enum_field_types variantTypeToMySQLType(VariantType dataType);

    /**
     * Translates DateTime to MySQL time
     * @param mysqlDate MYSQL_TIME&, MySQL time
     * @param timestamp DateTime, Timestamp
     * @param timeType VariantType, Time type, VAR_DATE or VAR_DATETIME
     */
    static void dateTimeToMySQLDate(MYSQL_TIME& mysqlDate, DateTime timestamp, VariantType timeType);

    /**
     * Translates MySQL time to DateTime
     * @param timestamp DateTime&, Timestamp
     * @param mysqlDate const MYSQL_TIME&, MySQL time
     */
    static void mysqlDateToDateTime(DateTime& timestamp, const MYSQL_TIME& mysqlDate);

    /**
     * Constructor
     * @param connection Connection*, MySQL connection
     * @param sql std::string, SQL statement
     * @param autoPrepare bool, If true then statement is executed as prepared.
     */
    MySQLStatement(MySQLConnection* connection, String sql, bool autoPrepare);

    /**
     * Deleted copy constructor
     */
    MySQLStatement(const MySQLStatement&) = delete;

    /**
     * Move constructor
     */
    MySQLStatement(MySQLStatement&&) = default;

    /**
     * Destructor
     */
    ~MySQLStatement() override;

    /**
     * Deleted copy assignment
     */
    MySQLStatement& operator = (const MySQLStatement&) = delete;

    /**
     * Move assignment
     */
    MySQLStatement& operator = (MySQLStatement&&) = default;

    /**
     * Generates normalized list of parameters
     * @param queryParams CParamList&, Standard query parameters
     */
    void enumerateParams(QueryParameterList& queryParams) override;

    /**
     * Sets actual parameter values for the statement execution
     */
    void setParameterValues() override;

    /**
     * Prepares MySQL statement
     * @param sql const std::string, statement SQL
     */
    void prepare(const String& sql) const;

    /**
     * Executes statement
     */
    void execute(bool) override;

    /**
     * Binds statement result metadata to query fields
     * @param fields CFieldList&, query fields (if any)
     */
    void bindResult(FieldList& fields);

    /**
     * Fetches statement result metadata to query fields
     * @param fields CFieldList&, query fields (if any)
     */
    void readResultRow(FieldList& fields);

    /**
     * Closes statement and releases allocated resources
     */
    void close() override;

    /**
     * Fetches next record
     */
    void fetch() override;

private:

    String                          m_sql;              ///< Statement SQL
    std::vector<MYSQL_BIND>         m_paramBuffers;     ///< Parameter binding buffers
    std::vector<unsigned long>      m_paramLengths;     ///< Parameter data lengths
    std::vector<MYSQL_BIND>         m_fieldBuffers;     ///< Fetch data buffers
    MYSQL_RES*                      m_result {nullptr}; ///< Statement handle
    MYSQL_ROW                       m_row {};           ///< Fetch data row

    /**
     * Reads not prepared statement result row to query fields
     * @param fields CFieldList&, query fields (if any)
     */
    void readUnpreparedResultRow(FieldList& fields);

    /**
     * Reads prepared statement result row to query fields
     * @param fields CFieldList&, query fields (if any)
     */
    void readPreparedResultRow(FieldList& fields);

    /**
     * Convert MySQL time data to field
     * @param _field             Output field
     * @param mysqlTime         MySQL time
     * @param fieldType         Field type (date or datetime)
     */
    static void decodeMySQLTime(Field* _field, const MYSQL_TIME& mysqlTime, VariantType fieldType);

    /**
     * Convert MySQL float data to field
     * @param _field             Output field
     * @param bind         MySQL field bind
     * @param fieldType         Field type (date or datetime)
     */
    static void decodeMySQLFloat(Field* _field, MYSQL_BIND& bind);

    [[noreturn]] void throwMySQLError() const
    {
        throw DatabaseException(mysql_stmt_error(statement()));
    }

    [[nodiscard]] bool bindVarCharField(MYSQL_BIND& bind, MySQLStatementField* field, size_t fieldIndex, uint32_t dataLength) const;
};

}

