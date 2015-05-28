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
#include <sptk5/net/CSSLSocket.h>

using namespace std;
using namespace sptk;

int main(int, const char**)
{
    try {
        CSSLContext sslContext;
        sslContext.loadKeys("keys/privkey.pem", "keys/cacert.pem", "password", "keys/cacert.pem");

        CSSLSocket client(sslContext);
        CBuffer buffer;

        for (unsigned i = 0; i < 10; i++) {
            client.open("localhost", 443);

            client.write("GET /\n",6);
            client.readLine(buffer, '\n');
            cout << "Receiving: ";
            cout << buffer.data() << "\n";
            client.close();
            CThread::msleep(3000);
        }

    } catch (exception& e) {
        cout << "Exception was caught: ";
        cout << e.what() << "\nExiting.\n";
    }

    cout << "Exiting\n";

    return 0;
}
