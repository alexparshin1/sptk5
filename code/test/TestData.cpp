/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "TestData.h"

#include <sptk5/net/SocketVirtualMethods.h>

using namespace std;
using namespace sptk;

namespace sptk {
uint16_t TestData::freePort(const int socketType)
{
    const auto probe = ::socket(AF_INET, socketType, 0);
    if (probe == INVALID_SOCKET)
    {
        throw Exception("Can't create a probe socket to pick a free port");
    }

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;

    uint16_t port = 0;
    if (::bind(probe, bit_cast<sockaddr*>(&address), sizeof(address)) == 0)
    {
        socklen_t addressLength = sizeof(address);
        if (::getsockname(probe, bit_cast<sockaddr*>(&address), &addressLength) == 0)
        {
            port = ntohs(address.sin_port);
        }
    }

#ifndef _WIN32
    ::close(probe);
#else
    ::closesocket(probe);
#endif

    if (port == 0)
    {
        throw Exception("Can't obtain a free test port");
    }
    return port;
}

std::filesystem::path TestData::DataDirectory()
{
#ifdef _WIN32
    if (const auto* programData = getenv("PROGRAMDATA"))
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
    const auto dataDirectory = DataDirectory();
    return dataDirectory.parent_path() / "test_ssl_keys";
}

} // namespace sptk
