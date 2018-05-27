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

#include "sptk5/net/SocketPool.h"
#include <iostream>
#include "sptk5/SystemException.h"

using namespace std;
using namespace sptk;

EventWindowClass::EventWindowClass()
{
	m_className = "EventWindow" + int2string(time(NULL));

	WNDCLASS wndclass;
	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)windowProc;
	wndclass.lpszClassName = m_className.c_str();
	m_windowClass = RegisterClass(&wndclass);
	if (!windowClass())
		throw Exception("Can't register event window class");
}

LRESULT CALLBACK EventWindowClass::EventWindowClass::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

const string EventWindowClass::className() const
{
	return m_className;
}

const ATOM EventWindowClass::windowClass() const
{
	return m_windowClass;
}

static const EventWindowClass eventWindowClass;

EventWindow::EventWindow(SocketEventCallback eventsCallback)
: m_eventsCallback(eventsCallback)
{
    m_window = CreateWindow(eventWindowClass.className().c_str(),
                                  "", WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                  NULL, NULL, NULL, NULL);
    if (!m_window)
        throw SystemException("Can't create EventWindow");
}

EventWindow::~EventWindow()
{
    DestroyWindow(m_window);
}

#define WM_SOCKET_EVENT (WM_USER + 1000)

void EventWindow::eventMessageFunction(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_SOCKET_EVENT) {
        SocketEventType events = ET_UNKNOW_EVENT;

        switch (WSAGETSELECTEVENT(lParam)) {
        case FD_ACCEPT:
        case FD_READ:
            events = ET_HAS_DATA;
            break;
        case FD_CLOSE:
            events = ET_CONNECTION_CLOSED;
            break;
        }

        if (events == ET_UNKNOW_EVENT && WSAGETSELECTERROR(lParam))
            events = ET_CONNECTION_CLOSED;

        if (events)
            m_eventsCallback((void*)wParam, events);
   }
}

int EventWindow::poll(std::vector<event>&, size_t timeoutMS)
{
	MSG			msg;
    UINT_PTR	timeoutTimerId = SetTimer(m_window, 0, (UINT) timeoutMS, NULL);

    int rc = GetMessage(&msg, m_window, 0, 0);
    if (rc == -1)
        return 0;

    if (msg.message == WM_TIMER)
        return false; // timeout
    KillTimer(m_window, timeoutTimerId);

    eventMessageFunction(msg.message, msg.wParam, msg.lParam);

    return 1;
}

SocketPool::SocketPool(SocketEventCallback eventsCallback)
: m_pool(NULL), m_threadId(this_thread::get_id()), m_eventsCallback(eventsCallback)
{
    open();
}

SocketPool::~SocketPool()
{
    close();
}

void SocketPool::open()
{
    if (m_pool)
        return;
    m_pool = new EventWindow(m_eventsCallback);
}

void SocketPool::close()
{
    if (m_pool) {
        delete m_pool;
        m_pool = NULL;
    }

    lock_guard<mutex> lock(*this);
    for (auto itor: m_socketData)
        free(itor.second);
    m_socketData.clear();
}

void SocketPool::watchSocket(BaseSocket& socket, void* userData)
{
    if (!socket.active())
        throw Exception("Socket is closed");

    lock_guard<mutex> lock(*this);

    int socketFD = socket.handle();

    if (WSAAsyncSelect(socketFD, m_pool->handle(), WM_SOCKET_EVENT, FD_ACCEPT|FD_READ|FD_CLOSE) != 0)
        throw SystemException("Can't add socket to WSAAsyncSelect");

    m_socketData[&socket] = userData;
}

void SocketPool::forgetSocket(BaseSocket& socket)
{
    if (!socket.active())
        throw Exception("Socket is closed");

    {
        lock_guard<mutex> lock(*this);

        map<BaseSocket*,void*>::iterator itor = m_socketData.find(&socket);
        if (itor == m_socketData.end())
            return;

        m_socketData.erase(itor);
    }

    if (WSAAsyncSelect(socket.handle(), m_pool->handle(), WM_SOCKET_EVENT, 0))
        throw SystemException("Can't remove socket from WSAAsyncSelect");
}

void SocketPool::waitForEvents(chrono::milliseconds timeout)
{
	size_t timeoutMS = timeout.count();
    thread::id threadId = this_thread::get_id();
    if (threadId != m_threadId)
        throw Exception("SocketPool has to be used in the same must thread where it is created");

    vector<event> signaled;
    m_pool->poll(signaled, timeoutMS);
/*
    for (int i = 0; i < eventCount; i++) {
        epoll_event& event = events[i];
        if (event.events & (EPOLLHUP | EPOLLRDHUP))
            m_eventsCallback(event.data.ptr, ET_CONNECTION_CLOSED);
        else
            m_eventsCallback(event.data.ptr, ET_HAS_DATA);
    }
*/
}
