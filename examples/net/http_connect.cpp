/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       http_connect.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <sptk5/cnet>
#include <sptk5/cgui>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/net/SSLSocket.h>

using namespace std;
using namespace sptk;

CInput         *urlInput;
CInput         *statsInput;
CMemoInput     *paramsInput;
CComboBox      *paramsCombo;
CEditor        *textdisp;

void go_callback(Fl_Widget *,void *)
{
    textdisp->textBuffer()->text("");
    textdisp->redraw();

    std::string  input = urlInput->data();

    RegularExpression     getpage("^(http://|https://){0,1}([^/]+)(/.*)*", "i");
    Strings    matches;

    getpage.m(input, matches);

    int         port = 80;
    string      protocol = matches[0];
    bool        https = false;
    if (lowerCase(protocol) == "https://") {
        https = true;
        port = 443;
    }

    string      hostName = matches[1];
    std::string pageName = matches[2];

    size_t portPos = hostName.find(":");
    if (portPos != string::npos) {
        portPos++;
        port = string2int(hostName.c_str() + portPos);
    }

    TCPSocket*     socket;
    SSLContext sslContext;
    try {
        DateTime        started = DateTime::Now();

        if (!https)
            socket = new TCPSocket;
        else
            socket = new SSLSocket(sslContext);

        HttpConnect sock(*socket);

        socket->open(Host(hostName, port));

        Strings text(paramsInput->data(),"\n");
        HttpParams httpFields;

        if (paramsCombo->data() == "HTTP Get") {

            for (unsigned i = 0; i < text.size(); i++) {
                Strings data(text[i],"=");
                if (data.size() == 2) {
                    httpFields["first_name"] = text[0];
                    httpFields["last_name"] = text[1];
                }
            }

            sock.cmd_get(pageName, httpFields, 30000);
        } else {
            for (unsigned i = 0; i < text.size(); i++) {
                Strings data(text[i],"=");

                if (data.size() == 2) {
                    httpFields["first_name"] = text[0];
                    httpFields["last_name"] = text[1];
                }
            }
        }

        DateTime    finished = DateTime::Now();
        int durationMS = chrono::duration_cast<chrono::milliseconds>(finished - started).count();
        textdisp->textBuffer()->text(sock.htmlData().data());
        statsInput->data("Received " + int2string(sock.htmlData().bytes()) + " bytes for " + int2string(durationMS) + "ms");
    } catch (Exception &e) {
        delete socket;
        spError(e.what());
        return;
    }
    delete socket;
}

int main(int argc,char *argv[])
{
    // Initialize themes
    CThemes themes;

    CWindow main_window(600,400,"CHttpConnect test");

    CToolBar urlToolBar;
    CButton go_button(SP_EXEC_BUTTON,SP_ALIGN_RIGHT,"Go");
    go_button.callback(go_callback);
    urlInput = new CInput("http/https", 300, SP_ALIGN_CLIENT);
    urlToolBar.end();

    CToolBar paramsToolBar;
    paramsToolBar.layoutSize(150);
    paramsCombo = new CComboBox("Mode",10,SP_ALIGN_TOP);
    //paramsCombo->labelWidth(80);
    paramsCombo->addRows("http mode",Strings("HTTP Get|HTTP Post","|"));
    paramsCombo->columns()[0].width(100);
    paramsCombo->data("HTTP Post");
    statsInput = new CInput("Stats");
    paramsInput = new CMemoInput("",80,SP_ALIGN_CLIENT);
    paramsToolBar.end();

    CEditor  editor(10,SP_ALIGN_CLIENT);

    textdisp = &editor;

    CGroup status_line("",25,SP_ALIGN_BOTTOM);
    status_line.layoutSpacing(1);

    status_line.end();

    main_window.end();
    main_window.resizable(main_window);
    main_window.show(argc,argv);

    return Fl::run();
}
