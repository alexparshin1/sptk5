/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;

Exception::Exception(const String& text, const fs::path& file, int line, const String& description) DOESNT_THROW
    : m_file(file.string())
    , m_line(line)
    , m_text(text)
    , m_description(description)
    , m_fullMessage(m_text)
{
    if (m_line != 0 && !m_file.empty())
    {
        RegularExpression matchFileName(R"(([^\\\/]+[\\\/][^\\\/]+)$)");
        String fname(file.string());
        if (auto matches = matchFileName.m(file.string().c_str()); !matches.empty())
        {
            fname = matches[0].value;
        }
        m_fullMessage += " in " + fname + "(" + int2string(uint32_t(m_line)) + ")";
    }

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

TimeoutException::TimeoutException(const String& text, const fs::path& file, int line,
                                   const String& description) DOESNT_THROW
    : Exception(text, file, line, description)
{
}

ConnectionException::ConnectionException(const String& text, const fs::path& file, int line,
                                         const String& description) DOESNT_THROW
    : Exception(text, file, line, description)
{
}

DatabaseException::DatabaseException(const String& text, const fs::path& file, int line,
                                     const String& description) DOESNT_THROW
    : Exception(text, file, line, description)
{
}

SOAPException::SOAPException(const String& text, const fs::path& file, int line, const String& description) DOESNT_THROW
    : Exception(text, file, line, description)
{
}

HTTPException::HTTPException(size_t statusCode, const String& text, const fs::path& file, int line,
                             const String& description) DOESNT_THROW
    : Exception(text, file, line, description)
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

    auto itor = statusCodeInfo.find(statusCode);
    if (itor == statusCodeInfo.end())
    {
        return "Unknown";
    }
    return itor->second;
}

#ifdef USE_GTEST

TEST(SPTK_Exception, throwException)
{
    try
    {
        throw Exception("Test exception");
    }
    catch (const Exception& e)
    {
        EXPECT_STREQ("Test exception", e.what());
    }

    constexpr int testLineNumber = 1234;
    try
    {
        throw Exception("Test exception", __FILE__, testLineNumber, "This happens sometimes");
    }
    catch (const Exception& e)
    {
#ifdef _WIN32
        EXPECT_STREQ("Test exception in core\\Exception.cpp(1234). This happens sometimes.", e.what());
#else
        EXPECT_STREQ("Test exception in core/Exception.cpp(1234). This happens sometimes.", e.what());
#endif
        EXPECT_STREQ("Test exception", e.message().c_str());
        EXPECT_STREQ(__FILE__, e.file().c_str());
        EXPECT_EQ(testLineNumber, e.line());
    }
}

TEST(SPTK_HttpException, throw)
{
    constexpr size_t firstErrorCode = 400;
    constexpr size_t maxErrorCode = 512;
    constexpr int testLineNumber = 1234;
    for (size_t code = firstErrorCode; code < maxErrorCode; ++code)
    {
        auto expectedStatus = HTTPException::httpResponseStatus(code);
        if (expectedStatus.empty())
        {
            continue;
        }
        try
        {
            throw HTTPException(code, "Something happened", __FILE__, testLineNumber, "This happens sometimes");
        }
        catch (const HTTPException& e)
        {
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
