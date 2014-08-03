/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          smtp_connect.cpp  -  description
                             -------------------
    begin                : 26 June 2003
    copyright            : (C) 1999-2014 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <stdio.h>
#include <iostream>
#include <sptk5/cnet>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main(int argc, char *argv[])
{
    CFileLog        logger("smtp.log");
    logger.option(CBaseLog::CLO_STDOUT, true);
    
    CSmtpConnect    SMTP(&logger);
    std::string     user, password, email, host, portStr;

    cout << "Testing SMTP connectivity." << endl;
    
    if (argc == 3) {
        CRegExp parser("([\\w_-]+)(:\\S+){0,1}@([\\w_-]+)(:\\d+){0,1}$", "i");
        CStrings matches;
        if (parser.m(argv[1], matches)) {
            user = matches[0];
            password = matches[1]; if (!password.empty()) password.erase(0,1);
            host = matches[2];
            portStr = matches[3]; if (!portStr.empty()) portStr.erase(0,1);
        }
        email = argv[2];
    } else {
        cout << "SMTP server name: ";
        cin >> host;
        cout << "SMTP server port[25]: ";
        cin >> portStr;
        cout << "SMTP user name (or N/A if not required): ";
        cin >> user;
        if (trim(lowerCase(user)) != "n/a") {
            cout << "SMTP user password: ";
            cin >> password;
        } else
            user = "";
        cout << "E-mail address to test: ";
        cin >> email;
    }
   
    int port = atoi(portStr.c_str());
    if (port < 1) port = 25;

    cout << "\nTrying to connect to SMTP server.." << endl;

    try {
        SMTP.host(host);
        SMTP.port(port);
        SMTP.cmd_auth(user, password, "login"); // Supported methods are login and plain
        //SMTP.cmd_login("","");
        cout << SMTP.response().asString("\n") << endl;

        SMTP.subject("Test e-mail");
        SMTP.from("Yourself <"+email+">");
        SMTP.to(email);
        SMTP.body("<HTML><BODY>Hello, <b>my friend!</b><br><br>\n\nIf you received this e-mail it means the SMTP module works just fine.<br><br>\n\nSincerely, SPTK.<br>\n</BODY></HTML>", true);
        //SMTP.attachments("test.html");
        
        cout << "\nSending test message.." << endl;
        SMTP.cmd_send();
        cout << SMTP.response().asString("\n") << endl;
        
        cout << "\nClosing SMTP connection.." << endl;
        SMTP.cmd_quit();
        cout << SMTP.response().asString("\n") << endl;
        
        cout << endl << "Message send. Please, check your mail in " << email << endl;
    }
    catch (std::exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}
