/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ssl_server_test.cpp - description                      ║
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

#include <sptk5/net/SSLContext.h>
#include <sptk5/net/SSLSocket.h>

using namespace std;
using namespace sptk;

void readAndReply(SSLSocket& socket)
{
    char        buffer[1024];
    char        reply[2048];
    size_t      bytes;
    const char* HTMLecho="<html><body><pre>%s</pre></body></html>\n\n";

    if (!socket.readyToRead(chrono::seconds(3))) {
        cerr << "Read timeout" << endl;
        return;
    }

    bytes = socket.socketBytes();
    if (bytes == 0) {
        cout << "Connection is closed by client" << endl;
        return;
    }

    if (bytes > sizeof(buffer))
        bytes = sizeof(buffer);

    bytes = socket.read(buffer, bytes);
    buffer[bytes] = 0;
    printf("Client msg: \"%s\"\n", buffer);
    snprintf(reply, sizeof(reply), HTMLecho, buffer);
    socket.write(reply, strlen(reply));
}

int main(int argc, const char *argv[])
{
    int port = string2int(argv[1]);

    if (argc != 2 || port == 0) {
        printf("Usage: %s <portnum>\n", argv[0]);
        return 1;
    }

    try {
        TCPSocket server;
        server.host(Host("localhost", 3000));

        server.listen();

        while (true)
        {
            SOCKET clientSocketFD;
            struct sockaddr_in clientInfo = {};

            server.accept(clientSocketFD, clientInfo);
            printf("Connection: %s:%u\n", inet_ntoa(clientInfo.sin_addr), (unsigned) ntohs(clientInfo.sin_port));

            SSLSocket connection;
            connection.loadKeys("key.pem", "cert.pem", "");
            try {
                connection.attach(clientSocketFD);
                readAndReply(connection);         /* service connection */
                connection.close();
            }
            catch (const Exception& e) {
                cerr << e.what() << endl;
            }
        }
    }
    catch (const Exception& e) {
        cerr << e.what() << endl;
    }
}
