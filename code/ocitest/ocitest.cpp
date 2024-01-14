#include "ocilib.hpp"
#include <iostream>

using namespace ocilib;

int main()
{
    try
    {
        Environment::Initialize();

        Connection con("theater:1521/xe", "gtest", "test#123");

        Statement st(con);
        st.Execute("select 1 from dual");

        Resultset rs = st.GetResultset();
        while (rs.Next())
        {
            std::cout << rs.Get<int>(1) << std::endl;
        }
    }
    catch(std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    Environment::Cleanup();

    return EXIT_SUCCESS;
}
