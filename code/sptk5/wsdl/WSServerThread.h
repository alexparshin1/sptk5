/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/wsdl/WSConnection.h>

namespace sptk {

class WSServer;

/**
 * @brief Web service server thread
 */
class SP_EXPORT WSServerThread : public Thread
{
public:
    /**
     * @brief Constructor
     * @param server        Web service server
     */
    WSServerThread(WSServer* server);

    /**
     * @brief Destructor
     */
    ~WSServerThread() override;

    /**
     * @brief Queue connection that has incoming data
     * @param connection    Web service connection
     */
    void queue(const std::shared_ptr<WSConnection>& connection);

protected:
    WSServer*                                        m_server;
    SynchronizedQueue<std::shared_ptr<WSConnection>> m_connectionQueue;

    void threadFunction() override;
};

using SWSServerThread = std::shared_ptr<WSServerThread>;

/**
 * @brief Web service server threads
 */
class SP_EXPORT WSServerThreads
{
public:
    /**
     * @brief Constructor
     * @param server        Web service server
     * @param threadCount   Number of threads
     */
    WSServerThreads(WSServer* server, size_t threadCount);

    /**
     * @brief Get next thread from the pool
     * @return thread
     */
    SWSServerThread nextThread();

    /**
     * @brief Terminate all threads
     */
    void terminate();

private:
    std::mutex                   m_mutex;
    std::vector<SWSServerThread> m_threads;
    size_t                       m_nextThreadIndex = 0;
};

} // namespace sptk
