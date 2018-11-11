/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Exception.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/Exception.h>
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;

Exception::Exception(const String& text, const String& file, int line, const String& description) DOESNT_THROW
: m_file(file), m_line(line), m_text(text), m_description(description), m_fullMessage(m_text)
{
    if (m_line != 0 && !m_file.empty()) {
        RegularExpression matchFileName(R"(([^\\\/]+[\\\/][^\\\/]+)$)");
        Strings matches;
        String fname(file);
        if (matchFileName.m(file, matches))
            fname = matches[0];
        m_fullMessage += " in " + fname + "(" + int2string(uint32_t(m_line)) + ")";
    }

    if (!m_description.empty())
        m_fullMessage += ". " + m_description + ".";
}

Exception::Exception(const Exception& other) DOESNT_THROW
: m_file(other.m_file), m_line(other.m_line), m_text(other.m_text), m_description(other.m_description), m_fullMessage(other.m_fullMessage)
{
}

const char* Exception::what() const DOESNT_THROW
{
    return m_fullMessage.c_str();
}

String Exception::message() const
{
    return m_text;
}

String Exception::file() const
{
    return m_file;
}

int Exception::line() const
{
    return m_line;
}

String Exception::description() const
{
    return m_description;
}

TimeoutException::TimeoutException(const String& text, const String& file, int line, const String& description) DOESNT_THROW
: Exception(text, file, line, description)
{
}

ConnectionException::ConnectionException(const String& text, const String& file, int line, const String& description) DOESNT_THROW
: Exception(text, file, line, description)
{
}

DatabaseException::DatabaseException(const String& text, const String& file, int line, const String& description) DOESNT_THROW
: Exception(text, file, line, description)
{
}

SOAPException::SOAPException(const String& text, const String& file, int line, const String& description) DOESNT_THROW
: Exception(text, file, line, description)
{
}

HTTPException::HTTPException(size_t statusCode, const String& text, const String& file, int line, const String& description) DOESNT_THROW
: Exception(text, file, line, description), m_statusCode(statusCode)
{
    switch (statusCode) {
        case 400:
            m_statusText = "Bad Request";
            break;
        case 401:
            m_statusText = "Unauthorized";
            break;
        case 402:
            m_statusText = "Payment Required";
            break;
        case 403:
            m_statusText = "Forbidden";
            break;
        case 404:
            m_statusText = "Not Found";
            break;
        case 405:
            m_statusText = "Method Not Allowed";
            break;
        case 406:
            m_statusText = "Not Acceptable";
            break;
        case 407:
            m_statusText = "Proxy Authentication Required";
            break;
        case 408:
            m_statusText = "Request Timeout";
            break;
        case 409:
            m_statusText = "Conflict";
            break;
        case 410:
            m_statusText = "Gone";
            break;
        case 411:
            m_statusText = "Length Required";
            break;
        case 412:
            m_statusText = "Precondition Failed";
            break;
        case 413:
            m_statusText = "Payload Too Large";
            break;
        case 414:
            m_statusText = "URI Too Long";
            break;
        case 415:
            m_statusText = "Unsupported Media Type";
            break;
        case 416:
            m_statusText = "Range Not Satisfiable";
            break;
        case 417:
            m_statusText = "Expectation Failed";
            break;
        case 418:
            m_statusText = "I'm a teapot";
            break;
        case 421:
            m_statusText = "Misdirected Request";
            break;
        case 424:
            m_statusText = "Failed Dependency";
            break;
        case 426:
            m_statusText = "Upgrade Required";
            break;
        case 428:
            m_statusText = "Precondition Required";
            break;
        case 429:
            m_statusText = "Too Many Requests";
            break;
        case 431:
            m_statusText = "Request Header Fields Too Large";
            break;
        case 451:
            m_statusText = "Unavailable For Legal Reasons";
            break;
        case 500:
            m_statusText = "Internal Server Error";
            break;
        case 501:
            m_statusText = "Not Implemented";
            break;
        case 502:
            m_statusText = "Bad Gateway";
            break;
        case 503:
            m_statusText = "Service Unavailable";
            break;
        case 504:
            m_statusText = "Gateway Timeout";
            break;
        case 505:
            m_statusText = "HTTP Version Not Supported";
            break;
        case 510:
            m_statusText = "Not Extended";
            break;
        case 511:
            m_statusText = "Network Authentication Required";
            break;
        default:
            m_statusText = "Status undefined";
            break;
    }
}

#if USE_GTEST
#include <gtest/gtest.h>

TEST(Exception, throw)
{
    try {
        throw Exception("Test exception");
    }
    catch (const Exception& e) {
        EXPECT_STREQ("Test exception", e.what());
    }

    try {
        throw Exception("Test exception", __FILE__, 1234, "This happens sometimes");
    }
    catch (const Exception& e) {
        EXPECT_STREQ("Test exception in core/Exception.cpp(1234). This happens sometimes.", e.what());
        EXPECT_STREQ("Test exception", e.message().c_str());
        EXPECT_STREQ(__FILE__, e.file().c_str());
        EXPECT_EQ(1234, e.line());
    }
}

TEST(HttpException, throw)
{
    try {
        throw HTTPException(404, "File not found", __FILE__, 1234, "This happens sometimes");
    }
    catch (const HTTPException& e) {
        EXPECT_STREQ("File not found in core/Exception.cpp(1234). This happens sometimes.", e.what());
        EXPECT_STREQ("File not found", e.message().c_str());
        EXPECT_STREQ(__FILE__, e.file().c_str());
        EXPECT_EQ(1234, e.line());
        EXPECT_EQ(size_t(404), e.statusCode());
        EXPECT_STREQ("Not Found", e.statusText().c_str());
    }

    try {
        throw HTTPException(500, "Something happen", __FILE__, 1234, "This happens sometimes");
    }
    catch (const HTTPException& e) {
        EXPECT_STREQ("Something happen in core/Exception.cpp(1234). This happens sometimes.", e.what());
        EXPECT_STREQ("Something happen", e.message().c_str());
        EXPECT_STREQ(__FILE__, e.file().c_str());
        EXPECT_EQ(1234, e.line());
        EXPECT_EQ(size_t(500), e.statusCode());
        EXPECT_STREQ("Internal Server Error", e.statusText().c_str());
    }
}

#endif
