/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       http_connect.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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
#include <FL/fl_ask.H>
#include <sptk5/cnet>
#include <sptk5/cgui>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/net/SSLSocket.h>

using namespace std;
using namespace sptk;

int main(int argc,char *argv[])
{
    system("rm -rf /tmp/logs");
    system("mkdir /tmp/logs");

    for (int i = 0; i < 10; i++)
    try {
        DateTime        started = DateTime::Now();

        TCPSocket* socket = new TCPSocket;

        HttpConnect sock(*socket);

        Host api("api.karrostech.io", 80);
        socket->open(api);

        HttpParams httpFields;
        httpFields["tenant"] = "7561721b-abf2-4c35-a47d-21b4d1157bdb";
        httpFields["startdate"]="2017-08-23T21:00:00.000Z";
        httpFields["size"] = "10000";
        httpFields["page"] = "0";
        httpFields["enddate"] = "2017-08-23T21:59:59.999Z";

        sock.requestHeaders()["Authorization"] = "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJodHRwOi8va2Fycm9zdGVjaC5jb20iLCJzdWIiOjEsImlhdCI6MTUwNzkxMTAxMDc4NywiZXhwIjoxNTE0NDE5MTk5MDAwLCJlbWFpbCI6ImFkbWluQGVkdWxvZy5jb20iLCJmaXJzdE5hbWUiOiJBZG1pbiIsIm1pZGRsZU5hbWUiOm51bGwsImxhc3ROYW1lIjpudWxsLCJhdXRob3JpemF0aW9uIjp7Imdyb3VwcyI6W3siZ3JvdXBOYW1lIjoiQWNjb3VudCBNYW5hZ2VtZW50Iiwicm9sZXMiOlsiQWRtaW4iXX0seyJncm91cE5hbWUiOiJQYXJlbnQgUG9ydGFsIiwicm9sZXMiOlsiQWRtaW4iXX0seyJncm91cE5hbWUiOiJFZHVsb2ciLCJyb2xlcyI6WyJTeXN0ZW0gQWRtaW4iXX1dfX0.9n-ZuM_mVgxOiKYR-qxiuFYyJLw0fmU4DwOdsHE68zc";
        try {
            sock.cmd_get("/event/api/0.1/events", httpFields, chrono::seconds(30));
        }
        catch (const exception& e) {
            cerr << e.what() << endl;
            cerr << sock.htmlData().c_str() << endl;
        }

        cout << "Received " << sock.htmlData().bytes() << endl << endl;

        DateTime    finished = DateTime::Now();
        int durationMS = chrono::duration_cast<chrono::milliseconds>(finished - started).count();

        cout << "Elapsed " << durationMS << " ms " << endl;

        delete socket;

    } catch (Exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
