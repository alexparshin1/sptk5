/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       logfile_test.cpp - description                         ║
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

using namespace std;
using namespace sptk;

int main()
{
   try {
      cout << "Creating a log file ./logfile_test.log: " << endl;
      FileLogEngine fileLog("logfile_test.log");
      Logger  log(fileLog);

      /// Cleaning log file before test.
      fileLog.reset();
	  fileLog.option(LogEngine::LO_STDOUT, true);

      /// Set the minimal priority for the messages.
      /// Any messages with the less priority are ignored.
      /// This means, in this example, that no messages with CLP_DEBUG priority
      /// would make it to the log.
      fileLog.minPriority(LP_INFO);
      
      cout << "Sending 'Hello, World!' to this file.." << endl;
      log.info("Hello, World! Welcome to SPTK.");
      log.log(LP_WARNING, "Eating too much nuts will turn you into HappySquirrel!");
      log.log(LP_DEBUG, "This statement is not confirmed by HappySquirrel");
      log.log(LP_INFO, "This is the end of the log.");
   }
   catch (exception& e) {
      cerr << e.what() << endl;
   }

   return 0;
}
