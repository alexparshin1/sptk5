/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include "sptk5/db/DatabaseField.h"
namespace sptk {
class OracleOciDatabaseField final : public DatabaseField
{
public:
    OracleOciDatabaseField(const String& fieldName, int fieldType, VariantDataType dataType, size_t fieldLength, int fieldScale, const String& sqlType);

    [[nodiscard]] String sqlType() const
    {
        return m_sqlType;
    }

    using Field::operator=;

private:
    String m_sqlType;
};

} // namespace sptk
