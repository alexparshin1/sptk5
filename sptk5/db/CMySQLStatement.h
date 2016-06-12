/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        CMySQLStatement.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CMYSQLSTATEMENT_H__
#define __CMYSQLSTATEMENT_H__

#include <mysql.h>

#include <list>
#include <string>
#include <stdio.h>

#include <sptk5/db/CDatabaseField.h>
#include <sptk5/db/CDatabaseStatement.h>

namespace sptk
{

class CMySQLConnection;

class CMySQLStatement : public CDatabaseStatement<CMySQLConnection,MYSQL_STMT>
{
    std::string                     m_sql;
    std::vector<MYSQL_BIND>         m_paramBuffers;
    std::vector<unsigned long>      m_paramLengths;
    std::vector<MYSQL_BIND>         m_fieldBuffers;

    MYSQL_RES*                      m_result;
    MYSQL_ROW                       m_row;

    /// @brief Reads not prepared statement result row to query fields
    /// @param fields CFieldList&, query fields (if any)
    void readUnpreparedResultRow(CFieldList& fields);

    /// @brief Reads prepared statement result row to query fields
    /// @param fields CFieldList&, query fields (if any)
    void readPreparedResultRow(CFieldList& fields);
    
public:

    /// @brief Translates MySQL native type to CVariant type
    /// @param mysqlType enum_field_types, MySQL native type
    /// @returns CVariant type
    static CVariantType mySQLTypeToVariantType(enum_field_types mysqlType);

    /// @brief Translates CVariant type to MySQL native type
    /// @param dataType CVariantType&, CVariant type
    /// @returns MySQL native type
    static enum_field_types variantTypeToMySQLType(CVariantType dataType);

    /// @brief Translates CDateTime to MySQL time
    /// @param mysqlDate MYSQL_TIME&, MySQL time
    /// @param timestamp CDateTime, Timestamp
    /// @param timeType CVariantType, Time type, VAR_DATE or VAR_DATETIME
    static void dateTimeToMySQLDate(MYSQL_TIME& mysqlDate, CDateTime timestamp, CVariantType timeType);

    /// @brief Translates MySQL time to CDateTime
    /// @param timestamp CDateTime&, Timestamp
    /// @param mysqlDate const MYSQL_TIME&, MySQL time
    static void mysqlDateToDateTime(CDateTime& timestamp, const MYSQL_TIME& mysqlDate);

public:
    /// @brief Constructor
    /// @param connection Connection*, MySQL connection
    /// @param sql std::string, SQL statement
    /// @param autoPrepare bool, If true then statement is executed as prepared.
    CMySQLStatement(CMySQLConnection* connection, std::string sql, bool autoPrepare);

    /// @brief Destructor
    virtual ~CMySQLStatement();

    /// @brief Generates normalized list of parameters
    /// @param queryParams CParamList&, Standard query parameters
    void enumerateParams(CParamList& queryParams);

    /// @brief Sets actual parameter values for the statement execution
    void setParameterValues();

    /// @brief Prepares MySQL statement
    /// @param sql const std::string, statement SQL
    void prepare(const std::string& sql);

    /// @brief Executes statement
    void execute(bool);

    /// @brief Binds statement result metadata to query fields
    /// @param fields CFieldList&, query fields (if any)
    void bindResult(CFieldList& fields);

    /// @brief Fetches statement result metadata to query fields
    /// @param fields CFieldList&, query fields (if any)
    void readResultRow(CFieldList& fields);

    /// @brief Closes statement and releases allocated resources
    void close();

    /// @brief Fetches next record
    void fetch();
};

}

#endif
