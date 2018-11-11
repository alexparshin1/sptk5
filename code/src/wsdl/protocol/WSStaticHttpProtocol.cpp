/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSStaticHttpProtocol.cpp - description                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
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

#include "WSStaticHttpProtocol.h"

using namespace std;
using namespace sptk;

WSStaticHttpProtocol::WSStaticHttpProtocol(TCPSocket* socket, const String& url, const HttpHeaders& headers, const String& staticFilesDirectory)
: WSProtocol(socket, headers), m_url(url), m_staticFilesDirectory(staticFilesDirectory)
{
}

void WSStaticHttpProtocol::process()
{
    Buffer page;
    try {
        page.loadFromFile(m_staticFilesDirectory + m_url);
        m_socket.write("HTTP/1.1 200 OK\n");
        m_socket.write("Content-Type: text/html; charset=utf-8\n");
        m_socket.write("Content-Length: " + int2string(page.bytes()) + "\n\n");
        m_socket.write(page);
    }
    catch (...) {
        string text("<html><head><title>Not Found</title></head><body>Sorry, the page " + m_staticFilesDirectory + m_url + " was not found.</body></html>\n");
        m_socket.write("HTTP/1.1 404 Not Found\n");
        m_socket.write("Content-Type: text/html; charset=utf-8\n");
        m_socket.write("Content-length: " + int2string(text.length()) + "\n\n");
        m_socket.write(text);
    }
}

