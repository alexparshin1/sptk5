/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          syslog_test.cpp  -  description
                             -------------------
    begin                : Fri Feb 3, 2006
    copyright            : (C) 1999-2016 by Alexey Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <iostream>
#include <sptk5/CSysLog.h>

using namespace std;
using namespace sptk;

int main(int,char*[])
{
#ifdef _WIN32
   cout << "Attention: This example project must include file VC6\\events.rc." << endl;
   cout << "You should also have enough access rights to write into HKEY_LOCAL_MACHINE" << endl;
   cout << "in Windows registry." << endl << endl;
#endif
   try {
      cout << "Defining a log attributes: " << endl;
      CSysLog::programName("syslog_test");
      CSysLog sysLog(LOG_USER);
      CSysLog authLog(LOG_AUTH);

      cout << "Sending 'Hello, World!' to the log.." << endl;
      sysLog  << "Hello, World!" << endl;
      sysLog  << "Welcome to SPTK." << endl;
      authLog << CLP_ALERT << "This is SPTK test message" << endl;
      sysLog  << CLP_WARNING << "Eating too much nuts will turn you into HappySquirrel!" << endl;
   }
   catch (exception& e) {
      puts(e.what());
   }

   return 0;
}
