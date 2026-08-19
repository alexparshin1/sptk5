/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include "SocketReader.h"


#include <sptk5/FieldList.h>
#include <sptk5/Strings.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief IMAP socket.
 *
 * Class CImapConnect is used to communicate with IMAP 4 servers.
 * It implements the most popular commands of IMAP protocol to build
 * a simple IMAP client.
 */
class SP_EXPORT ImapConnect
{
public:
    /**
     * @brief Default constructor.
     * @remarks If the external connection is not provided, use the host() method to define a new one.
     * @param socket            External connection.
     */
    explicit ImapConnect(const std::shared_ptr<TCPSocket>& socket = {});

    /**
     * @brief Sends a command with the arguments. Arguments (if any) are automatically enquoted with double-quotes.
     *
     * The command is also appended with the new line characters (CRLF).
     * @param cmd IMAP4 command.
     * @param arg1 optional command argument1.
     * @param arg2 optional command argument2.
     */
    void command(const String& cmd, const String& arg1 = "", const String& arg2 = "");

    /**
     * @brief Close socket connection.
     */
    void close() const;

    /**
     * @brief Returns reference to the last command response.
     */
    [[nodiscard]] const Strings& response() const
    {
        return m_response;
    }

    /**
     * @brief IMAPv4 commands - any state.
     *
     * Retrieves server's capabilities in response().
     */
    void cmd_capability()
    {
        command("capability");
    }

    /**
     * @brief Sends NOOP command.
     */
    void cmd_noop()
    {
        command("noop");
    }

    /**
     * @brief Logs out from the current session.
     */
    void cmd_logout()
    {
        try
        {
            command("LOGOUT");
        }
        catch (const std::exception& e)
        {
            // ignore any exceptions on logout
            (void) e;
        }
    }

    // IMAPv4 commands - not logged in

    /**
     * @brief Logs in the server. The server name or address should be defined with the call of host() method.
     * @remark Always re-connects to the server.
     * @param user The username on the server.
     * @param password The user password on the server.
     */
    void cmd_login(const String& user, const String& password);

    // IMAPv4 commands - logged in, mailbox-operations

    /**
     * @brief Selects the mailbox for future operations.
     * @param mail_box The name of the mailbox.
     * @param total_msgs Returns the total messages in the mailbox.
     */
    void cmd_select(const String& mail_box, int32_t& total_msgs);

    /**
     * @brief Retrieves the mailbox information into response().
     * @param mail_box The name of the mailbox.
     */
    void cmd_examine(const String& mail_box)
    {
        command("examine", mail_box);
    }

    /**
     * @brief Subscribes the mailbox to the user.
     * @param mail_box The name of the mailbox.
     */
    void cmd_subscribe(const String& mail_box)
    {
        command("subscribe", mail_box);
    }

    /**
     * @brief Unsubscribes the mailbox from the user.
     * @param mail_box the name of the mailbox.
     */
    void cmd_unsubscribe(const String& mail_box)
    {
        command("unsubscribe", mail_box);
    }

    /**
     * @brief Creates the new mailbox.
     * @param mail_box The name of the mailbox.
     */
    void cmd_create(const String& mail_box)
    {
        command("create", mail_box);
    }

    /**
     * @brief Deletes the new mailbox.
     * @param mail_box The name of the mailbox.
     */
    void cmd_delete(const String& mail_box)
    {
        command("delete", mail_box);
    }

    /**
     * @brief Renames the new mailbox.
     * @param mail_box The name of the mailbox.
     * @param new_name The new name of the mailbox.
     */
    void cmd_rename(const String& mail_box, const String& new_name)
    {
        command("rename ", mail_box, new_name);
    }

    /**
     * @brief Retrieves the list of mailboxes.
     * @param mail_box_mask The mask for the mailbox names.
     * @param decode True if you want to convert the response into the plain folder list.
     */
    void cmd_list(const String& mail_box_mask, bool decode = false);

    /**
     * @brief Appends the message to the mailbox.
     * @param mail_box The name of the mailbox.
     * @param message The RFC-2060 defined message.
     */
    void cmd_append(const String& mail_box, const Buffer& message);

    // IMAPv4 commands - logged in, selected mailbox-operations

    /**
     * @brief Closes the connection with the server.
     */
    void cmd_close()
    {
        command("close");
    }

    /**
     * @brief Expanges the deleted messages in the current mailbox.
     */
    void cmd_expunge()
    {
        command("expunge");
    }

    /**
     * @brief Retrieves all the messages list in the current mailbox.
     */
    void cmd_search_all(String& result);

    /**
     * @brief Retrieves the new messages list in the current mailbox.
     */
    void cmd_search_new(String& result);

    /**
     * @brief Retrieves the headers for the message.
     * @param msg_id int32_t, the message identifier.
     * @param result CFieldList, the message headers information.
     */
    void cmd_fetch_headers(int32_t msg_id, FieldList& result);

    /**
     * @brief Retrieves the message information.
     * @param msg_id The message identifier.
     * @param result The complete message information.
     */
    void cmd_fetch_message(int32_t msg_id, FieldList& result);

    /**
     * @brief Gets message flags.
     * @param msg_id The message identifier.
     * @returns The message flags.
     */
    String cmd_fetch_flags(int32_t msg_id);

    /**
     * @brief Sets message flags.
     * @param msg_id int, the message identifier.
     * @param flags const char *, the message flags.
     */
    void cmd_store_flags(int32_t msg_id, const char* flags);

    /**
     * @brief Get IMAP host.
     */
    [[nodiscard]] Host host() const;

    /**
     * @brief Set IMAP host.
     */
    void host(const Host& host) const;


protected:
    /**
     * @brief Sends a command to the server but doesn't retrieve the server response.
     *
     * The new line characters (CRLF) are added to the end of every command.
     * @param cmd The complete text of the IMAP4 command.
     * @returns The unique command identifier.
     */
    String sendCommand(const String& cmd);

    /**
     * @brief Gets a response from the server for a previously sent command, identified by the ident.
     * @param ident the command identifier returned by prior sendCommand().
     * @param timeout Read timeout in milliseconds. Default is 30 seconds.
     */
    void getResponse(const String& ident, std::chrono::milliseconds timeout = std::chrono::seconds(30));

    /**
     * @brief Parses the result of SEARCH command in response. Returns results in result parameter.
     * @param result returns the search results.
     */
    void parseSearch(String& result) const;

    /**
     * @brief Parses server response as message data (after the appropriate command) to the set of fields.
     * @param results The set of fields with the message information.
     * @param headersOnly bool, true if we don't want to retrieve the message body.
     */
    void parseMessage(FieldList& results, bool headersOnly);

    /**
     * @brief Parses server response as a folder list (after the appropriate command) and converts the response to it.
     * As a result, the response contains the plain list of folders.
     */
    void parseFolderList();

private:
    Strings                       m_response;   ///< Internal response buffer.
    int32_t                       m_ident {1};  ///< Message id.
    std::shared_ptr<TCPSocket>    m_socket;     ///< Connection socket.
    std::shared_ptr<SocketReader> m_reader;     ///< Socket reader.
    static const String           empty_quotes; ///< Empty quotes string.
};

/**
 * @}
 */
} // namespace sptk
