/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tcp_server_test.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/cutils>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;

void processConnection(TCPSocket& server, SOCKET clientSocketFD)
{
    TCPSocket new_sock;
    new_sock.attach(clientSocketFD);

    try {
        String data;

        COUT("Sending: Test SPTK server 1.00\n");
        new_sock.write("Test SPTK server 1.00\n");

        COUT("Receving (strings): ");

        do {
            new_sock.readLine(data);
            COUT(data.c_str() << "\n");
        } while (data != "EOD");

        COUT("Sending: confirmation\n");
        new_sock.write("Data accepted\n");

        // End of session
        new_sock.readLine(data);

        server.close();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
    }
}

int main()
{
    try {
        // Create the socket
        TCPSocket server;
        server.host(Host("localhost", 3000));

        SOCKET clientSocketFD;
        struct sockaddr_in clientInfo{};

        server.listen();

        COUT("Listening on port 3000\n");

        server.accept(clientSocketFD, clientInfo);

        processConnection(server, clientSocketFD);
    }
    catch (const Exception& e) {
        COUT("Exception was caught: " << e.what() << "\nExiting.\n");
    }
    COUT("Server session closed\n");
    return 0;
}
