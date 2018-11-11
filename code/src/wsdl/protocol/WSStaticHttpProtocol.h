/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSStaticHttpProtocol.h - description                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
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

#ifndef __WSHTTPPROTOCOL_H__
#define __WSHTTPPROTOCOL_H__

#include "WSProtocol.h"
#include <sptk5/cnet>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Handler for static files (.html, .js, .png, etc)
///
/// Session disconnects as soon as file is served.
class WSStaticHttpProtocol : public WSProtocol
{
    String  m_url;                      ///< File URL
    String  m_staticFilesDirectory;     ///< Directory where static files reside on the server
public:

    /// @brief Constructor
    /// @param socket           Connection socket
    /// @param url              File URL
    /// @param headers          Connection HTTP headers
    /// @param staticFilesDirectory String, Directory where static files reside on the server
    WSStaticHttpProtocol(TCPSocket* socket, const String& url, const HttpHeaders& headers, const String& staticFilesDirectory);

    /// @brief Process method
    ///
    /// Writes HTTP response and file content to the connection
    void process() override;
};

/// @}

} // namespace sptk

#endif
