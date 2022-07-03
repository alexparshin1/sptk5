/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tcp_server_test.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/wsdl/WSListener.h>

using namespace std;
using namespace sptk;

class StubRequest
    : public WSRequest
{
protected:
    void requestBroker(const String& requestName, const xdoc::SNode&, const xdoc::SNode& jsonNode, HttpAuthentication*,
                       const WSNameSpace&) override
    {
        // Not used in this test
    }

public:
    StubRequest() = default;
};

int main()
{
    try
    {
        auto request = make_shared<StubRequest>();
        SysLogEngine log("ws_server_test");
        Logger logger(log);

        char hostname[128];
        int rc = gethostname(hostname, sizeof(hostname));
        if (rc != 0)
        {
            throw SystemException("Can't get hostname");
        }
        WSConnection::Paths paths("index.html", "request", "/var/lib/pgman/webapp");
        WSConnection::Options options(paths);
        WSServices services(request);
        WSListener server(services, log, hostname, 32, options);
        server.listen(8000);
        while (true)
        {
            this_thread::sleep_for(chrono::milliseconds(1000));
        }
    }
    catch (const Exception& e)
    {
        CERR("Exception was caught: " << e.what() << endl
                                      << "Exiting." << endl)
        return 1;
    }
    COUT("Server session closed" << endl)
    return 0;
}
