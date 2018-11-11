/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       syslog_test.cpp - description                          ║
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

#include <sptk5/cutils>
#include <sptk5/cthreads>

using namespace std;
using namespace sptk;

int main(int argc, char* argv[])
{
#ifdef _WIN32
   cout << "Attention: This example project must include file events.rc." << endl;
   cout << "You should also have enough access rights to write into HKEY_LOCAL_MACHINE" << endl;
   cout << "in Windows registry." << endl << endl;
#endif
   try {
      cout << "Defining a log attributes: " << endl;
      SysLogEngine   logger1("syslog_test", LOG_USER);
      Logger sysLog(logger1);

      SysLogEngine   logger2("syslog_test", LOG_AUTH);
      Logger authLog(logger2);

      cout << "Sending 'Hello, World!' to the log.." << endl;
      sysLog.info("Hello, World! Welcome to SPTK.");
      authLog.log(LP_ALERT, "This is SPTK test message");
      sysLog.log(LP_WARNING, "Eating too much nuts will turn you into HappySquirrel!");
   }
   catch (exception& e) {
      cerr << e.what() << endl;
      this_thread::sleep_for(chrono::seconds(5));
   }

   return 0;
}
