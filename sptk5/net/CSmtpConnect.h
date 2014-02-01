/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSmtpConnect.h  -  description
                             -------------------
    begin                : Jun 25 2003
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CSMTPCONNECT_H__
#define __CSMTPCONNECT_H__

#include <sptk5/net/CTCPSocket.h>
#include <sptk5/CStrings.h>
#include <sptk5/net/CBaseMailConnect.h>

#include <string>

namespace sptk
{

/// @addtogroup utility Utility Classes
/// @{

/// @brief SMTP socket
///
/// Sends an e-mail message using SMTP protocol.
/// It uses CSocket class to establish the connection, and CBaseMailConnect
/// to make the complete RFC 822 message.
class SP_EXPORT CSmtpConnect: public CBaseMailConnect,
                              public CTCPSocket
{
    CStrings    m_response;

    /// Processes tag for strippedHtml.
    /// Extracts the text information for the tag
    /// @param tag std::string, the tag name
    /// @param params std::string, the tag parameters
    /// @returns the extracted text like URL
    static std::string processTag(std::string tag, std::string params);
protected:

    /// @brief Sends command using SMTP protocol
    ///
    /// The CRLF characters after the command are added automatically.
    /// @param cmd std::string, SMTP protocol command
    /// @param encode bool, encode the arguments to Base64 or not
    void sendCommand(std::string cmd, bool encode = false);

    /// @brief Retrieves the server response after the command into internal CStrings buffer
    ///
    /// The response can be read then with response() method.
    /// @param decode bool, decode the response from Base64 or not
    int getResponse(bool decode = false);

    /// Mime-encodes the buffer
    static std::string mime(const CBuffer& buffer);

    /// Mime-encodes the string
    static std::string mime(std::string s);

    /// Mime-decodes the string
    static std::string unmime(std::string s);

public:

    /// Default constructor
    CSmtpConnect();

    /// Destructor
    ~CSmtpConnect();

    /// Sends command using SMTP protocol and retrieve the server response.
    /// The response can be read then with response() method.
    /// The CRLF characters after the command are added automatically.
    /// @param cmd std::string, SMTP protocol command
    /// @param encodeCommand bool, encode the comand argument to Base64 or not
    /// @param decodeResponse bool, decode the response from Base64 or not
    int command(std::string cmd, bool encodeCommand = false, bool decodeResponse = false);

    /// @brief The response from the server - makes sence after calling any command
    CStrings& response()
    {
        return m_response;
    }

    /// @brief Logs in to the server host()
    /// @param user std::string, user name
    /// @param password std::string, user password
    /// @param method std::string, AUTH method: "login" or "plain"
    void cmd_auth(std::string user, std::string password, std::string method = "plain");

    /// @brief Sends the message
    ///
    /// Message is based on the information defined by the methods from
    /// CBaseMailConnect, and retrieves the server output. An alias for sendMessage().
    void cmd_send();

    /// Ends the SMTP session
    void cmd_quit();

    /// @brief Sends the message
    ///
    /// The message based on the information defined by the methods from
    /// CBaseMailConnect, and retrieves the server output.
    virtual void sendMessage();

    /// @brief Strips HTML tags off the message, prepare the alternative text for an HTML message
    /// @param html const std::string&, the HTML text
    /// @returns plain text with stripped HTML messages
    static std::string stripHtml(const std::string& html);
};
/// @}
}
#endif
