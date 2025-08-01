/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       smtp_connect.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <sptk5/cnet>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main(int argc, char* argv[])
{
    try {
        FileLogEngine logEngine("smtp.log");
        logEngine.option(LogEngine::LO_STDOUT, true);

        Logger logger(logEngine);

        SmtpConnect SMTP(&logger);
        String user;
        String password;
        String email;
        String host;
        String portStr;

        COUT("Testing SMTP connectivity." << endl);

        if (argc == 3) {
            RegularExpression parser("^((\\S+):(\\S+)@){0,1}([\\w_\\-\\.]+)(:\\d+){0,1}$", "i");
            Strings matches;
            if (parser.m(argv[1], matches)) {
                user = matches[1];
                password = matches[2];
                host = matches[3];
                portStr = matches[4];
                if (!portStr.empty())
                    portStr.erase(0, 1);
                else
                    portStr = "25";
            }
            email = argv[2];
        } else {
            COUT("Please provide server hostname/port, user credentials, ad destination email address." << endl);
            COUT("You can also use command line arguments:" << endl);
            COUT("  ./smtp_connect [username:password@]<hostname>[:port] <email address>" << endl << endl);

            COUT("SMTP server name: ");
            cin >> host;
            COUT("SMTP server port[25]: ");
            cin >> portStr;
            COUT("SMTP user name (or N/A if not required): ");
            cin >> user;
            if (trim(lowerCase(user)) != "n/a") {
                COUT("SMTP user password: ");
                cin >> password;
            } else
                user = "";
            COUT("E-mail address to test: ");
            cin >> email;
        }

        auto port = (uint16_t) string2int(portStr);
        if (port < 1) port = 25;

        COUT("\nTrying to connect to SMTP server.." << endl);

        SMTP.host(Host(host, port));
        if (!user.empty() && !password.empty())
            SMTP.cmd_auth(user, password);  // Supported methods are login and plain

        COUT(SMTP.response().join("\n") << endl);

        SMTP.subject("Test e-mail");
        SMTP.from("Me <" + email + ">");
        SMTP.to(email);
        SMTP.body(
                "<HTML><BODY>Hello, <b>my friend!</b><br><br>\n\nIf you received this e-mail it means the SMTP module works just fine.<br><br>\n\nSincerely, SPTK.<br>\n</BODY></HTML>",
                true);

        COUT("\nSending test message.." << endl);
        SMTP.cmd_send();
        COUT(SMTP.response().join("\n") << endl);

        COUT("\nClosing SMTP connection.." << endl);
        SMTP.cmd_quit();
        COUT(SMTP.response().join("\n") << endl);

        COUT(endl << "Message sent. Please, check your mail in " << email << endl);

        return 0;
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
        return 1;
    }
}
