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

#include <sptk5/cutils>

#include "sptk5/net/TCPServer.h"
#include <sptk5/net/SSLServerConnection.h>
#include <sptk5/net/TCPServerConnection.h>
#include <sptk5/net/TCPServerListener.h>


using namespace std;
using namespace sptk;

const map<String, LogDetails::MessageDetail> LogDetails::detailNames {
    {"serial_id", MessageDetail::SERIAL_ID},
    {"source_ip", MessageDetail::SOURCE_IP},
    {"request_name", MessageDetail::REQUEST_NAME},
    {"request_duration", MessageDetail::REQUEST_DURATION},
    {"request_data", MessageDetail::REQUEST_DATA},
    {"response_data", MessageDetail::RESPONSE_DATA},
    {"thread_pooling", MessageDetail::THREAD_POOLING}};

LogDetails::LogDetails(const Strings& details)
{
    for (const auto& detailName: details)
    {
        const auto itor = detailNames.find(detailName);
        if (itor == detailNames.end())
        {
            continue;
        }
        m_details.insert(itor->second);
    }
}

String LogDetails::toString(const String& delimiter) const
{
    Strings names;
    for (const auto& [name, value]: detailNames)
    {
        if (m_details.contains(value))
        {
            names.push_back(name);
        }
    }
    return names.join(delimiter.c_str());
}

TCPServer::TCPServer(const String& listenerName, size_t threadLimit, LogEngine* logEngine, const LogDetails& logDetails)
    : ThreadPool(static_cast<uint32_t>(threadLimit),
                 60s,
                 listenerName,
                 logDetails.has(LogDetails::MessageDetail::THREAD_POOLING) ? logEngine : nullptr)
    , m_logDetails(logDetails)
{
    if (logEngine != nullptr)
    {
        m_logger = make_shared<Logger>(*logEngine);
    }

    constexpr unsigned             maxHostNameLength = 128;
    array<char, maxHostNameLength> hostname = {"localhost"};
    const int                      result = gethostname(hostname.data(), sizeof(hostname));
    if (result == 0)
    {
        m_host = Host(hostname.data());
    }
}

TCPServer::~TCPServer()
{
    TCPServer::stop();
}

const Host& TCPServer::host() const
{
    const scoped_lock lock(m_mutex);
    return m_host;
}

void TCPServer::host(const Host& host)
{
    const scoped_lock lock(m_mutex);
    if (!m_portListeners.empty())
    {
        throw Exception("Can't change host while listening");
    }
    m_host = host;
}

void TCPServer::addListener(ServerConnection::Type connectionType, uint16_t port, uint16_t threadCount)
{
    scoped_lock lock(m_mutex);

    auto& listenerThreads = m_portListeners[port];
    if (!listenerThreads.empty())
    {
        throw Exception("Port is already used");
    }

    if (threadCount == 0)
    {
        threadCount = 1;
    }

    m_host.port(port);
    for (uint16_t i = 0; i < threadCount; ++i)
    {
        auto listenerThread = make_unique<TCPServerListener>(this, port, connectionType);
        listenerThread->listen();
        listenerThreads.push_back(std::move(listenerThread));
    }
}

[[maybe_unused]] void TCPServer::removeListener(uint16_t port)
{
    const scoped_lock lock(m_mutex);

    auto& listenerThreads = m_portListeners[port];

    for (const auto& listenerThread: listenerThreads)
    {
        listenerThread->stop();
    }

    listenerThreads.clear();

    m_portListeners.erase(port);
}

bool TCPServer::allowConnection(const sockaddr_in*)
{
    return true;
}

void TCPServer::stop()
{
    const scoped_lock lock(m_mutex);

    if (!m_portListeners.empty())
    {
        for (auto& [port, listenerThreads]: m_portListeners)
        {
            for (const auto& listenerThread: listenerThreads)
            {
                listenerThread->stop();
            }
            listenerThreads.clear();
        }
        m_portListeners.clear();
    }

    ThreadPool::stop();
}

void TCPServer::setSSLKeys(shared_ptr<SSLKeys> sslKeys)
{
    const scoped_lock lock(m_mutex);
    if (!filesystem::exists(sslKeys->certificateFileName()))
    {
        throw Exception("Can't find certificate file: " + m_sslKeys->certificateFileName().string());
    }
    m_sslKeys = std::move(sslKeys);
}

std::shared_ptr<SSLKeys> TCPServer::getSSLKeys() const
{
    const scoped_lock lock(m_mutex);
    return m_sslKeys;
}

void TCPServer::threadEvent(Thread* thread, Type eventType, SRunable runable)
{
    if (eventType == Type::RUNABLE_FINISHED)
    {
        runable.reset();
    }
    ThreadPool::threadEvent(thread, eventType, runable);
}

UServerConnection TCPServer::createConnection(ServerConnection::Type connectionType, SocketType connectionSocket, const sockaddr_in* peer)
{
    if (connectionType == ServerConnection::Type::TCP)
    {
        return make_unique<TCPServerConnection>(*this, connectionSocket, peer, m_connectionFunction);
    }
    return make_unique<SSLServerConnection>(*this, connectionSocket, peer, m_connectionFunction);
}

void TCPServer::onConnection(const ServerConnection::Function& function)
{
    m_connectionFunction = function;
}
