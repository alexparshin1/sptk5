/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       udp_server_test.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/cnet>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main()
{
    try
    {
        // Create the socket
        UDPSocket server;
        server.host(Host("localhost", 3000));

        sockaddr_in clientInfo = {};

        server.listen();

        COUT("Listening: Test SPTK UDP server 1.00\n");

        Buffer readBuffer(1024);

        for (;;)
        {
            if (server.readyToRead(chrono::seconds(1)))
            {
                const size_t bytes = server.read(readBuffer, 1024, &clientInfo);

                String data(readBuffer.c_str(), bytes);
                COUT("Received data: " << data << '\n');

                if (data.startsWith("EOD"))
                {
                    server.close();
                    COUT("Server session closed\n");
                    break;
                }
            }
        }
    }
    catch (const Exception& e)
    {
        CERR("Exception was caught: " << e.what() << "\nExiting.");
        return 1;
    }

    return 0;
}
