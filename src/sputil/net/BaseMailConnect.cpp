/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseMailConnect.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <stdio.h>
#include <string.h>
#include <sstream>

#ifdef _MSC_VER
#pragma warning (disable: 4786)
#endif

#include <sptk5/Base64.h>
#include <sptk5/net/BaseMailConnect.h>
#include <sptk5/Strings.h>
#include <sptk5/DateTime.h>

#define LINE_CHARS 72

using namespace std;
using namespace sptk;

class ContentTypes
{
    static map<string, string> m_contentTypes;
public:
    ContentTypes()
    { if (!m_contentTypes.size()) init(); }

    void init();

    static string type(string fileName);
};

map<string, string> ContentTypes::m_contentTypes;
static ContentTypes contentTypes;

void ContentTypes::init()
{
    m_contentTypes["txt"] = "text/plain";
    m_contentTypes["htm"] = "text/html";
    m_contentTypes["html"] = "text/html";
    m_contentTypes["gif"] = "image/gif";
    m_contentTypes["png"] = "image/png";
    m_contentTypes["bmp"] = "image/bmp";
    m_contentTypes["jpg"] = "image/jpeg";
    m_contentTypes["tif"] = "image/tiff";
    m_contentTypes["pdf"] = "application/pdf";
    m_contentTypes["xls"] = "application/vnd.ms-excel";
    m_contentTypes["csv"] = "text/plain";
    m_contentTypes["doc"] = "application/msword";
    m_contentTypes["wav"] = "application/data";
}

string ContentTypes::type(string fileName)
{
    const char* extension = strrchr(fileName.c_str(), '.');
    if (!extension)
        return "application/octet-stream";
    extension++;
    if (strlen(extension) > 4)
        return "application/octet-stream";
    const map<string, string>::iterator itor = m_contentTypes.find(extension);
    if (itor != m_contentTypes.end()) return itor->second;
    else
        return "application/octet-stream";
}

BaseMailConnect::~BaseMailConnect()
{
}


void BaseMailConnect::mimeFile(string fileName, string fileAlias, stringstream& message)
{
    Buffer bufSource;
    string strDest;
    //char    *header = new char[1024];

    bufSource.loadFromFile(fileName.c_str());

    string ctype = contentTypes.type(trim(fileName));

    message << "Content-Type: " << ctype << "; name=\"" << fileAlias << "\"" << endl;
    message << "Content-Transfer-Encoding: base64" << endl;
    message << "Content-Disposition: attachment; filename=\"" << fileAlias << "\"" << endl << endl;

    Buffer buffer;

    Base64::encode(strDest, bufSource);
    uint32_t cnt = (uint32_t) strDest.length();
    const char* data = strDest.c_str();
    char line[90];
    for (uint32_t p = 0; p < cnt; p += LINE_CHARS) {
        uint32_t length = cnt - p;
        if (length > LINE_CHARS) length = LINE_CHARS;
        memcpy(line, data + p, 80);
        line[length] = '\n';
        //line[length+1] = '\n';
        length++;
        line[length] = 0;
        buffer.append(line, length);
    }
    message << buffer.data();
}

void BaseMailConnect::mimeMessage(Buffer& buffer)
{
    static const char boundary[] = "--MESSAGE-MIME-BOUNDARY--";
    static const char boundary2[] = "--TEXT-MIME-BOUNDARY--";
    stringstream message;

    if (m_from.length())
        message << "From: " << m_from << endl;
    else
        message << "From: postmaster" << endl;

    replaceAll(m_to, ";", ", ");
    message << "To: " << m_to << endl;

    if (m_cc.length()) {
        m_cc = replaceAll(m_cc, ";", ", ");
        message << "CC: " << m_cc << endl;
    }

    message << "Subject: " << m_subject << endl;

    DateTime date = DateTime::Now();
    short dy, dm, dd, th, tm, ts, tms;
    date.decodeDate(&dy, &dm, &dd);
    date.decodeTime(&th, &tm, &ts, &tms);

    char dateBuffer[128];
    const char* sign = "-";
    int offset = DateTime::timeZoneOffset;
    if (offset >= 0)
        sign = "";
    else
        offset = -offset;

    sprintf(dateBuffer,
            "Date: %s, %i %s %04i %02i:%02i:%02i %s%04i (%s)",
            date.dayOfWeekName().substr(0, 3).c_str(),
            dd,
            DateTime::monthNames[dm - 1].substr(0, 3).c_str(),
            dy,
            th, tm, ts,
            sign,
            offset * 100,
            DateTime::timeZoneName.c_str()
    );

    message << dateBuffer << endl;

    message << "MIME-Version: 1.0" << endl;
    message << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"" << endl << endl;

    message << endl << "--" << boundary << endl;

    if (m_body.type() == MMT_PLAIN_TEXT_MESSAGE) {
        message << "Content-Type: text/plain; charset=ISO-8859-1" << endl;
        message << "Content-Transfer-Encoding: 7bit" << endl;
        message << "Content-Disposition: inline" << endl << endl;
        message << m_body.text() << endl << endl;
    } else {
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

    //message << endl << "--" << boundary << "--" << endl;

    CStrings sl(m_attachments, ";");
    for (unsigned i = 0; i < sl.size(); i++) {
        string attachment = sl[i];
        string attachmentAlias = attachment;
        const char* separator = "\\";
        if (attachment.find("/") != STRING_NPOS)
            separator = "/";
        CStrings attachmentParts(attachment, separator);
        uint32_t attachmentPartsCount = (uint32_t) attachmentParts.size();
        if (attachmentPartsCount > 1)
            attachmentAlias = attachmentParts[attachmentPartsCount - 1].c_str();
        if (attachment.length()) {
            message << endl << "--" << boundary << endl;
            mimeFile(attachment, attachmentAlias, message);
            //message << "--" << boundary << "--" << endl;
        }
    }

    message << endl << "--" << boundary << "--" << endl;

    buffer.reset(2048);
    buffer.append(message.str().c_str(), (uint32_t) message.str().length());
    buffer.saveToFile("/tmp/mimed.txt");
    //exit(0);
}
