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

SocketEventType EventWindow::translateEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SocketEventType events = ET_UNKNOWN_EVENT;
	
	if (uMsg == WM_SOCKET_EVENT) {
        switch (WSAGETSELECTEVENT(lParam)) {
        case FD_ACCEPT:
        case FD_READ:
            events = ET_HAS_DATA;
            break;
        case FD_CLOSE:
            events = ET_CONNECTION_CLOSED;
            break;
        }

        if (events == ET_UNKNOWN_EVENT && WSAGETSELECTERROR(lParam))
            events = ET_CONNECTION_CLOSED;
	}

	return events;
}

SocketEventType EventWindow::poll(SOCKET& socket, size_t timeoutMS)
{
	MSG			msg;
    UINT_PTR	timeoutTimerId = SetTimer(m_window, 0, (UINT) timeoutMS, NULL);

    int rc = GetMessage(&msg, m_window, 0, 0);
    if (rc == -1)
        return ET_UNKNOWN_EVENT;

    if (msg.message == WM_TIMER)
        return ET_UNKNOWN_EVENT; // timeout
    KillTimer(m_window, timeoutTimerId);

	if (msg.message == WM_SOCKET_EVENT) {
		SocketEventType eventType = translateEvent(msg.message, msg.wParam, msg.lParam);
		if (eventType == ET_UNKNOWN_EVENT)
			socket = 0;
		else
			socket = (SOCKET)msg.wParam;
		return eventType;
	}
	
	TranslateMessage(&msg);
	DispatchMessage(&msg);

	return ET_UNKNOWN_EVENT;
}

SocketPool::SocketPool(SocketEventCallback eventsCallback)
: m_pool(nullptr), m_eventsCallback(eventsCallback)
{
}

SocketPool::~SocketPool()
{
    close();
}

void SocketPool::open()
{
	lock_guard<mutex> lock(*this);

    if (m_pool)
        return;

    m_pool = new EventWindow(m_eventsCallback);
	m_threadId = this_thread::get_id();
}

void SocketPool::close()
{
	lock_guard<mutex> lock(*this);

	if (m_pool) {
        delete m_pool;
        m_pool = nullptr;
		m_socketData.clear();
	}
}

bool SocketPool::active()
{
	lock_guard<mutex> lock(*this);
	return m_pool != nullptr;
}

void SocketPool::watchSocket(BaseSocket& socket, void* userData)
{
    if (!socket.active())
        throw Exception("Socket is closed");

	int socketFD = socket.handle();

	lock_guard<mutex> lock(*this);

    if (WSAAsyncSelect(socketFD, m_pool->handle(), WM_SOCKET_EVENT, FD_ACCEPT|FD_READ|FD_CLOSE) != 0)
        throw SystemException("Can't add socket to WSAAsyncSelect");

    m_socketData[socketFD] = userData;
}

void SocketPool::forgetSocket(BaseSocket& socket)
{
    if (!socket.active())
        throw Exception("Socket is closed");

	int socketFD = socket.handle();
	
	{
        lock_guard<mutex> lock(*this);

        auto itor = m_socketData.find(socketFD);
        if (itor == m_socketData.end())
            return;

        m_socketData.erase(itor);
    }

    if (WSAAsyncSelect(socketFD, m_pool->handle(), WM_SOCKET_EVENT, 0))
        throw SystemException("Can't remove socket from WSAAsyncSelect");
}

void SocketPool::waitForEvents(chrono::milliseconds timeout)
{
	size_t timeoutMS = timeout.count();
    thread::id threadId = this_thread::get_id();
    if (threadId != m_threadId)
        throw Exception("SocketPool has to be used in the same must thread where it is created");

	SOCKET socketFD;
	SocketEventType eventType = ET_UNKNOWN_EVENT;
	{
		lock_guard<mutex> lock(*this);

		if (m_pool == nullptr) {
			this_thread::sleep_for(chrono::milliseconds(100));
			return;
		}
	}

	eventType = m_pool->poll(socketFD, timeoutMS);

	if (eventType != ET_UNKNOWN_EVENT) {
		lock_guard<mutex> lock(*this);

		auto itor = m_socketData.find(socketFD);
		if (itor == m_socketData.end())
			return;

		void* userData = itor->second;

		m_eventsCallback(userData, eventType);
	}
}
