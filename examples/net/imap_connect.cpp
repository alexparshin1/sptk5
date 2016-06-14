/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       imap_connect.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <stdio.h>

#include <sptk5/cnet>
#include <sptk5/cutils>
#include <sptk5/cgui>

using namespace std;
using namespace sptk;

void printResponse(const Strings& response) {
    for (unsigned i = 0; i < response.size(); i++) {
        puts(response[i].c_str());
    }
    puts("---------------------------------");
}

int main( int argc, char *argv[] )
{
    // Initialize themes
    CThemes themes;

    char           buffer[128];
    ImapConnect    IMAP;
    std::string    user, password, server;
    Registry       registry("imap_connect.ini","");

    puts("Testing IMAP connectivity.\n");

    try {
        registry.load();
        CXmlNode* hostNode = registry.findFirst("host");
        if (hostNode) {
            server = (string) hostNode->getAttribute("hostname");
            user = (string) hostNode->getAttribute("user");
            password = (string) hostNode->getAttribute("password");
        }
        IMAP.host(server);
    }
    catch (...) {}

	if (!user.length()) {
		printf("IMAP server name: ");
		scanf("%s",buffer);
		server = buffer;
		IMAP.host(server);

		printf("IMAP user name: ");
		scanf("%s",buffer);
		user = buffer;

		printf("IMAP user password: ");
		scanf("%s",buffer);
		password = buffer;
	}

	puts("\nTrying to connect to IMAP server..");

	try {
		IMAP.cmd_login(user,password);
		printResponse(IMAP.response());

      // Connected? Save the logon parameters..
		try {
			CXmlNode* hostNode = registry.findFirst("host");
			if (!hostNode)
				hostNode = new CXmlElement(registry,"host");
			hostNode->setAttribute("hostname",server);
			hostNode->setAttribute("user",user);
			hostNode->setAttribute("password",password);
			registry.save();
		}
		catch (...) {
		}

      // RFC 2060 test message :)
		Buffer msgBuffer(
				"Date: Mon, 7 Feb 1994 21:52:25 -0800 (PST)\n\r"
				"From: Fred Foobar <foobar@Blurdybloop.COM\n\r"
				"Subject: afternoon meeting\n\r"
				"To: mooch@owatagu.siam.edu\n\r"
				"Message-Id: <B27397-0100000@Blurdybloop.COM>\n\r"
				"MIME-Version: 1.0\n\r"
				"Content-Type: TEXT/PLAIN; CHARSET=US-ASCII\n\r\n\r"
				"Hello Joe, do you think we can meet at 3:30 tomorrow?\n\r");

		IMAP.cmd_append("Sent",msgBuffer);
		printResponse(IMAP.response());

		puts("\nClosing IMAP connection..");
		IMAP.cmd_logout();
		printResponse(IMAP.response());
	}
	catch (exception& e) {
		puts(e.what());
	}
	return 0;
}
