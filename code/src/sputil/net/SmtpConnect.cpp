/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SmtpConnect.cpp - description                          ║
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

#include <cstdio>
#include <sptk5/Base64.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/net/SmtpConnect.h>

using namespace std;
using namespace sptk;

SmtpConnect::SmtpConnect(Logger* log)
: m_log(log)
{
}

SmtpConnect::~SmtpConnect()
{
    close();
}

#define RSP_BLOCK_SIZE 1024

int SmtpConnect::getResponse(bool decode)
{
    char readBuffer[RSP_BLOCK_SIZE + 1];
    string longLine;
    bool readCompleted = false;
    int rc = 0;

    if (!readyToRead(chrono::seconds(30)))
        throw TimeoutException("SMTP server response timeout");

    while (!readCompleted) {
        size_t len = readLine(readBuffer, RSP_BLOCK_SIZE);
        longLine = readBuffer;
        if (m_log != nullptr)
            m_log->debug("[RECV] " + string(readBuffer));
        if (len == RSP_BLOCK_SIZE && readBuffer[RSP_BLOCK_SIZE] != '\n') {
            do {
                len = readLine(readBuffer, RSP_BLOCK_SIZE);
                longLine += readBuffer;
                if (m_log != nullptr)
                    m_log->debug("[RECV] " + string(readBuffer));
            }
            while (len == RSP_BLOCK_SIZE);
        }

        if (longLine[3] == ' ') {
            readCompleted = true;
            longLine[3] = 0;
            rc = string2int(longLine);
        }

        if (!longLine.empty()) {
            size_t lastCharPos = longLine.length() - 1;
            if (longLine[lastCharPos] == '\r')
                longLine.erase(lastCharPos);
        }

        const char * text = longLine.c_str() + 4;
        if (rc <= 432 && decode) {
            longLine = unmime(text);
            m_response.push_back(longLine);
        }
        else
            m_response.push_back(std::string(text));
    }
    return rc;
}

void SmtpConnect::sendCommand(string cmd, bool encode)
{
    if (!active())
        throw Exception("Socket isn't open");
    if (encode)
        cmd = mime(cmd);
    if (m_log != nullptr)
        m_log->debug("[SEND] " + string(cmd));
    cmd += "\r\n";
    write(cmd.c_str(), (uint32_t) cmd.length());
}

int SmtpConnect::command(const string& cmd, bool encodeCommand, bool decodeResponse)
{
    m_response.clear();
    if (!active())
        throw Exception("Socket isn't open");
    sendCommand(cmd, encodeCommand);
    return getResponse(decodeResponse);
}

string SmtpConnect::mime(const Buffer& buffer)
{
    Buffer result;
    Base64::encode(result, buffer);
    return result;
}

string SmtpConnect::mime(string s)
{
    string result;
    Buffer src;
    src.set(s.c_str(), (uint32_t) s.length());
    Base64::encode(result, src);
    return result;
}

string SmtpConnect::unmime(const string& s)
{
    Buffer dest;
    Base64::decode(dest, s);
    string result(dest.data(), dest.bytes());
    return result;
}

void SmtpConnect::cmd_auth(const string& user, const string& password)
{
    close();
    open(Host(), SOM_CONNECT, true, chrono::seconds(30));

    m_response.clear();
    getResponse();

    int rc = command("EHLO localhost");
    if (rc > 251)
        throw Exception(m_response.asString("\n"));

    Strings authInfo = m_response.grep("^AUTH ");
    if (authInfo.empty())
        return; // Authentication not advertised and not required

    RegularExpression matchAuth("^AUTH ");
    String authMethodsStr = matchAuth.s(authInfo[0], "");
    Strings authMethods(authMethodsStr, " ");

    string method = "LOGIN";
    if (authMethods.indexOf("LOGIN") < 0) {
        if (authMethods.indexOf("PLAIN") < 0)
            throw Exception("This SMTP module only supports LOGIN and PLAIN authentication.");
        method = "PLAIN";
    }

    if (!trim(user).empty()) {
        if (method == "LOGIN") {
            rc = command("AUTH LOGIN", false, true);
            if (rc > 432)
                throw Exception(m_response.asString("\n"));

            rc = command(user, true, true);
            if (rc > 432)
                throw Exception(m_response.asString("\n"));

            rc = command(password, true, false);
            if (rc > 432)
                throw Exception(m_response.asString("\n"));
            return;
        }

        if (method == "PLAIN") {
            Buffer userAndPassword;
            char nullChar = 0;
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(user.c_str(), user.size());
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(password.c_str(), password.size());

            string userAndPasswordMimed = mime(userAndPassword);
            rc = command("AUTH PLAIN " + userAndPasswordMimed, false, false);
            if (rc > 432)
                throw Exception(m_response.asString("\n"));
            return;
        }

        throw Exception("AUTH method " + method + " is not supported");
    }
}

void SmtpConnect::cmd_send()
{
    sendMessage();
}

void SmtpConnect::cmd_quit()
{
    command("QUIT");
    close();
}

String parseAddress(const String& fullAddress)
{
    size_t p1 = fullAddress.find('<');
    size_t p2 = fullAddress.find('>');
    if (p1 == STRING_NPOS || p2 == STRING_NPOS || p2 < p1)
        return fullAddress;
    return trim(fullAddress.substr(p1 + 1, p2 - p1 - 1));
}

void SmtpConnect::sendMessage()
{
    int rc = command("MAIL FROM:<" + parseAddress(m_from) + ">");
    if (rc > 251)
        throw Exception("Can't send message:\n" + m_response.asString("\n"));

    String rcpts = m_to + ";" + m_cc + ";" + m_bcc;
    rcpts = rcpts.replace("[, ]+", ";");
    Strings recepients(rcpts, ";");
    auto cnt = (uint32_t) recepients.size();
    for (uint32_t i = 0; i < cnt; i++) {
        String address = trim(recepients[i]);
        if (address[0] == 0) continue;
        rc = command("RCPT TO:<" + parseAddress(recepients[i]) + ">");
        if (rc > 251)
            throw Exception("Recepient " + recepients[i] + " is not accepted.\n" + m_response.asString("\n"));
    }

    mimeMessage(m_messageBuffer);
    rc = command("DATA");
    if (rc != 354)
        throw Exception("DATA command is not accepted.\n" + m_response.asString("\n"));

    sendCommand(m_messageBuffer.data());
    rc = command("\n.");
    if (rc > 251)
        throw Exception("Message body is not accepted.\n" + m_response.asString("\n"));
}
