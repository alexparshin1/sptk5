/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include <gtest/gtest.h>

#include <sptk5/db/ODBCConnection.h>

#if defined(HAVE_ODBC)

using namespace std;
using namespace sptk;

namespace {
class ODBCConnectionProbe : public ODBCConnection
{
public:
    using ODBCConnection::ODBCConnection;

    static pair<int32_t, VariantDataType> mapType(const int32_t sqlType)
    {
        int32_t         ctype = 0;
        VariantDataType variantType = VariantDataType::VAR_NONE;
        odbcTypeToCType(sqlType, ctype, variantType);
        return {ctype, variantType};
    }
};
} // namespace

TEST(ODBCConnectionTests,nativeConnectionString)
{
    // Everything from the URL must reach the driver, and values with spaces or semicolons must be
    // brace-quoted so that the resulting string can be parsed back.
    ODBCConnection connection(
        "odbc://user:secret@my+dsn/db?application_name=xmq+client&TrustServerCertificate=yes&extra=a%3Bb");
    const String native = connection.nativeConnectionString();

    EXPECT_NE(native.find("DSN=my+dsn"), String::npos);
    EXPECT_NE(native.find("UID=user"), String::npos);
    EXPECT_NE(native.find("PWD=secret"), String::npos);
    EXPECT_NE(native.find("DATABASE=db"), String::npos);
    EXPECT_NE(native.find("application_name={xmq client}"), String::npos);
    EXPECT_NE(native.find("TrustServerCertificate=yes"), String::npos);
    EXPECT_NE(native.find("extra={a;b}"), String::npos);
}

TEST(ODBCConnectionTests,typeMapping)
{
    // SQL_C_SSHORT must be readable by queryFetch, SQL_BIGINT must not be narrowed to 32 bits,
    // and exact numerics must not be converted to double on the way in.
    const auto [smallType, smallVariant] = ODBCConnectionProbe::mapType(SQL_SMALLINT);
    EXPECT_EQ(smallType, SQL_C_SSHORT);
    EXPECT_EQ(smallVariant, VariantDataType::VAR_INT);

    const auto [bigType, bigVariant] = ODBCConnectionProbe::mapType(SQL_BIGINT);
    EXPECT_EQ(bigType, SQL_C_SBIGINT);
    EXPECT_EQ(bigVariant, VariantDataType::VAR_INT64);

    const auto [numericType, numericVariant] = ODBCConnectionProbe::mapType(SQL_NUMERIC);
    EXPECT_EQ(numericType, SQL_C_CHAR);
    EXPECT_EQ(numericVariant, VariantDataType::VAR_STRING);
}

#endif
