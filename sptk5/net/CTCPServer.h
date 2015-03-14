/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTCPServer.h  -  description
                             -------------------
    begin                : Jul 13 2013
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#ifndef __CTCPSERVER_H__
#define __CTCPSERVER_H__

#include <sptk5/net/CTCPSocket.h>
#include <sptk5/threads/CThread.h>
#include <set>

namespace sptk
{

class CTCPServer;

/// @brief Internal TCP server listener thread
class CTCPServerListener: public CThread
{
    CTCPServer*     m_server;
    CTCPSocket      m_listenerSocket;
    std::string     m_error;
public:
    /// @brief Constructor
    /// @param port int, Listener port number
    CTCPServerListener(CTCPServer* server, int port);

    /// @brief Thread function.
    virtual void threadFunction();

    void listen()
    {
        m_listenerSocket.listen();
    }

    std::string error() const
    {
        return m_error;
    }
};

/// @brief Abstract TCP connection thread
///
/// Application creates concrete TCP connection based on this class
/// to use with CTCPServer as connection template
class CTCPConnection: public CThread
{
protected:
    CTCPSocket      m_socket;
    CTCPServer*     m_server;
public:
    /// @brief Constructor
    /// @param connectionSocket SOCKET, Already accepted incoming connection socket
    CTCPConnection(SOCKET connectionSocket) :
            CThread("CTCPServer::Connection")
    {
        m_socket.attach(connectionSocket);
    }

    /// @brief Destructor
    virtual ~CTCPConnection();

    /// @brief Thread function
    virtual void threadFunction() = 0;

    /// @brief Method that is called upon thread exit
    virtual void onThreadExit();
};

/// @addtogroup net Networking Classes
/// @{

/// @brief TCP server
///
/// For every incoming connection, creates connection thread.
class CTCPServer: public CSynchronized
{
    friend class CTCPServerListener;
    friend class CTCPConnection;
    CTCPServerListener*         m_listenerThread;
    std::set<CTCPConnection*>   m_connectionThreads;
    CSynchronized               m_connectionThreadsLock;
protected:
    /// @brief Screens incoming connection request
    ///
    /// Method is called right after connection request is accepted,
    /// and allows ignore unwanted connections. By default simply returns true.
    /// @param connectionRequest sockaddr_in*, Incoming connection information
    virtual bool allowConnection(sockaddr_in* connectionRequest);

    /// @brief Creates connection thread derived from CTCPConnection
    ///
    /// Application should override this method to create concrete connection object.
    /// Created connection object is maintained by CTCPServer.
    /// @param connectionSocket SOCKET, Already accepted incoming connection socket
    /// @param peer sockaddr_in*, Incoming connection information
    virtual CTCPConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) = 0;

    /// @brief Receives notification on connection thread created
    /// @param connection CTCPConnection*, Newly created connection thread
    void registerConnection(CTCPConnection* connection);

    /// @brief Receives notification on connection thread exited
    ///
    /// Connection thread is self-destructing immediately after exiting this method
    /// @param connection CTCPConnection*, Exited connection thread
    void unregisterConnection(CTCPConnection* connection);

public:
    /// @brief Constructor
    CTCPServer() :
            m_listenerThread(NULL)
    {
    }

    /// @brief Destructor
    virtual ~CTCPServer()
    {
        stop();
    }

    /// @brief Starts listener
    /// @param port int, Listener port number
    void listen(int port);

    /// @brief Stops listener
    void stop();

    /// @brief Returns server state
    bool active() const
    {
        return m_listenerThread != NULL;
    }
};

/// @}
}
#endif
