/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       udp_server_test.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

using namespace std;
using namespace sptk;

int main (int argc, char* argv[])
{
    try {
        // Create the socket
        UDPSocket server;
        server.host(Host("localhost",3000));

        struct sockaddr_in clientInfo = {};

        server.listen();

        cout << "Listening: Test SPTK UDP server 1.00\n";

        char readBuffer[1024];

        for (;;) {
            if (server.readyToRead(chrono::seconds(1))) {
                size_t bytes = server.read(readBuffer, sizeof(readBuffer),&clientInfo);

                string data(readBuffer, bytes);
                cout << "Received data: " << data << endl;
                
                if (data.find("EOD") == 0) {
                    server.close();
                    cout << "Server session closed" << endl;
                    break;
                }
            }
        }
    } catch (exception& e) {
        cout << "Exception was caught: " << e.what() << "\nExiting.\n";
    }

    return 0;
}
