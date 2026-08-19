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

#include "sptk5/net/FastTCPServer.h"
#include "sptk5/net/SocketReader.h"

#include <sptk5/cutils>

namespace sptk {
class TCPSocket;

class TestImapServer final : public FastTCPServer
{
public:
    /**
     * @brief Constructor.
     * @param port Port number to listen on.
     */
    explicit TestImapServer(uint16_t port);

    ~TestImapServer() override;

protected:
    /**
     * @brief Create an IMAP connection and send the server greeting.
     */
    SServerConnection createConnection(ServerConnection::Type connectionType, SocketType connectionSocket,
                                       const sockaddr_in* peer) override;

    /**
     * @brief Process IMAP commands as they arrive on a monitored connection.
     */
    void socketEventCallback(const std::shared_ptr<ServerConnection>& connection, SocketEventType eventType) override;

private:
    /**
     * @brief Read incoming data.
     */
    static void readIncomingData(const std::shared_ptr<SocketReader>& socketReader);
    static void reply(const std::shared_ptr<SocketReader>& socketReader, const std::string& response);

    static void handle_cmd_capability(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident);
    static void handle_cmd_login(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_select(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident);
    static void handle_cmd_examine(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident);
    static void handle_cmd_close(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident);
    static void handle_cmd_subscribe(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_unsubscribe(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_create(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_delete(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_rename(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_list(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_append(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_fetch(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_logout(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident);
};


} // namespace sptk
