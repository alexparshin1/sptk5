/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ssl_client_test.cpp - description                      ║
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

#include <iostream>
#include <sptk5/cnet>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main(int, const char**)
{
    try
    {
        SSLSocket client;
        SocketReader clientReader(client);

        SSLKeys keys("keys/privkey.pem", "keys/cacert.pem", "password", "keys/cacert.pem");
        client.loadKeys(keys);
        Buffer buffer;

        for (unsigned i = 0; i < 10; i++)
        {
            client.open(Host("localhost", 443));

            client.write((const uint8_t*) "GET /\n", 6);
            clientReader.readLine(buffer, '\n');
            COUT("Receiving: ");
            COUT(buffer.data() << endl);
            client.close();
            this_thread::sleep_for(chrono::milliseconds(3000));
        }
    }
    catch (const Exception& e)
    {
        CERR("Exception was caught: " << e.what() << endl
                                      << "Exiting." << endl);
        return 1;
    }

    CERR("Exiting" << endl);
    return 0;
}
