/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday Sep 17 2015                                   ║
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

#include "LoadBalance.h"
#include <signal.h>

using namespace std;
using namespace sptk;

int main(int argc, const char* argv[])
{
    // Mask unwanted signals
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    try {
        Loop<Host> destinations;
        destinations.add(Host("localhost", 1883));

        Loop<String> interfaces;
        interfaces.add(String("127.0.0.1"));

        LoadBalance loadBalance(1100, destinations, interfaces);

        loadBalance.run();
        while (true)
            this_thread::sleep_for(chrono::milliseconds(100));

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
