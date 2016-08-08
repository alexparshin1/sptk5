/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tcp_server_test.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/wsdl/WSListener.h>

using namespace std;
using namespace sptk;

class StubRequest : public WSRequest
{
protected:
    virtual void requestBroker(XMLElement* requestNode) THROWS_EXCEPTIONS
    {}

public:
    StubRequest() {}
};

int main( int argc, char* argv[] )
{
    StubRequest     request;
    SysLogEngine    log;
    Logger          logger(log);
    try {
        // Create the socket
        WSListener server(request, logger, "/var/lib/pgman/webapp");
        server.listen(8000);
        while (true)
            Thread::msleep(1000);
    }
    catch (std::exception& e) {
        std::cout << "Exception was caught: " << e.what() << "\nExiting.\n";
    }
    std::cout << "Server session closed" << std::endl;
    return 0;
}
