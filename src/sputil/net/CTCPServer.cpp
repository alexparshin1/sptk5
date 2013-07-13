/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTCPServer.cpp  -  description
                             -------------------
    begin                : Jul 13 2013
    copyright            : (C) 2000-2013 by Alexey Parshin. All rights reserved.
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

#include <sptk5/net/CTCPServer.h>

using namespace std;
using namespace sptk;

CTCPServerListener::CTCPServerListener(CTCPServer* server, int port)
: CThread("CTCPServer::Listener"), m_server(server)
{
    m_listenerSocket.host("localhost");
    m_listenerSocket.port(port);
}

void CTCPServerListener::threadFunction()
{
    try {
        m_listenerSocket.listen();
        while (!terminated()) {
            if (m_listenerSocket.readyToRead(1000)) {
                SOCKET connectionFD;
                struct sockaddr_in connectionInfo;
                m_listenerSocket.accept(connectionFD, connectionInfo);
                if (m_server->allowConnection(&connectionInfo)) {
                    CTCPConnection* connection = m_server->createConnection(connectionFD, &connectionInfo);
                    m_server->registerConnection(connection);
                    connection->run();
                } else {
                    #ifndef _WIN32
                        ::close (connectionFD);
                    #else
                        closesocket (connectionFD);
                    #endif
                }
            }
        }
    }
    catch (exception& e) {
        m_error = e.what();
    }
    catch (...) {
        m_error = "Unknown exception";
    }
}

void CTCPConnection::onThreadExit()
{
    m_server->unregisterConnection(this);
    delete this;
}

void CTCPServer::listen(int port)
{
    SYNCHRONIZED_CODE;
    if (m_listenerThread)
        throwException("Server already listens");
    m_listenerThread = new CTCPServerListener(this, port);
    m_listenerThread->run();
}

void CTCPServer::stop()
{
    SYNCHRONIZED_CODE;
    {
        CSynchronizedCode   m_sync(m_connectionThreadsLock);
        set<CTCPConnection*>::iterator itor, iend;

        for (itor = m_connectionThreads.begin(); itor != iend; itor++)
            (*itor)->terminate();
    }
    for (;;) {
        CThread::msleep(100);
        CSynchronizedCode   m_sync(m_connectionThreadsLock);
        if (m_connectionThreads.empty())
            break;
    }
    m_listenerThread->terminate();
    delete m_listenerThread;
}

void CTCPServer::registerConnection(CTCPConnection* connection)
{
    CSynchronizedCode   m_sync(m_connectionThreadsLock);
    m_connectionThreads.insert(connection);
    cout << "Connection created" << endl;
}

void CTCPServer::unregisterConnection(CTCPConnection* connection)
{
    CSynchronizedCode   m_sync(m_connectionThreadsLock);
    m_connectionThreads.erase(connection);
    cout << "Connection closed" << endl;
}
