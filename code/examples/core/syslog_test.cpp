/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       syslog_test.cpp - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cthreads>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main()
{
#ifdef _WIN32
    COUT("Attention: This example project must include file events.rc." << endl);
    COUT("You should also have enough access rights to write into HKEY_LOCAL_MACHINE" << endl);
    COUT("in Windows registry." << endl
                                << endl);
#endif
    try
    {
        COUT("Defining a log attributes: " << endl);
        SysLogEngine logger1("syslog_test", LOG_USER);
        Logger sysLog(logger1);

        SysLogEngine logger2("syslog_test", LOG_AUTH);
        Logger authLog(logger2);

        COUT("Sending 'Hello, World!' to the log.." << endl);
        sysLog.info("Hello, World! Welcome to SPTK.");
        authLog.log(LogPriority::Alert, "This is SPTK test message");
        sysLog.log(LogPriority::Warning, "Eating too much nuts will turn you into HappySquirrel!");
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
        return 1;
    }

    return 0;
}
