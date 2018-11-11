/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tcp_server_test.cpp - description                      ║
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

using namespace sptk;

int main( int argc, char* argv[] )
{
   std::cout << "running....\n";

   try {
      // Create the socket
      TCPSocket server;
      server.host(Host("localhost", 3000));

      SOCKET clientSocketFD;
      struct sockaddr_in clientInfo {};

      server.listen();
      server.accept(clientSocketFD, clientInfo);

      TCPSocket new_sock;
      new_sock.attach(clientSocketFD);

      try {
         std::string data;

         std::cout << "Sending: Test SPTK server 1.00\n";
         new_sock.write("Test SPTK server 1.00\n");

         std::cout << "Receving (strings): ";

         do {
            new_sock.readLine(data);
            std::cout << data.c_str() << "\n";
         } while (data != "EOD");

         std::cout << "Sending: confirmation\n";
         new_sock.write("Data accepted\n");

         // End of session
         try {
            new_sock.readLine(data);
         }
         catch(...) {}

         server.close();
      }
      catch (std::exception&) {}
   }
   catch (std::exception& e) {
      std::cout << "Exception was caught: " << e.what() << "\nExiting.\n";
   }
   std::cout << "Server session closed" << std::endl;
   return 0;
}
