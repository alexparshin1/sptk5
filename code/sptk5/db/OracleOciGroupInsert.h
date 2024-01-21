/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/Query.h>

namespace sptk {

class OracleOciGroupInsert
{
public:
    OracleOciGroupInsert(OracleOciConnection* connection, const String& tableName, const Strings& columnNames, size_t groupSize);

    void insertRows(const std::vector<VariantVector>& rows);

private:
    Query m_insertQuery;
    Strings m_columnNames;
    String m_tableName;
    size_t m_groupSize;
    OracleOciConnection* m_connection {nullptr};

    [[nodiscard]] static String makeInsertSQL(const String& tableName, const Strings& columnNames, size_t groupSize);
    void insertGroupRows(Query& insertQuery, std::vector<VariantVector>::const_iterator startRow, std::vector<VariantVector>::const_iterator end);
};

} // namespace sptk
