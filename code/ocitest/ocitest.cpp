#include "ocilib.hpp"
#include "sptk5/db/OracleOciStatement.h"
#include <iostream>
#include <sptk5/db/OracleOciConnection.h>

using namespace std;
using namespace sptk;
using namespace ocilib;

int main()
{
    try
    {
        //Environment::Initialize();

        const String connectionString("oracleoci://gtest:test#123@theater:1521/xe");
        OracleOciConnection connection(connectionString, chrono::seconds(10));

        connection._openDatabase(connectionString);

        OracleOciStatement stmt(&connection, "select 1 from dual");
        stmt.execute(false);

        auto rset = stmt.resultSet();
        while (rset.Next())
        {
            std::cout << "# " << rset.Get<int>(1) << std::endl;
        }

        Statement st(*connection.connection());
        st.Execute("select 1 from dual");

        Resultset rs = st.GetResultset();
        while (rs.Next())
        {
            std::cout << rs.Get<int>(1) << std::endl;
        }
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    Environment::Cleanup();

    return EXIT_SUCCESS;
}
