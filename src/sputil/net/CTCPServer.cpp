/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTCPServer.cpp  -  description
                             -------------------
    begin                : Jul 13 2013
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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
#include <iostream>

using namespace std;
using namespace sptk;

bool CTCPServer::allowConnection(sockaddr_in* connectionRequest)
{
    return true;
}

CTCPServerListener::CTCPServerListener(CTCPServer* server, int port)
: CThread("CTCPServer::Listener"), m_server(server)
{
    m_listenerSocket.host("localhost");
    m_listenerSocket.port(port);
}

void CTCPServerListener::threadFunction()
{
    try {
        while (!terminated()) {
            if (m_listenerSocket.readyToRead(1000)) {
                try {
                    SOCKET connectionFD;
                    struct sockaddr_in connectionInfo;
                    m_listenerSocket.accept(connectionFD, connectionInfo);
                    if (int(connectionFD) == -1)
                        continue;
                    if (m_server->allowConnection(&connectionInfo)) {
                        CServerConnection* connection = m_server->createConnection(connectionFD, &connectionInfo);
                        m_server->registerConnection(connection);
                        connection->run();
                    }
                    else {
#ifndef _WIN32
                        shutdown(connectionFD,SHUT_RDWR);
                        ::close (connectionFD);
#else
                        closesocket(connectionFD);
#endif
                    }
                }
                catch (exception& e) {
                    m_server->log(CLP_ERROR, e.what());
                }
                catch (...) {
                    m_server->log(CLP_ERROR, "Unknown exception");
                }
            }
        }
    }
    catch (exception& e) {
        m_server->log(CLP_ERROR, e.what());
    }
    catch (...) {
        m_server->log(CLP_ERROR, "Unknown exception");
    }
}

void CTCPServerListener::terminate()
{
    CThread::terminate();
    m_listenerSocket.close();
}

void CTCPServer::listen(int port)
{
    SYNCHRONIZED_CODE;
    if (m_listenerThread) {
        m_listenerThread->terminate();
        m_listenerThread->join();
        delete m_listenerThread;
    }
    m_listenerThread = new CTCPServerListener(this, port);
    m_listenerThread->listen();
    m_listenerThread->run();
}

void CTCPServer::stop()
{
    SYNCHRONIZED_CODE;
    {
        CSynchronizedCode   m_sync(m_connectionThreadsLock);

        set<CServerConnection*>::iterator itor;

        for (itor = m_connectionThreads.begin(); itor != m_connectionThreads.end(); itor++)
            (*itor)->terminate();
    }

    while (true) {
        CThread::msleep(100);
        CSynchronizedCode   m_sync(m_connectionThreadsLock);
        if (m_connectionThreads.empty())
            break;
    }

    if (m_listenerThread) {
        m_listenerThread->terminate();
        m_listenerThread->join();
        delete m_listenerThread;
        m_listenerThread = NULL;
    }
}

void CTCPServer::registerConnection(CServerConnection* connection)
{
    CSynchronizedCode   m_sync(m_connectionThreadsLock);
    m_connectionThreads.insert(connection);
    connection->m_server = this;
    //cout << "Connection created" << endl;
}

void CTCPServer::unregisterConnection(CServerConnection* connection)
{
    CSynchronizedCode   m_sync(m_connectionThreadsLock);
    m_connectionThreads.erase(connection);
    //cout << "Connection closed" << endl;
}
