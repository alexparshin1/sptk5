/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ImapConnect.h - description                            ║
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

#ifndef __IMAPCONNECT_H__
#define __IMAPCONNECT_H__

#include <sptk5/net/TCPSocket.h>
#include <sptk5/Strings.h>
#include <sptk5/FieldList.h>

#include <string>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief IMAP socket
 *
 * Class CImapConnect is used to communicate with IMAP 4 servers.
 * It implements the most popular commands of IMAP protocol to build
 * a simple IMAP client.
 */
class ImapConnect: public TCPSocket
{
    /**
     * Internal response buffer
     */
    sptk::Strings               m_response;

    /**
     * Message id
     */
    int32_t                     m_ident;

    /**
     * Empty quotes string
     */
    static const String    empty_quotes;

protected:

    /**
     * @brief Sends a command to the server, but doesn't retrieve the server response
     *
     * The new line characters (CRLF) are added to the end of every command.
     * @param cmd std::string, the complete text of IMAP4 command
     * @returns the unique command identifier
     */
    String sendCommand(const String& cmd);

    /**
     * @brief Gets a response from the server for a previously sent command, identified by the ident
     * @param ident std::string, the command identifier returned by prior sendCommand().
     */
    bool getResponse(const String& ident);

    /**
     * @brief Parses the result of SEARCH command in response. Returns results in result parameter
     * @param result std::string, returns the search results.
     */
    void parseSearch(String& result);

    /**
     * @brief Parses server response as a message data (after the appropriate command) to the set of fields
     * @param result CFieldList, the set of fields with the message information.
     * @param headersOnly bool, true if we don't want to retrieve message body.
     */
    void parseMessage(FieldList& result, bool headersOnly);

    /**
     * @brief Parses server response as a folder list (after the appropriate command), and converts the response to it
     *
     * As a result, the response contains the plain list of folders.
     */
    void parseFolderList();
public:

    /**
     * Default constructor
     */
    ImapConnect();

    /**
     * Destructor, closes the connection if it's open.
     */
    ~ImapConnect();

    /**
     * Sends a command with the arguments. Arguments (if any) are automatically
     * enquoted with double-quotes. The command is also appended with the new line characters (CRLF).
     * @param cmd std::string, IMAP4 command
     * @param arg1 std::string, optional command argument1
     * @param arg2 std::string, optional command argument2
     */
    void command(const String& cmd, const String& arg1 = "", const String& arg2 = "");

    /**
     * Returns reference to a last command response.
     */
    const sptk::Strings& response() const
    {
        return m_response;
    }

    /**
     * IMAPv4 commands - any state
     *
     * Retrieves server's capabilities in response().
     */
    void cmd_capability()
    {
        command("capability");
    }

    /**
     * Sends NOOP command.
     */
    void cmd_noop()
    {
        command("noop");
    }

    /**
     * Logs out from the current session.
     */
    void cmd_logout()
    {
        command("LOGOUT");
    }

    // IMAPv4 commands - not logged in

    /**
     * Logs in the server. The server name or address should be defined
     * with the call of host() method.
     * @param user std::string&, user name on the server
     * @param password std::string&, user password on the server
     */
    void cmd_login(const String& user, const String& password);

    // IMAPv4 commands - logged in, mailbox-operations

    /**
     * Selects the mail box for future operations.
     * @param mail_box const std::string&, the name of the mail box
     * @param total_msgs int32_t&, returns the total messages in the mail box
     */
    void cmd_select(const String& mail_box, int32_t& total_msgs);

    /**
     * Retrieves the mail box information into response().
     * @param mail_box std::string, the name of the mail box
     */
    void cmd_examine(const String& mail_box)
    {
        command("examine", mail_box);
    }

    /**
     * Subscribes the mail box to the user.
     * @param mail_box std::string, the name of the mail box
     */
    void cmd_subscribe(const String& mail_box)
    {
        command("subscribe", mail_box);
    }

    /**
     * Unsubscribes the mail box from the user.
     * @param mail_box std::string, the name of the mail box
     */
    void cmd_unsubscribe(const String& mail_box)
    {
        command("unsubscribe", mail_box);
    }

    /**
     * Creates the new mail box.
     * @param mail_box std::string, the name of the mail box
     */
    void cmd_create(const String& mail_box)
    {
        command("create", mail_box);
    }

    /**
     * Deletes the new mail box.
     * @param mail_box std::string, the name of the mail box
     */
    void cmd_delete(const String& mail_box)
    {
        command("delete", mail_box);
    }

    /**
     * Renames the new mail box.
     * @param mail_box std::string, the name of the mail box
     * @param new_name std::string, the new name of the mail box
     */
    void cmd_rename(const String& mail_box, const String& new_name)
    {
        command("rename ", mail_box, new_name);
    }

    /**
     * Retrieves the list of mail boxes.
     * @param mail_box_mask std::string, the mask for the mail box names
     * @param decode bool, true if you want to convert the response into plain folder list.
     */
    void cmd_list(const String& mail_box_mask, bool decode = false);

    /**
     * Appends the message to the mail box.
     * @param mail_box std::string, the name of the mail box
     * @param message CBuffer, the RFC-2060 defined message
     */
    void cmd_append(const String& mail_box, const Buffer& message);

    // IMAPv4 commands - logged in, selected mailbox-operations

    /**
     * Closes the connection with the server.
     */
    void cmd_close()
    {
        command("close");
    }

    /**
     * Expanges the deleted messages in the current mail box.
     */
    void cmd_expunge()
    {
        command("expunge");
    }

    /**
     * Reatrieves all the messages list in the current mail box.
     */
    void cmd_search_all(String& result);

    /**
     * Reatrieves the new messages list in the current mail box.
     */
    void cmd_search_new(String& result);

    /**
     * Reatrieves the headers for the message.
     * @param msg_id int32_t, the message identifier
     * @param result CFieldList, the message headers information
     */
    void cmd_fetch_headers(int32_t msg_id, FieldList& result);

    /**
     * Reatrieves the message information.
     * @param msg_id int, the message identifier
     * @param result CFieldList, the complete message information
     */
    void cmd_fetch_message(int32_t msg_id, FieldList& result);

    /**
     * Gets message flags
     * @param msg_id int, the message identifier
     * @returns std::string, the message flags
     */
    String cmd_fetch_flags(int32_t msg_id);

    /**
     * Sets message flags
     * @param msg_id int, the message identifier
     * @param flags const char *, the message flags
     */
    void cmd_store_flags(int32_t msg_id, const char *flags);
};

/**
 * @}
 */
}
#endif
