/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "sptk5/db/OracleOciDatabaseField.h"

using namespace std;
using namespace sptk;

OracleOciDatabaseField::OracleOciDatabaseField(const string_view& fieldName, int fieldType, VariantDataType dataType,
                                               int fieldLength, int fieldScale, const String& sqlType)
    : DatabaseField(fieldName, fieldType, dataType, fieldLength, fieldScale)
    , m_sqlType(sqlType.toLowerCase())
{
}
