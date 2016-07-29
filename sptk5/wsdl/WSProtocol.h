/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSProtocol.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
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

#ifndef __WSPROTOCOL_H__
#define __WSPROTOCOL_H__

#include <sptk5/cnet>
#include <sptk5/wsdl/WSRequest.h>

namespace sptk {

class WSProtocol
{
protected:
    TCPSocket&                      m_socket;
    const std::map<String,String>&  m_headers;
public:
    WSProtocol(TCPSocket *socket, const std::map<String,String>& headers)
    : m_socket(*socket), m_headers(headers)
    {
    }

    virtual ~WSProtocol()
    {
        m_socket.close();
    }
    
    virtual void process() = 0;
};

class WSStaticHttpProtocol : public WSProtocol
{
    String  m_url;
    String  m_staticFilesDirectory;
public:
    WSStaticHttpProtocol(TCPSocket *socket, String url, const std::map<String,String>& headers, String staticFilesDirectory);
    virtual void process();
};

class WSWebSocketsProtocol : public WSProtocol
{
public:
    WSWebSocketsProtocol(TCPSocket *socket, const std::map<String,String>& headers);
    virtual void process();
};

class WSWebServiceProtocol : public WSProtocol
{
    WSRequest&     m_service;
public:
    WSWebServiceProtocol(TCPSocket *socket, const std::map<String,String>& headers, WSRequest& service);
    virtual void process();
};

}

#endif
