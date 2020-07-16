/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
        String fname(file);
        auto matches = matchFileName.m(file);
        if (!matches.empty())
            fname = matches[size_t(0)].value;
		m_fullMessage += " in " + fname + "(" + int2string(uint32_t(m_line)) + ")";
    }

    if (!m_description.empty())
        m_fullMessage += ". " + m_description + ".";
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
    m_statusText = httpResponseStatus(statusCode);
}

String HTTPException::httpResponseStatus(size_t statusCode)
{
    String statusText("");

    switch (statusCode) {
        case 400:
            statusText = "Bad Request";
            break;
        case 401:
            statusText = "Unauthorized";
            break;
        case 402:
            statusText = "Payment Required";
            break;
        case 403:
            statusText = "Forbidden";
            break;
        case 404:
            statusText = "Not Found";
            break;
        case 405:
            statusText = "Method Not Allowed";
            break;
        case 406:
            statusText = "Not Acceptable";
            break;
        case 407:
            statusText = "Proxy Authentication Required";
            break;
        case 408:
            statusText = "Request Timeout";
            break;
        case 409:
            statusText = "Conflict";
            break;
        case 410:
            statusText = "Gone";
            break;
        case 411:
            statusText = "Length Required";
            break;
        case 412:
            statusText = "Precondition Failed";
            break;
        case 413:
            statusText = "Payload Too Large";
            break;
        case 414:
            statusText = "URI Too Long";
            break;
        case 415:
            statusText = "Unsupported Media Type";
            break;
        case 416:
            statusText = "Range Not Satisfiable";
            break;
        case 417:
            statusText = "Expectation Failed";
            break;
        case 418:
            statusText = "I'm a teapot";
            break;
        case 421:
            statusText = "Misdirected Request";
            break;
        case 424:
            statusText = "Failed Dependency";
            break;
        case 426:
            statusText = "Upgrade Required";
            break;
        case 428:
            statusText = "Precondition Required";
            break;
        case 429:
            statusText = "Too Many Requests";
            break;
        case 431:
            statusText = "Request Header Fields Too Large";
            break;
        case 451:
            statusText = "Unavailable For Legal Reasons";
            break;

        case 500:
            statusText = "Internal Server Error";
            break;
        case 501:
            statusText = "Not Implemented";
            break;
        case 502:
            statusText = "Bad Gateway";
            break;
        case 503:
            statusText = "Service Unavailable";
            break;
        case 504:
            statusText = "Gateway Timeout";
            break;
        case 505:
            statusText = "HTTP Version Not Supported";
            break;
        case 510:
            statusText = "Not Extended";
            break;
        case 511:
            statusText = "Network Authentication Required";
            break;
    }
    return statusText;
}

#if USE_GTEST

TEST(SPTK_Exception, throw)
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
#ifdef _WIN32
		EXPECT_STREQ("Test exception in core\\Exception.cpp(1234). This happens sometimes.", e.what());
#else
		EXPECT_STREQ("Test exception in core/Exception.cpp(1234). This happens sometimes.", e.what());
#endif
        EXPECT_STREQ("Test exception", e.message().c_str());
        EXPECT_STREQ(__FILE__, e.file().c_str());
        EXPECT_EQ(1234, e.line());
    }
}

TEST(SPTK_HttpException, throw)
{
    for (int code = 400; code < 512; ++code) {
        auto expectedStatus = HTTPException::httpResponseStatus(code);
        if (expectedStatus.empty())
            continue;
        try {
            throw HTTPException(code, "Something happened", __FILE__, 1234, "This happens sometimes");
        }
        catch (const HTTPException& e) {
#ifdef _WIN32
            EXPECT_STREQ("Something happened in core\\Exception.cpp(1234). This happens sometimes.", e.what());
#else
            EXPECT_STREQ("Something happened in core/Exception.cpp(1234). This happens sometimes.", e.what());
#endif
            EXPECT_STREQ("Something happened", e.message().c_str());
            EXPECT_STREQ(__FILE__, e.file().c_str());
            EXPECT_EQ(1234, e.line());
            EXPECT_EQ(size_t(code), e.statusCode());
            EXPECT_EQ(expectedStatus, e.statusText());
        }
    }
}

#endif
