/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       FTPConnect.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __FTPCONNECT_H__
#define __FTPCONNECT_H__

#include <sptk5/Strings.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief FTP socket
 *
 * Connects to FTP server. It takes two of such sockets
 * to establish the correct FTP communication
 */
class SP_EXPORT FTPSocket: public TCPSocket
{
    /**
     * Server response string list
     */
    Strings m_response;
public:
    /**
     * Default constructor
     */
    FTPSocket();

    /**
     * Destructor
     */
    ~FTPSocket();

    /**
     * Establish the connection
     */
    virtual void open(const std::string& hostName = "", uint32_t port = 0, CSocketOpenMode openMode = SOM_CONNECT,
                      bool blockingMode = true, uint32_t timeoutMS = 0) THROWS_EXCEPTIONS;

    /**
     * Returns a reference to server response string list
     */
    const Strings& response() const
    {
        return m_response;
    }

    /**
     * Logs in to the server
     */
    const Strings& login(std::string user, std::string password);

    /**
     * Sends a command to the server
     */
    const Strings& command(std::string cmd);

    /**
     * Reads server response after calling a command
     */
    const Strings& get_response();
};

/**
 * Connection to the FTP server
 */
class SP_EXPORT CFTPConnect
{
protected:
    /**
     * FTP command socket
     */
    FTPSocket  m_commandSocket;

    /**
     * FTP data socket
     */
    FTPSocket  m_dataSocket;

    /**
     * FTP user name
     */
    std::string m_user;

    /**
     * FTP user password
     */
    std::string m_password;

    /**
     * FTP host name
     */
    std::string m_host;

    /**
     * FTP port
     */
    uint32_t    m_port;

    /**
     * Passive mode
     */
    bool        m_passive;


protected:

    /**
     * Opens data port on the FTP server
     */
    void openDataPort();

    /**
     * Sends command to the FTP server
     */
    void command(const std::string& cmd);

    /**
     * Gets the list from the FTP server
     */
    void getList(const std::string& cmd, Strings& list);

public:

    /**
     * Constructor
     */
    CFTPConnect();

    /**
     * Destructor
     */
    ~CFTPConnect();

    /**
     * Sets the passive mode true/false
     */
    void passive(bool p)
    {
        m_passive = p;
    }

    /**
     * Returns the passive mode
     */
    bool passive() const
    {
        return m_passive;
    }

    /**
     * Sets the user name
     */
    void user(std::string u)
    {
        m_user = u;
    }

    /**
     * Returns the user name
     */
    std::string user() const
    {
        return m_user;
    }

    /**
     * Sets the user password
     */
    void password(std::string p)
    {
        m_password = p;
    }

    /**
     * Returns the user password
     */
    std::string password() const
    {
        return m_password;
    }

    /**
     * Sets the host name and port
     */
    void host(const std::string& hostName, uint32_t portNumber = 21);

    /**
     * Opens the FTP connection
     */
    void open();

    /**
     * Closes the FTP connection
     */
    void close();

    /**
     * Returns the referense on the server response string list
     */
    const Strings& response() const
    {
        return m_commandSocket.response();
    }

    /**
     * Returns true if the connection is active
     */
    bool active() const
    {
        return m_commandSocket.active();
    }

    /**
     * FTP quit command
     */
    void cmd_quit();

    /**
     * FTP type command
     */
    void cmd_type(char type);

    /**
     * FTP cd command
     */
    void cmd_cd(std::string dir);

    /**
     * FTP pwd command
     */
    void cmd_pwd();

    /**
     * FTP list command
     */
    void cmd_list(Strings& result);

    /**
     * FTP nlist command
     */
    void cmd_nlst(Strings& result);

    /**
     * FTP retr command
     */
    void cmd_retr(std::string fileName);

    /**
     * FTP store command
     */
    void cmd_store(std::string fileName);
};
/**
 * @}
 */
}
#endif
