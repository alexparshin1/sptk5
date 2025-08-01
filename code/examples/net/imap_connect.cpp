/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       imap_connect.cpp - description                         ║
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

#include <cstdio>

#include <sptk5/cnet>
#include <sptk5/cutils>
#include <sptk5/cgui>

using namespace std;
using namespace sptk;

void printResponse(const Strings& response)
{
    COUT(response.join("\n") << endl);
    COUT("---------------------------------" << endl);
}

String getString()
{
    char    buffer[128];

    cin.getline(buffer, sizeof(buffer) - 2);

    return trim(buffer);
}

void saveSettings(const String& user, const String& password, const String& server, Registry& registry,
                  xml::Node* hostNode)
{
    try {
            hostNode = registry.findFirst("host");
            if (!hostNode)
                hostNode = new xml::Element(registry, "host");
            hostNode->setAttribute("hostname",server);
            hostNode->setAttribute("user",user);
            hostNode->setAttribute("password",password);
            registry.save();
        }
        catch (const Exception& e) {
            CERR(e.what() << endl);
        }
}

int main()
{
    try {
        // Initialize themes
        CThemes        themes;
        ImapConnect    IMAP;
        String         user;
        String         password;
        String         server;
        Registry       registry("imap_connect.ini","");

        COUT("Testing IMAP connectivity." << endl << endl);

        registry.load();
        xml::Node* hostNode = registry.findFirst("host");
        if (hostNode) {
            server = (String) hostNode->getAttribute("hostname");
            user = (String) hostNode->getAttribute("user");
            password = (String) hostNode->getAttribute("password");
        }
        IMAP.host(Host(server,143));

        if (!user.length()) {
            COUT("IMAP server name: ");
            server = getString();
            IMAP.host(Host(server,143));

            COUT("IMAP user name: ");
            user = getString();

            COUT("IMAP user password: ");
            password = getString();
        }

        COUT("\nTrying to connect to IMAP server.." << endl);

        IMAP.cmd_login(user,password);
        printResponse(IMAP.response());

        // Connected? Save the logon parameters..
        saveSettings(user, password, server, registry, hostNode);

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

        COUT(endl << "Closing IMAP connection.." << endl);
        IMAP.cmd_logout();
        printResponse(IMAP.response());
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
        return 1;
    }

    return 0;
}
