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

#include <cstdio>
#include <cstring>
#include <sstream>

#ifdef _MSC_VER
#pragma warning (disable: 4786)
#endif

#include <sptk5/Base64.h>
#include <sptk5/DateTime.h>
#include <sptk5/net/BaseMailConnect.h>

static constexpr int LINE_CHARS = 72;

using namespace std;
using namespace sptk;

class ContentTypes
{
public:
    static string type(const string& fileName);

private:
    static const map<string, string, std::less<>> m_contentTypes;
};

const map<string, string, std::less<>> ContentTypes::m_contentTypes
    {
        {"txt",  "text/plain"},
        {"htm",  "text/html"},
        {"html", "text/html"},
        {"gif",  "image/gif"},
        {"png",  "image/png"},
        {"bmp",  "image/bmp"},
        {"jpg",  "image/jpeg"},
        {"tif",  "image/tiff"},
        {"pdf",  "application/pdf"},
        {"xls",  "application/vnd.ms-excel"},
        {"csv",  "text/plain"},
        {"doc",  "application/msword"},
        {"wav",  "application/data"}
    };

string ContentTypes::type(const string& fileName)
{
    if (const char* extension = strrchr(fileName.c_str(), '.'); extension != nullptr)
    {
        ++extension;
        if (strlen(extension) > 4)
        {
            return "application/octet-stream";
        }
        const auto itor = m_contentTypes.find(extension);
        if (itor != m_contentTypes.end())
        {
            return itor->second;
        }
    }
    return "application/octet-stream";
}

void BaseMailConnect::mimeFile(const String& fileName, const String& fileAlias, stringstream& message)
{
    Buffer bufSource;
    String strDest;

    bufSource.loadFromFile(fileName);

    String ctype = ContentTypes::type(trim(fileName));

    message << "Content-Type: " << ctype << "; name=\"" << fileAlias << "\"" << endl;
    message << "Content-Transfer-Encoding: base64" << endl;
    message << "Content-Disposition: attachment; filename=\"" << fileAlias << "\"" << endl << endl;

    Buffer buffer;

    Base64::encode(strDest, bufSource);

    // Split encoded data to lines
    size_t dataLen = strDest.length();
    buffer.checkSize(dataLen + dataLen / LINE_CHARS);

    const char* ptr = strDest.c_str();
    for (size_t pos = 0; pos < dataLen; pos += LINE_CHARS)
    {
        size_t lineLen = dataLen - pos;
        if (lineLen > LINE_CHARS)
        {
            lineLen = LINE_CHARS;
        }
        buffer.append(ptr + pos, lineLen);
        buffer.append('\n');
    }
    message << buffer.data();
}

void BaseMailConnect::mimeMessage(Buffer& buffer)
{
    static const char boundary[] = "--MESSAGE-MIME-BOUNDARY--";
    static const char boundary2[] = "--TEXT-MIME-BOUNDARY--";
    stringstream message;

    if (!m_from.empty())
    {
        message << "From: " << m_from << endl;
    }
    else
    {
        message << "From: postmaster" << endl;
    }

    m_to = m_to.replace(";", ", ");
    message << "To: " << m_to << endl;

    if (!m_cc.empty())
    {
        m_cc = m_cc.replace(";", ", ");
        message << "CC: " << m_cc << endl;
    }

    message << "Subject: " << m_subject << endl;

    const DateTime date = DateTime::Now();
    short dy;
    short dm;
    short dd;
    short wd;
    short yd;
    short th;
    short tm;
    short ts;
    short tms;

    date.decodeDate(&dy, &dm, &dd, &wd, &yd);
    date.decodeTime(&th, &tm, &ts, &tms);

    array<char, 128> dateBuffer = {};
    const char* sign = "-";
    int offset = TimeZone::offset();
    if (offset >= 0)
    {
        sign = "";
    }
    else
    {
        offset = -offset;
    }

    int len = snprintf(dateBuffer.data(), sizeof(dateBuffer) - 1,
                       "Date: %s, %i %s %04i %02i:%02i:%02i %s%04i (%s)",
                       date.dayOfWeekName().substr(0, 3).c_str(),
                       dd,
                       DateTime::format(DateTime::Format::MONTH_NAME, size_t(dm) - 1).substr(0, 3).c_str(),
                       dy,
                       th, tm, ts,
                       sign,
                       offset * 100,
                       TimeZone::name().c_str()
    );

    message << String(dateBuffer.data(), (size_t) len) << endl;

    message << "MIME-Version: 1.0" << endl;
    message << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"" << endl << endl;

    message << endl << "--" << boundary << endl;

    if (m_body.type() == MailMessageType::PLAIN_TEXT_MESSAGE)
    {
        message << "Content-Type: text/plain; charset=ISO-8859-1" << endl;
        message << "Content-Transfer-Encoding: 7bit" << endl;
        message << "Content-Disposition: inline" << endl << endl;
        message << m_body.text() << endl << endl;
    }
    else
    {
        message << "Content-Type: multipart/alternative;  boundary=\"" << boundary2 << "\"" << endl << endl;

        message << endl << "--" << boundary2 << endl;
        message << "Content-Type: text/plain; charset=ISO-8859-1" << endl;
        message << "Content-Disposition: inline" << endl;
        message << "Content-Transfer-Encoding: 8bit" << endl << endl;

        message << m_body.text() << endl << endl;

        message << endl << "--" << boundary2 << endl;
        message << "Content-Type: text/html; charset=ISO-8859-1" << endl;
        message << "Content-Disposition: inline" << endl;
        message << "Content-Transfer-Encoding: 7bit" << endl << endl;

        message << m_body.html() << endl << endl;
        message << endl << "--" << boundary2 << "--" << endl;
    }

    Strings sl(m_attachments, ";");
    for (const auto& attachment: sl)
    {
        String attachmentAlias(attachment);
        const char* separator = "\\";
        if (attachment.find('/') != STRING_NPOS)
        {
            separator = "/";
        }
        Strings attachmentParts(attachment, separator);

        if (auto attachmentPartsCount = (uint32_t) attachmentParts.size(); attachmentPartsCount > 1)
        {
            attachmentAlias = attachmentParts[attachmentPartsCount - 1].c_str();
        }

        if (!attachment.empty())
        {
            message << endl << "--" << boundary << endl;
            mimeFile(attachment, attachmentAlias, message);
        }
    }

    message << endl << "--" << boundary << "--" << endl;

    buffer.reset(2048);
    buffer.append(message.str().c_str(), (uint32_t) message.str().length());
    buffer.saveToFile("/tmp/mimed.txt");
}
