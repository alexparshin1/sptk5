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

#include <sptk5/Base64.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/net/SmtpConnect.h>
#include <sptk5/net/SocketReader.h>

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
    Buffer readBuffer(RSP_BLOCK_SIZE);
    String longLine;
    bool   readCompleted = false;
    int    result = 0;

    SocketReader socketReader(*this);

    if (constexpr chrono::seconds readTimeout {30};
        !readyToRead(readTimeout))
    {
        throw TimeoutException("SMTP server response timeout");
    }

    while (!readCompleted)
    {
        socketReader.readLine(readBuffer);
        longLine = readBuffer.c_str();

        if (longLine[3] == ' ')
        {
            readCompleted = true;
            longLine[3] = 0;
            result = string2int(longLine);
        }

        if (!longLine.empty())
        {
            const size_t lastCharPos = longLine.length() - 1;
            if (longLine[lastCharPos] == '\r')
            {
                longLine.erase(lastCharPos);
            }
        }

        const char* text = longLine.c_str() + 4;
        if (result < minErrorCode && decode)
        {
            longLine = unmime(text);
            m_response.push_back(longLine);
        }
        else
        {
            m_response.push_back(text);
        }
    }
    return result;
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
    write(bit_cast<const uint8_t*>(cmd.c_str()), static_cast<uint32_t>(cmd.length()));
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
    return static_cast<String>(result);
}

String SmtpConnect::mime(const String& str)
{
    String result;
    Buffer src;
    src.set(bit_cast<const uint8_t*>(str.c_str()), static_cast<uint32_t>(str.length()));
    Base64::encode(result, src);
    return result;
}

String SmtpConnect::unmime(const String& str)
{
    Buffer dest;
    Base64::decode(dest, str);
    return {dest.c_str(), dest.bytes()};
}

void SmtpConnect::cmd_auth(const String& user, const String& password)
{
    constexpr int             minAuthErrorCode {252};
    constexpr chrono::seconds connectTimeout {30};

    close();
    open(Host(), OpenMode::CONNECT, true, connectTimeout);

    m_response.clear();
    getResponse();

    int result = command("EHLO localhost");
    if (result > minAuthErrorCode)
    {
        throw Exception(m_response.join("\n"));
    }

    Strings authInfo = m_response.grep("^AUTH ");
    if (authInfo.empty())
    {
        return;
    } // Authentication not advertised and not required

    const RegularExpression matchAuth("^AUTH ");
    const String            authMethodsStr = matchAuth.s(authInfo[0], "");
    const Strings           authMethods(authMethodsStr, " ");

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
            result = command("AUTH LOGIN", false, true);
            if (result >= minAuthErrorCode)
                throw Exception(m_response.join("\n"));

            result = command(user, true, true);
            if (result >= minAuthErrorCode)
                throw Exception(m_response.join("\n"));

            result = command(password, true, false);
            if (result >= minAuthErrorCode)
                throw Exception(m_response.join("\n"));
            return;
        }

        if (method == "PLAIN")
        {
            Buffer         userAndPassword;
            constexpr char nullChar = 0;
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(user.c_str(), user.size());
            userAndPassword.append(&nullChar, 1);
            userAndPassword.append(password.c_str(), password.size());

            const String userAndPasswordMimed = mime(userAndPassword);
            result = command("AUTH PLAIN " + userAndPasswordMimed, false, false);
            if (result >= minAuthErrorCode)
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
    const size_t addressStart = fullAddress.find('<');
    const size_t addressEnd = fullAddress.find('>');
    if (addressStart == STRING_NPOS || addressEnd == STRING_NPOS || addressEnd < addressStart)
    {
        return fullAddress;
    }
    return trim(fullAddress.substr(addressStart + 1, addressEnd - addressStart - 1));
}

void SmtpConnect::sendMessage()
{
    constexpr int minSendErrorCode {252};
    int           result = command("MAIL FROM:<" + parseAddress(from()) + ">");
    if (result >= minSendErrorCode)
    {
        throw Exception("Can't send message:\n" + m_response.join("\n"));
    }

    String rcpts = to() + ";" + cc() + ";" + bcc();
    rcpts = rcpts.replace("[, ]+", ";");
    Strings    recipients(rcpts, ";");
    const auto cnt = static_cast<uint32_t>(recipients.size());
    for (uint32_t i = 0; i < cnt; ++i)
    {
        if (trim(recipients[i]).empty())
        {
            continue;
        }
        result = command("RCPT TO:<" + parseAddress(recipients[i]) + ">");
        if (result >= minSendErrorCode)
        {
            throw Exception("Recipient " + recipients[i] + " is not accepted.\n" + m_response.join("\n"));
        }
    }

    constexpr int dataSuccessCode {354};
    Buffer        message(messageBuffer());
    mimeMessage(message);
    result = command("DATA");
    if (result != dataSuccessCode)
    {
        throw Exception("DATA command is not accepted.\n" + m_response.join("\n"));
    }

    sendCommand(message.c_str());
    result = command("\n.");
    if (result >= minSendErrorCode)
    {
        throw Exception("Message body is not accepted.\n" + m_response.join("\n"));
    }
}
