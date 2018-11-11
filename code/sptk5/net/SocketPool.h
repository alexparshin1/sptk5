/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday Sep 17 2015                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_SOCKETPOOL_H__
#define __SPTK_SOCKETPOOL_H__

#include <map>
#include <mutex>
#include <sptk5/Exception.h>
#include <sptk5/threads/Thread.h>
#include <sptk5/net/BaseSocket.h>

namespace sptk {

/**
 * Socket event types
 */
typedef enum {
    /**
     * Event is unknown or undefined
     */
    ET_UNKNOW_EVENT,
    /**
     * Socket has data available to read
     */
    ET_HAS_DATA,
    /**
     * Peer closed connection
     */
    ET_CONNECTION_CLOSED
} SocketEventType;

/**
 * Type definition of socket event callback function
 */
typedef void(*SocketEventCallback)(void *userData, SocketEventType eventType);

#ifdef _WIN32
    class EventWindowClass
    {
        std::string                 m_className;
        ATOM                        m_windowClass;
    public:
        EventWindowClass();
        static LRESULT CALLBACK EventWindowClass::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        const std::string className() const;
        const ATOM windowClass() const;
    };

    struct event {
        int      events;
        void*    udate;
    };

    class EventWindow
    {
        HWND                        m_window;
        SocketEventCallback         m_eventsCallback;

        static LRESULT CALLBACK     windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    public:
        EventWindow(SocketEventCallback eventsCallback);
        ~EventWindow();

        void eventMessageFunction(UINT uMsg, WPARAM wParam, LPARAM lParam);
        HWND handle() { return m_window; }

        int poll(std::vector<event>&, size_t timeoutMS);
    };
#endif

/**
 * Socket event manager.
 *
 * Uses OS-specific implementation.
 * On Linux it is using epoll, on BSD it is using kqueue,
 * and on Windows WSAAsyncSelect is used.
 */
class SocketPool : public std::mutex
{
private:

#ifdef _WIN32
    EventWindow*                m_pool;
    std::thread::id             m_threadId;
#else
    /**
     * Socket that controls other sockets events
     */
    SOCKET                      m_pool;
#endif

    /**
     * Callback function executed upon socket events
     */
    SocketEventCallback         m_eventsCallback;

    /**
     * Map of sockets to corresponding user data
     */
    std::map<BaseSocket*,void*> m_socketData;

public:
    /**
     * Constructor
     * @param eventCallback SocketEventCallback, Callback function executed upon socket events
     */
    SocketPool(SocketEventCallback eventCallback);

    /**
     * Destructor
     */
    ~SocketPool();

    /**
     * Initialize socket pool
     */
    void open();

    /**
     * Wait until one or more sockets are signaled.
     *
     * Execute callback function for each signaled socket.
     */
    void waitForEvents(std::chrono::milliseconds timeout);

    /**
     * Shutdown socket pool.
     */
    void close();

    /**
     * Add socket to monitored pool
     * @param socket BaseSocket&, Socket to monitor events
     * @param userData void*, User data to pass to callback function
     */
    void watchSocket(BaseSocket& socket, void* userData);

    /**
     * Remove socket from monitored pool
     * @param socket BaseSocket&, Socket from this pool
     */
    void forgetSocket(BaseSocket& socket);
};

}

#endif
