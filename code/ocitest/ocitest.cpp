#include "ocilib.hpp"
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
        OracleOciConnection oracleOciConnection(connectionString, chrono::seconds(10));

        oracleOciConnection._openDatabase(connectionString);

        Connection con("theater:1521/xe", "gtest", "test#123");

        Statement st(con);
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
