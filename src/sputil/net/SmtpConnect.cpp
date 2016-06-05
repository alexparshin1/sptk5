/***************************************************************************
 *                          SIMPLY POWERFUL TOOLKIT (SPTK)
 *                          CSmtpConnect.h  -  description
 *                             -------------------
 *    begin                : Jun 25 2003
 *    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
 *    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU Library General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or (at
 *   your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; if not, write to the Free Software Foundation,
 *   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 *   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <stdio.h>
#include <sptk5/CBase64.h>
#include <sptk5/net/SmtpConnect.h>

using namespace std;
using namespace sptk;

SmtpConnect::SmtpConnect(Logger* log)
: m_log(log)
{
    m_port = 25;
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

    while (!readCompleted) {
        size_t len = readLine(readBuffer, RSP_BLOCK_SIZE);
        longLine = readBuffer;
        if (m_log)
            *m_log << "[RECV] " << readBuffer << endl;
        if (len == RSP_BLOCK_SIZE && readBuffer[RSP_BLOCK_SIZE] != '\n') {
            do {
                len = readLine(readBuffer, RSP_BLOCK_SIZE);
                longLine += readBuffer;
                if (m_log)
                    *m_log << "[RECV] " << readBuffer << endl;
            }
            while (len == RSP_BLOCK_SIZE);
        }

        if (longLine[3] == ' ') {
            readCompleted = true;
            longLine[3] = 0;
            rc = atoi(longLine.c_str());
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
        throw CException("Socket isn't open");
    if (encode)
        cmd = mime(cmd);
    if (m_log)
        *m_log << "[SEND] " << cmd << endl;
    cmd += "\r\n";
    write(cmd.c_str(), (uint32_t) cmd.length());
}

int SmtpConnect::command(string cmd, bool encodeCommand, bool decodeResponse)
{
    m_response.clear();
    if (!active())
        throw CException("Socket isn't open");
    sendCommand(cmd, encodeCommand);
    return getResponse(decodeResponse);
}

string SmtpConnect::mime(const CBuffer& buffer)
{
    CBuffer result;
    CBase64::encode(result, buffer);
    return result;
}

string SmtpConnect::mime(string s)
{
    string result;
    CBuffer src;
    src.set(s.c_str(), (uint32_t) s.length());
    CBase64::encode(result, src);
    return result;
}

string SmtpConnect::unmime(string s)
{
    CBuffer dest;
    CBase64::decode(dest, s);
    string result(dest.data(), dest.bytes());
    return result;
}

void SmtpConnect::cmd_auth(string user, string password, string method)
{
    close();
    open();

    m_response.clear();
    getResponse();

    int rc = command("EHLO localhost");
    if (rc > 251)
        throw CException(m_response.asString("\n"));

    method = trim(lowerCase(method));
    if (trim(user).length()) {
        if (method.empty())
            return;

        if (method == "login") {
            rc = command("auth login", false, true);
            if (rc > 432)
                throw CException(m_response.asString("\n"));

            rc = command(user, true, true);
            if (rc > 432)
                throw CException(m_response.asString("\n"));

            rc = command(password, true, false);
            if (rc > 432)
                throw CException(m_response.asString("\n"));
            return;
        }

        if (method == "plain") {
            CBuffer userAndPassword;
            char nullChar = 0;
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(user.c_str(), user.size());
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(password.c_str(), password.size());

            string userAndPasswordMimed = mime(userAndPassword);
            rc = command("AUTH PLAIN " + userAndPasswordMimed, false, false);
            if (rc > 432)
                throw CException(m_response.asString("\n"));
            return;
        }

        throw CException("AUTH method " + method + " is not supported");
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

string parseAddress(string fullAddress)
{
    size_t p1 = fullAddress.find("<");
    size_t p2 = fullAddress.find(">");
    if (p1 == STRING_NPOS || p2 == STRING_NPOS || p2 < p1)
        return fullAddress;
    return trim(fullAddress.substr(p1 + 1, p2 - p1 - 1));
}

void SmtpConnect::sendMessage()
{
    int rc = command("MAIL FROM:<" + parseAddress(m_from) + ">");
    if (rc > 251)
        throw CException("Can't send message:\n" + m_response.asString("\n"));

    string rcpts = m_to + ";" + m_cc + ";" + m_bcc;
    rcpts = replaceAll(rcpts, ",", ";");
    rcpts = replaceAll(rcpts, " ", ";");
    CStrings recepients(rcpts, ";");
    uint32_t cnt = (uint32_t) recepients.size();
    for (uint32_t i = 0; i < cnt; i++) {
        string address = trim(recepients[i]);
        if (address[0] == 0) continue;
        rc = command("RCPT TO:<" + parseAddress(recepients[i]) + ">");
        if (rc > 251)
            throw CException("Recepient " + recepients[i] + " is not accepted.\n" + m_response.asString("\n"));
    }

    mimeMessage(m_messageBuffer);
    rc = command("DATA");
    if (rc != 354)
        throw CException("DATA command is not accepted.\n" + m_response.asString("\n"));

    sendCommand(m_messageBuffer.data());
    rc = command("\n.");
    if (rc > 251)
        throw CException("Message body is not accepted.\n" + m_response.asString("\n"));
}
