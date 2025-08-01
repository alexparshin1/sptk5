/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       udp_client_test.cpp - description                      ║
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

int main()
{
    try {
        UDPSocket client;
        client.host(Host("localhost", 3000));

        string data;

        struct sockaddr_in serv {};
        memset (&serv, 0, sizeof (serv));
        serv.sin_family = AF_INET;
        serv.sin_port = htons(client.host().port());
        serv.sin_addr.s_addr = inet_addr(client.host().hostname().c_str());

        data = "Data 1";
        client.write(data, &serv);

        data = "EOD";
        client.write(data, &serv);
    } catch (const Exception& e) {
        CERR("Exception was caught:" << e.what() << endl << "Exiting." << endl);
        return 1;
    }

    COUT("Exiting" << endl);
    return 0;
}
