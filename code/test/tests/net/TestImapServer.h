/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include "sptk5/net/SocketReader.h"
#include "sptk5/net/TCPServer.h"


#include <sptk5/cutils>

namespace sptk {
class TCPSocket;

class TestImapServer final : public TCPServer
{
public:
    /**
     * @brief Constructor.
     * @param port Port number to listen on.
     */
    explicit TestImapServer(uint16_t port);

private:
    static void imapSession(const ServerConnection& socket);

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
};


} // namespace sptk
