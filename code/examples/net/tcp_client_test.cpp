/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tcp_client_test.cpp - description                      ║
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

#include <sptk5/cnet>
#include <sptk5/Printer.h>

using namespace sptk;

int main() {
   try {
      TCPSocket client;
      client.host(Host("localhost",3000));

       client.open();

      COUT("Connected\n");

      String data;

      client.readLine(data);
      COUT("Receiving: " << data.c_str() << "\n");

      data = "Several copies of a single string";
      COUT("Sending: test data\n");
      client.write(data + "\n" + data + " " + data + "\n" + data + " " + data + " " + data + " " + data + "\n" + data + " " + data + "\n");

      COUT("Sending: end data\n");
      client.write("EOD\n");

      client.readLine(data);
      COUT("Receiving: " << data.c_str() << "\n");

      COUT("Sending: end session\n");
      client.write("EOS\n");

      client.readLine(data);
      COUT("Receiving: " << data.c_str() << "\n");
   } catch (const Exception& e) {
      CERR("Exception was caught:" << e.what() << "\nExiting.\n");
   }

   COUT("Exiting\n");

   return 0;
}
