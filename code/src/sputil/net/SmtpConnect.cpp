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

constexpr int RSP_BLOCK_SIZE = 1024;
constexpr int minErrorCode = 433;

int SmtpConnect::getResponse(bool decode)
{
    array<char, RSP_BLOCK_SIZE + 1> readBuffer {};
    String longLine;
    bool readCompleted = false;
    int rc = 0;


    if (constexpr chrono::seconds readTimeout {30};
        !readyToRead(readTimeout))
    {
        throw TimeoutException("SMTP server response timeout");
    }

    while (!readCompleted)
    {
        size_t len = readLine(readBuffer.data(), RSP_BLOCK_SIZE);
        longLine = readBuffer.data();
        if (len == RSP_BLOCK_SIZE && readBuffer[RSP_BLOCK_SIZE] != '\n')
        {
            do
            {
                len = readLine(readBuffer.data(), RSP_BLOCK_SIZE);
                longLine += readBuffer.data();
            } while (len == RSP_BLOCK_SIZE);
        }

        if (longLine[3] == ' ')
        {
            readCompleted = true;
            longLine[3] = 0;
            rc = string2int(longLine);
        }

        if (!longLine.empty())
        {
            size_t lastCharPos = longLine.length() - 1;
            if (longLine[lastCharPos] == '\r')
            {
                longLine.erase(lastCharPos);
            }
        }

        const char* text = longLine.c_str() + 4;
        if (rc < minErrorCode && decode)
        {
            longLine = unmime(text);
            m_response.push_back(longLine);
        }
        else
        {
            m_response.push_back(text);
        }
    }
    return rc;
}

void SmtpConnect::sendCommand(String cmd, bool encode)
{
    if (!active())
    {
        throw Exception("Socket isn't open");
    }
    if (encode)
    {
        cmd = mime(cmd);
    }
    if (m_log != nullptr)
    {
        m_log->debug("[SEND] " + cmd);
    }
    cmd += "\r\n";
    write((const uint8_t*) cmd.c_str(), (uint32_t) cmd.length());
}

int SmtpConnect::command(const String& cmd, bool encodeCommand, bool decodeResponse)
{
    m_response.clear();
    if (!active())
    {
        throw Exception("Socket isn't open");
    }
    sendCommand(cmd, encodeCommand);
    return getResponse(decodeResponse);
}

String SmtpConnect::mime(const Buffer& buffer)
{
    Buffer result;
    Base64::encode(result, buffer);
    return (String) result;
}

String SmtpConnect::mime(const String& s)
{
    String result;
    Buffer src;
    src.set((const uint8_t*) s.c_str(), (uint32_t) s.length());
    Base64::encode(result, src);
    return result;
}

String SmtpConnect::unmime(const String& s)
{
    Buffer dest;
    Base64::decode(dest, s);
    return {dest.c_str(), dest.bytes()};
}

void SmtpConnect::cmd_auth(const String& user, const String& password)
{
    constexpr int minAuthErrorCode {252};
    constexpr chrono::seconds connectTimeout {30};

    close();
    open(Host(), OpenMode::CONNECT, true, connectTimeout);

    m_response.clear();
    getResponse();

    int rc = command("EHLO localhost");
    if (rc > minAuthErrorCode)
    {
        throw Exception(m_response.join("\n"));
    }

    Strings authInfo = m_response.grep("^AUTH ");
    if (authInfo.empty())
    {
        return;
    } // Authentication not advertised and not required

    RegularExpression matchAuth("^AUTH ");
    String authMethodsStr = matchAuth.s(authInfo[0], "");
    Strings authMethods(authMethodsStr, " ");

    String method = "LOGIN";
    if (authMethods.indexOf("LOGIN") < 0)
    {
        if (authMethods.indexOf("PLAIN") < 0)
            throw Exception("This SMTP module only supports LOGIN and PLAIN authentication.");
        method = "PLAIN";
    }

    if (!trim(user).empty())
    {
        if (method == "LOGIN")
        {
            rc = command("AUTH LOGIN", false, true);
            if (rc >= minAuthErrorCode)
                throw Exception(m_response.join("\n"));

            rc = command(user, true, true);
            if (rc >= minAuthErrorCode)
                throw Exception(m_response.join("\n"));

            rc = command(password, true, false);
            if (rc >= minAuthErrorCode)
                throw Exception(m_response.join("\n"));
            return;
        }

        if (method == "PLAIN")
        {
            Buffer userAndPassword;
            char nullChar = 0;
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(user.c_str(), user.size());
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(password.c_str(), password.size());

            String userAndPasswordMimed = mime(userAndPassword);
            rc = command("AUTH PLAIN " + userAndPasswordMimed, false, false);
            if (rc >= minAuthErrorCode)
                throw Exception(m_response.join("\n"));
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
    {
        return fullAddress;
    }
    return trim(fullAddress.substr(p1 + 1, p2 - p1 - 1));
}

void SmtpConnect::sendMessage()
{
    constexpr int minSendErrorCode {252};
    int rc = command("MAIL FROM:<" + parseAddress(from()) + ">");
    if (rc >= minSendErrorCode)
    {
        throw Exception("Can't send message:\n" + m_response.join("\n"));
    }

    String rcpts = to() + ";" + cc() + ";" + bcc();
    rcpts = rcpts.replace("[, ]+", ";");
    Strings recepients(rcpts, ";");
    auto cnt = (uint32_t) recepients.size();
    for (uint32_t i = 0; i < cnt; ++i)
    {
        if (trim(recepients[i]).empty())
        {
            continue;
        }
        rc = command("RCPT TO:<" + parseAddress(recepients[i]) + ">");
        if (rc >= minSendErrorCode)
        {
            throw Exception("Recepient " + recepients[i] + " is not accepted.\n" + m_response.join("\n"));
        }
    }

    constexpr int dataSuccessCode {354};
    Buffer message(messageBuffer());
    mimeMessage(message);
    rc = command("DATA");
    if (rc != dataSuccessCode)
    {
        throw Exception("DATA command is not accepted.\n" + m_response.join("\n"));
    }

    sendCommand(message.c_str());
    rc = command("\n.");
    if (rc >= minSendErrorCode)
    {
        throw Exception("Message body is not accepted.\n" + m_response.join("\n"));
    }
}
