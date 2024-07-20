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

#include <sptk5/cutils>
#include <sptk5/net/ImapConnect.h>
#include <sptk5/net/SocketReader.h>

using namespace std;
using namespace sptk;

// Implementation is based on
// http://www.course.molina.com.br/RFC/Orig/rfc2060.txt

static constexpr int RSP_BLOCK_SIZE = 1024;

void ImapConnect::getResponse(const String& ident)
{
    Buffer readBuffer(RSP_BLOCK_SIZE);

    SocketReader socketReader(*this);

    for (;;)
    {
        socketReader.readLine(readBuffer);
        String longLine = readBuffer.c_str();
        m_response.push_back(longLine);
        if (ident[0] == 0)
        {
            return;
        }

        if (longLine[0] == '*')
        {
            continue;
        }
        if (longLine[0] == '+')
        {
            return;
        }
        if (longLine.find(ident) == 0)
        {
            auto p = (uint32_t) ident.length();
            while (longLine[p] == ' ')
            {
                ++p;
            }
            switch (longLine[p])
            {
                case 'O': // OK
                    return;
                case 'N': // NO
                    throw Exception(longLine.c_str() + 8);
                case 'B': // BAD
                    throw Exception(longLine.c_str() + 9);
                default:
                    break;
            }
        }
    }
}

const String ImapConnect::empty_quotes;

namespace {
String quotes(const String& st)
{
    return "\"" + st + "\"";
}
} // namespace

String ImapConnect::sendCommand(const String& cmd)
{
    String command(cmd);
    array<char, 10> id_str {};
    const int len = snprintf(id_str.data(), sizeof(id_str), "a%03i ", ++m_ident);
    String ident(id_str.data(), (size_t) len);
    command = ident + cmd + "\n";
    if (!active())
    {
        throw Exception("Socket isn't open");
    }
    write(bit_cast<const uint8_t*>(command.c_str()), (uint32_t) command.length());
    return ident;
}

void ImapConnect::command(const String& cmd, const String& arg1, const String& arg2)
{
    String command(cmd);
    if (!arg1.empty() || &arg1 == &empty_quotes)
    {
        command += " " + quotes(arg1);
    }

    if (!arg2.empty() || &arg2 == &empty_quotes)
    {
        command += " " + quotes(arg2);
    }

    m_response.clear();

    const String ident = sendCommand(command);
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

// RFC 2060 describes message

void ImapConnect::cmd_append(const String& mail_box, const Buffer& message)
{
    const String cmd = "APPEND \"" + mail_box + "\" (\\Seen) {" + int2string((uint32_t) message.bytes()) + "}";
    const String ident = sendCommand(cmd);
    getResponse(ident);
    write(message.data(), message.bytes());
    write((const uint8_t*) "\n", 1);
    getResponse(ident);
}

void ImapConnect::cmd_select(const String& mail_box, int32_t& total_msgs)
{
    command("select", mail_box);
    for (auto& st: m_response)
    {
        if (st[0] == '*')
        {
            const size_t p = st.find("EXISTS");
            if (p != STRING_NPOS)
            {
                total_msgs = string2int(st.substr(2, p - 2));
                break;
            }
        }
    }
}

void ImapConnect::parseSearch(String& result) const
{
    result = "";
    for (const auto& st: m_response)
    {
        if (st.find("* SEARCH") == 0)
        {
            result += st.substr(8, st.length());
        }
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

static const Strings required_headers {
    "Date",
    "From",
    "Subject",
    "To",
    "CC",
    "Content-Type",
    "Reply-To",
    "Return-Path"};

namespace {
void parse_header(const String& header, String& header_name, String& header_value)
{
    if (header[0] == ' ')
    {
        return;
    }

    const size_t position = header.find(' ');
    if (position == STRING_NPOS)
    {
        return;
    }
    if (header[position - 1] == ':')
    {
        header_name = lowerCase(header.substr(0, position - 1));
        header_value = header.substr(position + 1, header.length());
    }
}

DateTime decodeDate(const String& dt)
{
    array<char, 40> temp {};
    snprintf(temp.data(), sizeof(temp), "%s", dt.c_str() + 5);

    // 1. get the day of the month
    char* p1 = temp.data();
    char* p2 = strchr(p1, ' ');
    if (p2 == nullptr)
    {
        return DateTime();
    }
    *p2 = 0;

    const int mday = string2int(p1);

    // 2. get the month
    p1 = p2 + 1;
    int month = 1;
    switch (*p1)
    {
        case 'A':
            if (*(p1 + 1) == 'p')
            {
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
            if (*(p1 + 1) == 'a')
            {
                month = 1; // Jan
                break;
            }
            if (*(p1 + 2) == 'n')
            {
                month = 6; // Jun
                break;
            }
            month = 7; // Jul
            break;
        case 'M':
            if (*(p1 + 2) == 'r')
            {
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
    const int year = string2int(p1);
    p1 = p2 + 1;
    p2 = strchr(p1, ' ');
    if (p2 != nullptr)
    {
        *p2 = 0;
    }
    const DateTime time(p1);
    const DateTime date((short) year, (short) month, (short) mday);
    return date + time.timePoint().time_since_epoch();
}
} // namespace

void ImapConnect::parseMessage(FieldList& results, bool headers_only)
{
    results.clear();
    bool first = true;
    for (const auto& headerName: required_headers)
    {
        auto fld = make_shared<Field>(lowerCase(headerName).c_str());
        if (first)
        {
            fld->view().width = 16;
            first = false;
        }
        else
        {
            fld->view().width = 32;
        }
        results.push_back(fld);
    }

    // parse headers
    size_t i = 1;
    for (; i < m_response.size() - 1; ++i)
    {
        const String& st = m_response[i];
        if (st.empty())
        {
            break;
        }
        String header_name;
        String header_value;
        parse_header(st, header_name, header_value);
        if (!header_name.empty())
        {
            try
            {
                Field& field = results[header_name];
                if (header_name == "date")
                {
                    field.setDateTime(decodeDate(header_value), true);
                }
                else
                {
                    field = header_value;
                }
            }
            catch (const Exception& e)
            {
                CERR(e.what());
            }
        }
    }

    for (i = 0; i < results.size(); ++i)
    {
        Field& field = results[int(i)];
        if (field.dataType() == VariantDataType::VAR_NONE)
        {
            field.setString("");
        }
    }

    if (headers_only)
    {
        return;
    }

    String body;
    for (; i < m_response.size() - 1; ++i)
    {
        body += m_response[i] + "\n";
    }

    Field& bodyField = results.push_back(make_shared<Field>("body"));
    bodyField.setString(body);
}

void ImapConnect::cmd_fetch_headers(int32_t msg_id, FieldList& result)
{
    command("FETCH " + int2string(msg_id) + " (BODY[HEADER])");
    parseMessage(result, true);
}

void ImapConnect::cmd_fetch_message(int32_t msg_id, FieldList& result)
{
    command("FETCH " + int2string(msg_id) + " (BODY[])");
    parseMessage(result, false);
}

String ImapConnect::cmd_fetch_flags(int32_t msg_id)
{
    String result;
    command("FETCH " + int2string(msg_id) + " (FLAGS)");
    if (const size_t count = m_response.size() - 1;
        count > 0)
    {
        const String& st = m_response[0];
        const char* fpos = strstr(st.c_str(), "(\\");

        if (fpos == nullptr)
        {
            return "";
        }

        String flags(fpos + 1);
        if (const size_t pos = flags.find("))");
            pos != STRING_NPOS)
        {
            flags[pos] = 0;
        }

        return flags;
    }
    return result;
}

void ImapConnect::cmd_store_flags(int32_t msg_id, const char* flags)
{
    command("STORE " + int2string(msg_id) + " FLAGS " + String(flags));
}

namespace {
String strip_framing_quotes(const String& st)
{
    if (st[0] == '\"')
    {
        return st.substr(1, st.length() - 2);
    }
    return st;
}
} // namespace

void ImapConnect::parseFolderList()
{
    const String prefix = "* LIST ";
    Strings folder_names;
    for (const auto& st: m_response)
    {
        if (st.find(prefix) == 0)
        {
            // passing the attribute(s)
            const char* p = strstr(st.c_str() + prefix.length(), ") ");
            if (p == nullptr)
            {
                continue;
            }
            // passing the reference
            p = strchr(p + 2, ' ');
            if (p == nullptr)
            {
                continue;
            }
            ++p;
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
    {
        parseFolderList();
    }
}
