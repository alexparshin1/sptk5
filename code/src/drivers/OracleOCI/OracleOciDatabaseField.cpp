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

OracleOciDatabaseField::OracleOciDatabaseField(const String& fieldName, const int fieldType, const VariantDataType dataType,
                                               const size_t fieldLength, const int fieldScale, const String& sqlType)
    : DatabaseField(fieldName, fieldType, dataType, fieldLength, fieldScale)
    , m_sqlType(sqlType.toLowerCase())
{
}
