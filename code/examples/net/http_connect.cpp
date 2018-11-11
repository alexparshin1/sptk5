/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       http_connect.cpp - description                         ║
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

#include <FL/Fl.H>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;

int main(int argc, char* argv[])
{
    system("rm -rf /tmp/logs");
    system("mkdir /tmp/logs");

    DateTime totalStarted = DateTime::Now();

    for (int i = 0; i < 10; i++) {
        try {
            DateTime started = DateTime::Now();

            auto socket = new TCPSocket;
            HttpConnect sock(*socket);

            Host api("api.karrostech.io", 80);
            socket->open(api);

            HttpParams httpFields;
            httpFields["tenant"] = "7561721b-abf2-4c35-a47d-21b4d1157bdb";
            httpFields["startdate"] = "2017-08-23T21:00:00.000Z";
            httpFields["size"] = "10000";
            httpFields["page"] = "0";
            httpFields["enddate"] = "2017-08-23T21:59:59.999Z";

            Buffer data;
            try {
                sock.cmd_get("/event/api/0.1/events", httpFields, data, chrono::seconds(30));
            }
            catch (const exception& e) {
                cerr << e.what() << endl;
                cerr << data.c_str() << endl;
            }

            cout << "Received " << data.bytes() << endl;

            DateTime finished = DateTime::Now();
            long durationMS = chrono::duration_cast<chrono::milliseconds>(finished - started).count();

            cout << "Elapsed " << durationMS << " ms " << endl << endl;

            delete socket;

        } catch (Exception& e) {
            cerr << e.what() << endl;
            return 1;
        }
    }

    long totalMS = chrono::duration_cast<chrono::milliseconds>(DateTime::Now() - totalStarted).count();
    cout << "Total Elapsed " << totalMS << " ms " << endl << endl;

    return 0;
}
