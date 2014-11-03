/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          tcp_client_test.cpp  -  description
                             -------------------
    begin                : Wed Apr 20, 2005
    copyright            : (C) 2000-2011 by Alexey Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <iostream>
#include <sptk5/cutils>
#include <sptk5/cnet>
#include <sptk5/net/COpenSSLSocket.h>

using namespace std;
using namespace sptk;

int main(int, const char**)
{
    try {
        COpenSSLContext sslContext;
        sslContext.loadKeys("keys/private/client-key.pem", "keys/certs/client.pem", "password", "keys/cacert.pem");

        COpenSSLSocket client(sslContext);

        client.open("localhost", 3000);
        //client.open("atl0901.zonarsystems.net", 443);

        client.blockingMode(false);
        cout << "Connected\n";

        CBuffer buffer(1024);

        client.write("GET /\n",6);
        client.readLine(buffer, '\n');
        cout << "Receiving: ";
        cout << buffer.data() << "\n";

        string data = "Several copies of a single string";
        cout << "Sending: test data\n";
        client.write(data + "\n" + data + " " + data + "\n" + data + " " + data + " " + data + " " + data + "\n" + data + " " + data + "\n");

        cout << "Sending: end data\n";
        client.write("EOD\n");

        client.readLine(buffer, '\n');
        cout << "Receiving: ";
        cout << buffer.data() << "\n";

        cout << "Sending: end session\n";
        client.write("EOS\n");
    } catch (exception& e) {
        cout << "Exception was caught: ";
        cout << e.what() << "\nExiting.\n";
    }

    cout << "Exiting\n";

    return 0;
}
