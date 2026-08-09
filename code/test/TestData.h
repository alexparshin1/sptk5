/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include "sptk5/cutils"

// Brings in the platform socket headers, for SOCK_STREAM/SOCK_DGRAM in freePort().
#include <sptk5/net/SocketVirtualMethods.h>

namespace sptk {
class TestData
{
public:
    static std::filesystem::path DataDirectory();
    static std::filesystem::path SslKeysDirectory();

    /**
     * @brief Returns a TCP port that the kernel has just confirmed to be free.
     *
     * Tests that hard-code a listener port are unreliable: the ephemeral range
     * (/proc/sys/net/ipv4/ip_local_port_range) commonly spans the whole high port space, so a
     * fixed number may already be held as the source port of another test's outgoing
     * connection - and a port used by a previous run of this binary may still be in TIME_WAIT.
     * Either way bind() fails with EADDRINUSE and the test fails for no reason of its own.
     *
     * Binding a probe socket to port 0 makes the kernel name a port that is free at that
     * moment, which is as good a guarantee as a test can get.
     * @param socketType        SOCK_STREAM or SOCK_DGRAM - TCP and UDP have separate port
     *                          spaces, so the probe has to be of the same kind as the socket
     *                          the port is meant for.
     * @return Free port number.
     */
    static uint16_t freePort(int socketType = SOCK_STREAM);
};

} // namespace sptk
