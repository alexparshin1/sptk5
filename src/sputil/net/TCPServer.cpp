/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPServer.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/net/TCPServer.h>

using namespace std;
using namespace sptk;

bool TCPServer::allowConnection(sockaddr_in* connectionRequest)
{
    return true;
}

TCPServerListener::TCPServerListener(TCPServer* server, int port)
: Thread("CTCPServer::Listener"), m_server(server)
{
    m_listenerSocket.host(Host("localhost", port));
}

void TCPServerListener::threadFunction()
{
    try {
        while (!terminated()) {
            SYNCHRONIZED_CODE;
            if (m_listenerSocket.readyToRead(1000)) {
                try {
                    SOCKET connectionFD;
                    sockaddr_in connectionInfo = {};
                    m_listenerSocket.accept(connectionFD, connectionInfo);
                    if (int(connectionFD) == -1)
                        continue;
                    if (m_server->allowConnection(&connectionInfo)) {
                        ServerConnection* connection = m_server->createConnection(connectionFD, &connectionInfo);
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
                    m_server->log(LP_ERROR, e.what());
                }
                catch (...) {
                    m_server->log(LP_ERROR, "Unknown exception");
                }
            }
        }
    }
    catch (exception& e) {
        m_server->log(LP_ERROR, e.what());
    }
    catch (...) {
        m_server->log(LP_ERROR, "Unknown exception");
    }
}

void TCPServerListener::terminate()
{
    Thread::terminate();

    SYNCHRONIZED_CODE;
    m_listenerSocket.close();
}

void TCPServer::listen(int port)
{
    SYNCHRONIZED_CODE;
    if (m_listenerThread != nullptr) {
        m_listenerThread->terminate();
        m_listenerThread->join();
        delete m_listenerThread;
    }
    m_listenerThread = new TCPServerListener(this, port);
    m_listenerThread->listen();
    m_listenerThread->run();
}

void TCPServer::stop()
{
    SYNCHRONIZED_CODE;
    {
        SynchronizedCode   m_sync(m_connectionThreadsLock);

        set<ServerConnection*>::iterator itor;

        for (itor = m_connectionThreads.begin(); itor != m_connectionThreads.end(); ++itor)
            (*itor)->terminate();
    }

    while (true) {
        Thread::msleep(100);
        SynchronizedCode   m_sync(m_connectionThreadsLock);
        if (m_connectionThreads.empty())
            break;
    }

    if (m_listenerThread != nullptr) {
        m_listenerThread->terminate();
        m_listenerThread->join();
        delete m_listenerThread;
        m_listenerThread = nullptr;
    }
}

void TCPServer::registerConnection(ServerConnection* connection)
{
    SynchronizedCode   m_sync(m_connectionThreadsLock);
    m_connectionThreads.insert(connection);
    connection->m_server = this;
    //cout << "Connection created" << endl;
}

void TCPServer::unregisterConnection(ServerConnection* connection)
{
    SynchronizedCode   m_sync(m_connectionThreadsLock);
    m_connectionThreads.erase(connection);
    //cout << "Connection closed" << endl;
}
