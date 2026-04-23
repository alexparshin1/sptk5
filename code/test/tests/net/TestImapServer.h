/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       XMQ Message QUEUE                                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include "sptk5/net/SocketReader.h"
#include "sptk5/net/TCPServer.h"


#include <sptk5/cutils>

namespace sptk {
class TCPSocket;

class TestImapServer : public TCPServer
{
public:
    enum class Command
    {
        Login,
        Select,
        Examine,
        Close
    };

    /**
     * @brief Constructor.
     * @param port Port number to listen on.
     */
    explicit TestImapServer(uint16_t port);

    /**
     * @brief Destructor.
     */
    virtual ~TestImapServer() = default;

private:
    static void imapSession(const ServerConnection& socket);

    /**
     * @brief Read incoming data.
     */
    static void readIncomingData(const std::shared_ptr<SocketReader>& socketReader);
    static void reply(const std::shared_ptr<SocketReader>& socketReader, const std::string& response);

    static void handle_cmd_login(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident, const String& data);
    static void handle_cmd_select(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident);
    static void handle_cmd_examine(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident);
    static void handle_cmd_close(const std::string& ident, const std::shared_ptr<SocketReader>& socketReader);
};


} // namespace sptk
