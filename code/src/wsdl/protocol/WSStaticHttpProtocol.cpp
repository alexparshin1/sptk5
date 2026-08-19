/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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

#include <utility>

#include "sptk5/wsdl/protocol/WSStaticHttpProtocol.h"

using namespace std;
using namespace sptk;

WSStaticHttpProtocol::WSStaticHttpProtocol(const shared_ptr<TCPSocket>& socket, const URL& url, const HttpHeaders& headers,
                                           String staticFilesDirectory)
    : WSProtocol(socket, headers)
    , m_url(url)
    , m_staticFilesDirectory(std::move(staticFilesDirectory))
{
}

RequestInfo WSStaticHttpProtocol::process()
{
    static const RegularExpression matchImageFiles(R"(\.(png|gif|jpg|jpeg|pcx)$)", "i");

    RequestInfo requestInfo("HTTP GET");

    const String fullPath(m_staticFilesDirectory + m_url.path());

    requestInfo.request.input(Buffer(fullPath), "");

    Strings contentEncodings;
    if (!matchImageFiles.matches(m_url.path()))
    {
        contentEncodings.push_back("gzip");
    }
    try
    {
        filesystem::path filePath(fullPath.c_str());
        if (!exists(filePath))
        {
            // If the file not found, redirect to index.html.
            filePath = m_staticFilesDirectory + "index.html";
        }

        requestInfo.response.content().loadFromFile(filePath);

        const Buffer output = requestInfo.response.output(contentEncodings);

        // Header lines end with a carriage return and a newline, which is what HTTP says they end
        // with, and what the rest of this server sends. Browsers and curl accept a bare newline
        // as well - so this went unnoticed - but anything reading the response by the book finds
        // no end to the headers and waits for the rest of a message that has already arrived.
        socket().write("HTTP/1.1 200 OK\r\n");
        String contentType = "text/html";
        if (fullPath.ends_with(".css"))
        {
            contentType = "text/css";
        }
        else if (fullPath.ends_with(".js"))
        {
            contentType = "text/javascript";
        }
        socket().write("Content-Type: " + contentType + "; charset=utf-8\r\n");
        if (!requestInfo.response.contentEncoding().empty())
        {
            socket().write("Content-Encoding: " + requestInfo.response.contentEncoding() + "\r\n");
        }
        socket().write("Content-Length: " + to_string(output.size()) + "\r\n\r\n");
        socket().write(output);
    }
    catch (const Exception&)
    {
        const String text(
            "<html><head><title>Not Found</title></head><body>Resource " + m_staticFilesDirectory + m_url.path() +
            " was not found.</body></html>\n");
        const Buffer output = requestInfo.response.output(contentEncodings);
        requestInfo.response.content() = text;
        socket().write("HTTP/1.1 404 Not Found\r\n");
        socket().write("Content-Type: text/html; charset=utf-8\r\n");
        if (!requestInfo.response.contentEncoding().empty())
        {
            socket().write("Content-Encoding: " + requestInfo.response.contentEncoding() + "\r\n");
        }
        socket().write("Content-Length: " + to_string(text.length()) + "\r\n\r\n");
        socket().write(text);
    }
    return requestInfo;
}
