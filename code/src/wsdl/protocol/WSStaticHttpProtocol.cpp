/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/wsdl/protocol/WSStaticHttpProtocol.h"

using namespace std;
using namespace sptk;

WSStaticHttpProtocol::WSStaticHttpProtocol(TCPSocket* socket, const URL& url, const HttpHeaders& headers, const String& staticFilesDirectory)
: WSProtocol(socket, headers), m_url(url), m_staticFilesDirectory(staticFilesDirectory)
{
}

RequestInfo WSStaticHttpProtocol::process()
{
    static RegularExpression matchImageFiles(R"(\.(png|gif|jpg|jpeg|pcx)$)", "i");

    RequestInfo requestInfo("HTTP GET");

    String fullPath(m_staticFilesDirectory + m_url.path());

    requestInfo.request.input(Buffer(fullPath), "");

    Strings contentEncodings;
    if (!matchImageFiles.matches(m_url.path()))
        contentEncodings.push_back("gzip");
    try {
        requestInfo.response.content().loadFromFile(fullPath);
        Buffer output = requestInfo.response.output(contentEncodings);
        socket().write("HTTP/1.1 200 OK\n");
        socket().write("Content-Type: text/html; charset=utf-8\n");
        if (!requestInfo.response.contentEncoding().empty())
            socket().write("Content-Encoding: " + requestInfo.response.contentEncoding() + "\n");
        socket().write("Content-Length: " + int2string(output.length()) + "\n\n");
        socket().write(output);
    }
    catch (const Exception&) {
        String text("<html><head><title>Not Found</title></head><body>Resource " + m_staticFilesDirectory + m_url.path() + " was not found.</body></html>\n");
        Buffer output = requestInfo.response.output(contentEncodings);
        requestInfo.response.content() = text;
        socket().write("HTTP/1.1 404 Not Found\n");
        socket().write("Content-Type: text/html; charset=utf-8\n");
        if (!requestInfo.response.contentEncoding().empty())
            socket().write("Content-Encoding: " + requestInfo.response.contentEncoding() + "\n");
        socket().write("Content-length: " + int2string(text.length()) + "\n\n");
        socket().write(text);
    }
    return requestInfo;
}

