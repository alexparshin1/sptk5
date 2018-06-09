/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SmtpConnect.h - description                            ║
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

#ifndef __SMTPCONNECT_H__
#define __SMTPCONNECT_H__

#include <sptk5/net/TCPSocket.h>
#include <sptk5/Strings.h>
#include <sptk5/net/BaseMailConnect.h>
#include <sptk5/Logger.h>

#include <string>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief SMTP socket
 *
 * Sends an e-mail message using SMTP protocol.
 * It uses CSocket class to establish the connection, and CBaseMailConnect
 * to make the complete RFC 822 message.
 */
class SP_EXPORT SmtpConnect: public BaseMailConnect, public TCPSocket
{
    Logger*    m_log;
    Strings    m_response;

    /**
     * @brief Processes tag for strippedHtml.
     *
     * Extracts the text information for the tag
     * @param tag               The tag name
     * @param params            The tag parameters
     * @returns the extracted text like URL
     */
    static std::string processTag(std::string tag, std::string params);
protected:

    /**
     * @brief Sends command using SMTP protocol
     *
     * The CRLF characters after the command are added automatically.
     * @param cmd               SMTP protocol command
     * @param encode            Encode the arguments to Base64 or not
     */
    void sendCommand(std::string cmd, bool encode = false);

    /**
     * @brief Retrieves the server response after the command into internal Strings buffer
     *
     * The response can be read then with response() method.
     * @param decode            Decode the response from Base64 or not
     */
    int getResponse(bool decode = false);

    /**
     * @brief Mime-encodes the buffer
     * @param buffer            Source data
     * @return MIME-encoded data
     */
    static std::string mime(const Buffer& buffer);

    /**
     * @brief Mime-encodes the string
     * @param s                 Source data
     * @return MIME-encoded data
     */
    static std::string mime(std::string s);

    /**
     * @brief Mime-decodes the string
     * @param s                 Source data
     * @return Decoded data
     */
    static std::string unmime(const std::string& s);

public:

    /**
     * @brief Default constructor
     * @param log               Optional log object
     */
    SmtpConnect(Logger* log=NULL);

    /**
     * Destructor
     */
    ~SmtpConnect();

    /**
     * Sends command using SMTP protocol and retrieve the server response.
     * The response can be read then with response() method.
     * The CRLF characters after the command are added automatically.
     * @param cmd               SMTP protocol command
     * @param encodeCommand     Encode the comand argument to Base64 or not
     * @param decodeResponse    Decode the response from Base64 or not
     */
    int command(const std::string& cmd, bool encodeCommand = false, bool decodeResponse = false);

    /**
     * @brief The response from the server - makes sence after calling any command
     */
    Strings& response()
    {
        return m_response;
    }

    /**
     * @brief Logs in to the server host()
     * @param user              User name
     * @param password          User password
     */
    void cmd_auth(const std::string& user, const std::string& password);

    /**
     * @brief Sends the message
     *
     * Message is based on the information defined by the methods from
     * CBaseMailConnect, and retrieves the server output. An alias for sendMessage().
     */
    void cmd_send();

    /**
     * Ends the SMTP session
     */
    void cmd_quit();

    /**
     * @brief Sends the message
     *
     * The message based on the information defined by the methods from
     * CBaseMailConnect, and retrieves the server output.
     */
    virtual void sendMessage();

    /**
     * @brief Strips HTML tags off the message, prepare the alternative text for an HTML message
     * @param html              The HTML text
     * @returns plain text with stripped HTML messages
     */
    static std::string stripHtml(const std::string& html);
};
/**
 * @}
 */
}
#endif
