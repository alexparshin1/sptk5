/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/
//
// Created by alexeyp on 20/01/24.
//


#include "sptk5/db/OracleOciDatabaseField.h"

using namespace std;
using namespace sptk;

OracleOciDatabaseField::OracleOciDatabaseField(const string_view& fieldName, int fieldType, VariantDataType dataType,
                                               int fieldLength, int fieldScale, const String& sqlType)
    : DatabaseField(fieldName, fieldType, dataType, fieldLength, fieldScale)
    , m_sqlType(sqlType.toLowerCase())
{
}
