/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Exception.h>
#include <sptk5/RegularExpression.h>

#include <utility>

using namespace std;
using namespace sptk;


Exception::Exception(String text, const std::source_location& location, String description) noexcept
    : m_location(location)
    , m_text(std::move(text))
    , m_description(std::move(description))
    , m_fullMessage(m_text)
{
    filesystem::path filePath = m_location.file_name();

    const String fileName = filePath.filename().string();
    filePath = filePath.parent_path().filename() / fileName.c_str();
    m_fullMessage += " in " + filePath.string() + "(" + to_string(m_location.line()) + ")";

    if (!m_description.empty())
    {
        m_fullMessage += ". " + m_description + ".";
    }
}

const char* Exception::what() const DOESNT_THROW
{
    return m_fullMessage.c_str();
}

String Exception::message() const
{
    return m_text;
}

std::source_location Exception::location() const
{
    return m_location;
}

String Exception::description() const
{
    return m_description;
}

[[maybe_unused]] TimeoutException::TimeoutException(const String& text, const std::source_location& location, const String& description) DOESNT_THROW
    : Exception(text, location, description)
{
}

ConnectionException::ConnectionException(const String& text, const std::source_location& location,
                                         const String& description) DOESNT_THROW
    : Exception(text, location, description)
{
}

DatabaseException::DatabaseException(const String& text, const std::source_location& location, const String& description) DOESNT_THROW
    : Exception(text, location, description)
{
}

SOAPException::SOAPException(const String& text, const std::source_location& location, const String& description) DOESNT_THROW
    : Exception(text, location, description)
{
}

HTTPException::HTTPException(size_t statusCode, const String& text, const std::source_location& location, const String& description) DOESNT_THROW
    : Exception(text, location, description)
    , m_statusCode(statusCode)
{
    m_statusText = httpResponseStatus(statusCode);
}

String HTTPException::httpResponseStatus(size_t statusCode)
{
    static const map<size_t, const char*> statusCodeInfo {
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {402, "Payment Required"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {407, "Proxy Authentication Required"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {412, "Precondition Failed"},
        {413, "Payload Too Large"},
        {414, "URI Too Long"},
        {415, "Unsupported Media Type"},
        {416, "Range Not Satisfiable"},
        {417, "Expectation Failed"},
        {418, "I'm a teapot"},
        {421, "Misdirected Request"},
        {424, "Failed Dependency"},
        {426, "Upgrade Required"},
        {428, "Precondition Required"},
        {429, "Too Many Requests"},
        {431, "Request Header Fields Too Large"},
        {451, "Unavailable For Legal Reasons"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Timeout"},
        {505, "HTTP Version Not Supported"},
        {510, "Not Extended"},
        {511, "Network Authentication Required"}};

    const auto itor = statusCodeInfo.find(statusCode);
    if (itor == statusCodeInfo.end())
    {
        return "Unknown";
    }
    return itor->second;
}
