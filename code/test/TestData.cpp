/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "TestData.h"

using namespace std;
using namespace sptk;

namespace sptk {

std::filesystem::path TestData::DataDirectory()
{
#ifdef _WIN32
    const auto* programData = getenv("PROGRAMDATA");
    if (programData != nullptr)
    {
        return std::filesystem::path(programData) / "SPTK" / "test_data";
    }
    return "test_data";
#else
    return "/usr/local/share/sptk5/test_data";
#endif
}

std::filesystem::path TestData::SslKeysDirectory()
{
    auto dataDirectory = DataDirectory();
    return dataDirectory.parent_path() / "test_ssl_keys";
}

} // namespace sptk
