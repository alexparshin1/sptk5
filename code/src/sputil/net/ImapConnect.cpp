/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ImapConnect.cpp - description                          ║
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

#include <sptk5/net/ImapConnect.h>

using namespace std;
using namespace sptk;

// Implementation is based on
// http://www.course.molina.com.br/RFC/Orig/rfc2060.txt

ImapConnect::ImapConnect()
{
    m_ident = 1;
}

ImapConnect::~ImapConnect()
{
    close();
}

#define RSP_BLOCK_SIZE 1024

bool ImapConnect::getResponse(const String& ident)
{
    char readBuffer[RSP_BLOCK_SIZE + 1];

    for (; ;) {
        size_t len = readLine(readBuffer, RSP_BLOCK_SIZE);
        String longLine = readBuffer;
        if (len == RSP_BLOCK_SIZE && readBuffer[RSP_BLOCK_SIZE] != '\n') {
            do {
                len = readLine(readBuffer, RSP_BLOCK_SIZE);
                longLine += readBuffer;
            } while (len == RSP_BLOCK_SIZE);
        }
        m_response.push_back(longLine);
        if (ident[0] == 0) return true;

        if (longLine[0] == '*')
            continue;
        if (longLine[0] == '+')
            return true;
        if (longLine.find(ident) == 0) {
            auto p = (uint32_t) ident.length();
            while (longLine[p] == ' ') p++;
            switch (longLine[p]) {
                case 'O': // OK
                    return true;
                case 'N': // NO
                    throw Exception(longLine.c_str() + 8);
                case 'B': // BAD
                    throw Exception(longLine.c_str() + 9);
                default:
                    break;
            }
        }
    }
    //return false;
}

const String ImapConnect::empty_quotes;

static String quotes(const String& st)
{
    return "\"" + st + "\"";
}

String ImapConnect::sendCommand(const String& cmd)
{
    String command(cmd);
    char id_str[10];
	int len = snprintf(id_str, sizeof(id_str), "a%03i ", m_ident++);
    String ident(id_str, (size_t) len);
    command = ident + cmd + "\n";
    if (!active())
        throw Exception("Socket isn't open");
    write(command.c_str(), (uint32_t) command.length());
    return ident;
}

void ImapConnect::command(const String& cmd, const String& arg1, const String& arg2)
{
    String command(cmd);
    if (!arg1.empty() || &arg1 == &empty_quotes)
        command += " " + quotes(arg1);
    if (!arg2.empty() || &arg2 == &empty_quotes)
        command += " " + quotes(arg2);
    m_response.clear();
    String ident = sendCommand(command);
    getResponse(ident);
}

void ImapConnect::cmd_login(const String& user, const String& password)
{
    close();
    open();
    m_response.clear();
    getResponse("");
    command("login " + user + " " + password);
}

// RFC 2060 test message :)

/*
CBuffer testMsg(
   "Date: Mon, 7 Feb 1994 21:52:25 -0800 (PST)\n\r"
   "From: Fred Foobar <foobar@Blurdybloop.COM\n\r"
   "Subject: afternoon meeting\n\r"
   "To: mooch@owatagu.siam.edu\n\r"
   "Message-Id: <B27397-0100000@Blurdybloop.COM>\n\r"
   "MIME-Version: 1.0\n\r"
   "Content-Type: TEXT/PLAIN; CHARSET=US-ASCII\n\r\n\r"
   "Hello Joe, do you think we can meet at 3:30 tomorrow?\n\r");
 */
void ImapConnect::cmd_append(const String& mail_box, const Buffer& message)
{
    String cmd = "APPEND \"" + mail_box + "\" (\\Seen) {" + int2string((uint32_t) message.bytes()) + "}";
    String ident = sendCommand(cmd);
    getResponse(ident);
    write(message.data(), message.bytes());
    write("\n", 1);
    getResponse(ident);
}

void ImapConnect::cmd_select(const String& mail_box, int32_t& total_msgs)
{
    command("select", mail_box);
    for (auto& st: m_response) {
        if (st[0] == '*') {
            size_t p = st.find("EXISTS");
            if (p != STRING_NPOS) {
                total_msgs = string2int(st.substr(2, p - 2));
                break;
            }
        }
    }
}

void ImapConnect::parseSearch(String& result)
{
    result = "";
    for (auto& st: m_response) {
        if (st.find("* SEARCH") == 0)
            result += st.substr(8, st.length());
    }
}

void ImapConnect::cmd_search_all(String& result)
{
    command("search all");
    parseSearch(result);
}

void ImapConnect::cmd_search_new(String& result)
{
    command("search unseen");
    parseSearch(result);
}

static const char *required_headers[] = {
    "Date",
    "From",
    "Subject",
    "To",
    "CC",
    "Content-Type",
    "Reply-To",
    "Return-Path",
    nullptr
};

static void parse_header(const String& header, String& header_name, String& header_value)
{
    if (header[0] == ' ')
        return;

    size_t p = header.find(' ');
    if (p == STRING_NPOS)
        return;
    if (header[p - 1] == ':') {
        header_name = lowerCase(header.substr(0, p - 1));
        header_value = header.substr(p + 1, header.length());
    }
}

static DateTime decodeDate(const String& dt)
{
    char temp[40];
    strncpy(temp, dt.c_str() + 5, sizeof(temp));
	temp[sizeof(temp) - 1] = 0;

    // 1. get the day of the month
    char *p1 = temp;
    char *p2 = strchr(p1, ' ');
    if (p2 == nullptr)
        return DateTime();
    *p2 = 0;
    int mday = string2int(p1);

    // 2. get the month
    p1 = p2 + 1;
    int month = 1;
    switch (*p1) {
        case 'A':
            if (*(p1 + 1) == 'p') {
                month = 4; // Apr
                break;
            }
            month = 8; // Aug
            break;
        case 'D':
            month = 12; // Dec
            break;
        case 'F':
            month = 2; // Feb
            break;
        case 'J':
            if (*(p1 + 1) == 'a') {
                month = 1; // Jan
                break;
            }
            if (*(p1 + 2) == 'n') {
                month = 6; // Jun
                break;
            }
            month = 7; // Jul
            break;
        case 'M':
            if (*(p1 + 2) == 'r') {
                month = 3; // Mar
                break;
            }
            month = 5; // May
            break;
        case 'N':
            month = 11; // Oct
            break;
        case 'O':
            month = 10; // Oct
            break;
        case 'S':
            month = 9; // Sep
            break;
        default:
            break;
    }
    // 2. get the year
    p1 += 4;
    p2 = p1 + 4;
    *p2 = 0;
    int year = string2int(p1);
    p1 = p2 + 1;
    p2 = strchr(p1, ' ');
    if (p2 != nullptr)
        *p2 = 0;
    DateTime time(p1);
    DateTime date((short) year, (short) month, (short) mday);
    return date + time.timePoint().time_since_epoch();
}

void ImapConnect::parseMessage(FieldList &results, bool headers_only)
{
    results.clear();
    unsigned i;
    for (i = 0; required_headers[i] != nullptr; i++) {
        String headerName = required_headers[i];
        Field *fld = new Field(lowerCase(headerName).c_str());
        switch (i) {
            case 0:
                fld->view.width = 16;
                break;
            default:
                fld->view.width = 32;
                break;
        }
        results.push_back(fld);
    }

    // parse headers
    i = 1;
    for (; i < m_response.size() - 1; i++) {
        String &st = m_response[i];
        if (st.empty())
            break;
        String header_name, header_value;
        parse_header(st, header_name, header_value);
        if (!header_name.empty()) {
            try {
                Field &field = results[header_name];
                if (header_name == "date")
                    field.setDate(decodeDate(header_value));
                else
                    field = header_value;
            } catch (...) {
            }
        }
    }

    for (i = 0; i < results.size(); i++) {
        Field &field = results[i];
        if (field.dataType() == VAR_NONE)
            field.setString("");
    }

    if (headers_only) return;

    String body;
    for (; i < m_response.size() - 1; i++)
        body += m_response[i] + "\n";

    Field &bodyField = results.push_back(new Field("body"));
    bodyField.setString(body);
}

void ImapConnect::cmd_fetch_headers(int32_t msg_id, FieldList &result)
{
    command("FETCH " + int2string(msg_id) + " (BODY[HEADER])");
    parseMessage(result, true);
}

void ImapConnect::cmd_fetch_message(int32_t msg_id, FieldList &result)
{
    command("FETCH " + int2string(msg_id) + " (BODY[])");
    parseMessage(result, false);
}

String ImapConnect::cmd_fetch_flags(int32_t msg_id)
{
    String result;
    command("FETCH " + int2string(msg_id) + " (FLAGS)");
    size_t count = m_response.size() - 1;
    //for (; i < count; i++) {
    if (count > 0) {
        size_t i = 0;
        String &st = m_response[i];
        const char *fpos = strstr(st.c_str(), "(\\");
        if (fpos == nullptr)
            return "";
        String flags(fpos + 1);
        size_t pos = flags.find("))");
        if (pos != STRING_NPOS)
            flags[pos] = 0;
        return flags;
    }
    return result;
}

void ImapConnect::cmd_store_flags(int32_t msg_id, const char *flags)
{
    command("STORE " + int2string(msg_id) + " FLAGS " + String(flags));
}

static String strip_framing_quotes(const String& st)
{
    if (st[0] == '\"')
        return st.substr(1, st.length() - 2);
    return st;
}

void ImapConnect::parseFolderList()
{
    Strings folder_names;
    String prefix = "* LIST ";
    for (auto& st: m_response) {
        if (st.find(prefix) == 0) {
            // passing the attribute(s)
            const char *p = strstr(st.c_str() + prefix.length(), ") ");
            if (p == nullptr) continue;
            // passing the reference
            p = strchr(p + 2, ' ');
            if (p == nullptr) continue;
            p++;
            // Ok, we found the path
            folder_names.push_back(strip_framing_quotes(p));
        }
    }
    m_response = folder_names;
}

void ImapConnect::cmd_list(const String& mail_box_mask, bool decode)
{
    command("list", empty_quotes, mail_box_mask);
    if (decode)
        parseFolderList();
}

