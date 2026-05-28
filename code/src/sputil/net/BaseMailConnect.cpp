/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-21                                             ║
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

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <sptk5/Base64.h>
#include <sptk5/DateTime.h>
#include <sptk5/net/BaseMailConnect.h>

static constexpr auto LINE_CHARS = 72;

using namespace std;
using namespace sptk;

class ContentTypes
{
public:
    static string type(const string& fileName);

private:
    static const map<string, string, std::less<>> m_contentTypes;
};

const map<string, string, std::less<>> ContentTypes::m_contentTypes {
    {"txt", "text/plain"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"xhtml", "text/html"},
    {"gif", "image/gif"},
    {"png", "image/png"},
    {"bmp", "image/bmp"},
    {"jpg", "image/jpeg"},
    {"tif", "image/tiff"},
    {"tiff", "image/tiff"},
    {"pdf", "application/pdf"},
    {"xls", "application/vnd.ms-excel"},
    {"csv", "text/plain"},
    {"doc", "application/msword"},
    {"wav", "application/data"}};

string ContentTypes::type(const string& fileName)
{
    if (const char* extension = strrchr(fileName.c_str(), '.'); extension != nullptr)
    {
        ++extension;
        if (const auto iterator = m_contentTypes.find(extension);
            iterator != m_contentTypes.end())
        {
            return iterator->second;
        }
    }
    return "application/octet-stream";
}

void BaseMailConnect::mimeFile(const String& fileName, const String& fileAlias, stringstream& message)
{
    Buffer bufSource;
    String strDest;

    bufSource.loadFromFile(fileName.c_str());

    const String contentType = ContentTypes::type(trim(fileName));

    message << "Content-Type: " << contentType << "; name=\"" << fileAlias << "\"\n";
    message << "Content-Transfer-Encoding: base64" << '\n';
    message << "Content-Disposition: attachment; filename=\"" << fileAlias << "\"\n\n";

    Buffer buffer;

    Base64::encode(strDest, bufSource);

    // Split encoded data to lines
    const auto dataLen = strDest.length();
    buffer.checkSize(dataLen + dataLen / LINE_CHARS);

    const char* ptr = strDest.c_str();
    for (size_t pos = 0; pos < dataLen; pos += LINE_CHARS)
    {
        auto lineLen = dataLen - pos;
        lineLen = std::min<size_t>(lineLen, LINE_CHARS);
        buffer.append(ptr + pos, lineLen);
        buffer.append('\n');
    }
    message << buffer.data();
}

void BaseMailConnect::mimeMessage(Buffer& buffer) const
{
    auto         boundary = format("--MESSAGE-MIME-BOUNDARY-{}--", DateTime::Now().timePoint().time_since_epoch().count());
    stringstream message;

    if (!m_from.empty())
    {
        message << "From: " << m_from << '\n';
    }
    else
    {
        message << "From: postmaster" << '\n';
    }

    message << "To: " << m_to.replace(";", ", ") << '\n';

    if (!m_cc.empty())
    {
        message << "CC: " << m_cc.replace(";", ", ") << '\n';
    }

    message << "Subject: " << m_subject << '\n';

    const DateTime date = DateTime::Now();
    short          year {0};
    short          month {0};
    short          day {0};
    short          weekDay {0};
    short          yearDay {0};
    short          hour {0};
    short          minute {0};
    short          second {0};
    short          millisecond {0};

    date.decodeDate(&year, &month, &day, &weekDay, &yearDay);
    date.decodeTime(&hour, &minute, &second, &millisecond);

    constexpr auto sixtySeconds = 60;
    auto           sign = "-";
    const auto     tzOffset = static_cast<int>(TimeZone::offset().count());
    auto           offsetHours = TimeZone::offset().count() / sixtySeconds;
    const auto     offsetMinutes = TimeZone::offset().count() % sixtySeconds;
    if (tzOffset >= 0)
    {
        sign = "+";
    }
    else
    {
        offsetHours = -offsetHours;
    }

    const auto dayOfWeekName = date.dayOfWeekName().substr(0, 3);
    const auto monthName = DateTime::format(DateTime::Format::MONTH_NAME, static_cast<size_t>(month) - 1).substr(0, 3);
    const auto dateBuffer = format(
        "Date: {}, {} {} {:04d} {:02d}:{:02d}:{:02d} {}{:02d}{:02d} ({})",
        dayOfWeekName, day, monthName, year,
        hour, minute, second,
        sign, static_cast<int>(offsetHours), static_cast<int>(offsetMinutes),
        TimeZone::name().c_str());

    message << dateBuffer << '\n';

    message << "MIME-Version: 1.0" << '\n';
    message << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"" << '\n'
            << '\n';

    message << '\n'
            << "--" << boundary << '\n';

    if (m_body.type() == MailMessageType::PLAIN_TEXT_MESSAGE)
    {
        message << "Content-Type: text/plain; charset=UTF-8" << '\n';
        message << "Content-Transfer-Encoding: 8bit" << '\n';
        message << "Content-Disposition: inline" << '\n'
                << '\n';
        message << m_body.text() << '\n'
                << '\n';
    }
    else
    {
        auto boundary2 = format("--MESSAGE-MIME-BOUNDARY-{}--", DateTime::Now().timePoint().time_since_epoch().count() + 1);

        message << "Content-Type: multipart/alternative;  boundary=\"" << boundary2 << "\"" << '\n'
                << '\n';

        message << '\n'
                << "--" << boundary2 << '\n';
        message << "Content-Type: text/plain; charset=UTF-8" << '\n';
        message << "Content-Disposition: inline" << '\n';
        message << "Content-Transfer-Encoding: 8bit" << '\n'
                << '\n';

        message << m_body.text() << '\n'
                << '\n';

        message << '\n'
                << "--" << boundary2 << '\n';
        message << "Content-Type: text/html; charset=UTF-8" << '\n';
        message << "Content-Disposition: inline" << '\n';
        message << "Content-Transfer-Encoding: 8bit" << '\n'
                << '\n';

        message << m_body.html() << '\n'
                << '\n';
        message << '\n'
                << "--" << boundary2 << "--" << '\n';
    }


    for (const Strings strings(m_attachments, ";");
         const auto&   attachment: strings)
    {
        String attachmentAlias(attachment);
        auto   separator = "\\";
        if (attachment.find('/') != string::npos)
        {
            separator = "/";
        }
        Strings attachmentParts(attachment, separator);

        if (const auto attachmentPartsCount = static_cast<uint32_t>(attachmentParts.size()); attachmentPartsCount > 1)
        {
            attachmentAlias = attachmentParts[attachmentPartsCount - 1].c_str();
        }

        if (!attachment.empty())
        {
            message << "\n--" << boundary << "\n";
            mimeFile(attachment, attachmentAlias, message);
        }
    }

    message << "\n--" << boundary << "--\n";

    buffer = message.str();
}
