/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       syslog_test.cpp - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
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
    COUT("Attention: This example project must include file events.rc.");
    COUT("You should also have enough access rights to write into HKEY_LOCAL_MACHINE");
    COUT("in Windows registry.\n\n");
#endif
    try
    {
        COUT("Defining a log attributes: ");
        SysLogEngine logger1("syslog_test", LOG_USER);
        const Logger sysLog(logger1);

        SysLogEngine logger2("syslog_test", LOG_AUTH);
        const Logger authLog(logger2);

        COUT("Sending 'Hello, World!' to the log..");
        sysLog.info("Hello, World! Welcome to SPTK.");
        authLog.log(LogPriority::Alert, "This is SPTK test message");
        sysLog.log(LogPriority::Warning, "Eating too much nuts will turn you into HappySquirrel!");
    }
    catch (const Exception& e)
    {
        CERR(e.what());
        return 1;
    }

    return 0;
}
