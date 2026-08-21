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

namespace {

/// @brief Tells whether a resolved path is the directory itself or something under it.
///
/// Compared component by component rather than as text, so that a directory whose name merely
/// starts with the root's - '/var/www-old' against '/var/www' - is not taken for a part of it.
///
/// @param root         Resolved directory.
/// @param candidate    Resolved path to test.
/// @returns True when the candidate is contained in the root.
bool isUnder(const filesystem::path& root, const filesystem::path& candidate)
{
    auto rootPart = root.begin();
    auto candidatePart = candidate.begin();
    for (; rootPart != root.end(); ++rootPart, ++candidatePart)
    {
        if (candidatePart == candidate.end() || *candidatePart != *rootPart)
        {
            return false;
        }
    }
    return true;
}

} // namespace

filesystem::path WSStaticHttpProtocol::resolveFile(const String& staticFilesDirectory, const String& path)
{
    error_code error;

    const auto root = weakly_canonical(filesystem::path(staticFilesDirectory.c_str()), error).lexically_normal();
    if (error || root.empty())
    {
        return {};
    }

    // Appended, not joined with a separator: a request path begins with one, and joining an
    // absolute path to the root would replace the root instead of extending it.
    auto requested = String(path).trim();
    while (requested.starts_with("/"))
    {
        requested = requested.substr(1);
    }

    const auto candidate = weakly_canonical(root / filesystem::path(requested.c_str()), error).lexically_normal();
    if (error)
    {
        return {};
    }

    if (!isUnder(root, candidate))
    {
        return {};
    }

    return candidate;
}

RequestInfo WSStaticHttpProtocol::process()
{
    static const RegularExpression matchImageFiles(R"(\.(png|gif|jpg|jpeg|pcx)$)", "i");

    RequestInfo requestInfo("HTTP GET");

    requestInfo.request.input(Buffer(m_url.path()), "");

    Strings contentEncodings;
    if (!matchImageFiles.matches(m_url.path()))
    {
        contentEncodings.push_back("gzip");
    }
    try
    {
        auto filePath = resolveFile(m_staticFilesDirectory, m_url.path());
        if (filePath.empty())
        {
            // The request leads outside the directory the files are served from. Nothing more is
            // owed than that it is not here - whether such a file exists is not the caller's business.
            throw Exception("Resource not found");
        }
        if (!exists(filePath))
        {
            // If the file not found, redirect to index.html.
            filePath = resolveFile(m_staticFilesDirectory, "index.html");
        }

        requestInfo.response.content().loadFromFile(filePath);

        const Buffer output = requestInfo.response.output(contentEncodings);

        // Header lines end with a carriage return and a newline, which is what HTTP says they end
        // with, and what the rest of this server sends. Browsers and curl accept a bare newline
        // as well - so this went unnoticed - but anything reading the response by the book finds
        // no end to the headers and waits for the rest of a message that has already arrived.
        socket().write("HTTP/1.1 200 OK\r\n");
        String contentType = "text/html";
        if (m_url.path().ends_with(".css"))
        {
            contentType = "text/css";
        }
        else if (m_url.path().ends_with(".js"))
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
        // The requested path is not repeated back. It told the caller where the server keeps its
        // files, and it went into the page unescaped, which is a way to put markup there.
        const String text("<html><head><title>Not Found</title></head><body>The requested resource was not found.</body></html>\n");
        requestInfo.response.content() = text;

        // Encoded once the content is in place, and the encoded form is what goes out. The headers
        // below describe this buffer; they used to describe one produced before there was content,
        // so a 404 could announce an encoding that its body did not have.
        const Buffer output = requestInfo.response.output(contentEncodings);
        socket().write("HTTP/1.1 404 Not Found\r\n");
        socket().write("Content-Type: text/html; charset=utf-8\r\n");
        if (!requestInfo.response.contentEncoding().empty())
        {
            socket().write("Content-Encoding: " + requestInfo.response.contentEncoding() + "\r\n");
        }
        socket().write("Content-Length: " + to_string(output.size()) + "\r\n\r\n");
        socket().write(output);
    }
    return requestInfo;
}
