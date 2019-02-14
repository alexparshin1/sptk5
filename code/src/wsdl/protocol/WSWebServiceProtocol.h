/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSWebServiceProtocol.h - description                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __WSWEBSERVICEPROTOCOL_H__
#define __WSWEBSERVICEPROTOCOL_H__

#include "WSProtocol.h"
#include <sptk5/cnet>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief WebService connection handler
///
/// Uses WSRequest service object to parse WS request and
/// reply, then closes connection.
class WSWebServiceProtocol : public WSProtocol
{
    WSRequest&      m_service;  ///< Web Service
    const String    m_url;      ///< Request URL
    const String    m_hostname; ///< Listener's hostname
    const uint16_t  m_port;     ///< Listener's port

    /**
     * Process request message, and store response to output
     * @param output            Output buffer
     * @param message           Input message
     * @param authentication    Authentication
     * @param requestIsJSON     Request is in JSON format
     * @param httpStatusCode    Output HTTP status code
     * @param httpStatusText    Output HTTP status text
     * @param contentType       Output content type
     */
    void processMessage(Buffer& output, xml::Document& message,
                        std::shared_ptr<HttpAuthentication> authentication, bool requestIsJSON,
                        size_t& httpStatusCode, String& httpStatusText, String& contentType);
public:

    /**
     * @brief Constructor
     * @param socket            Connection socket
     * @param url               Method URL
     * @param headers           Connection HTTP headers
     * @param service           Web service that handles request
     * @param hostname          Listener's hostname
     * @param port              Listener's port
     */
    WSWebServiceProtocol(TCPSocket* socket, const String& url, const HttpHeaders& headers,
                         WSRequest& service, const String& hostname, uint16_t port);

    /// @brief Process method
    ///
    /// Calls WebService request through service object
    void process() override;

private:

    xml::Node* getFirstChildElement(const xml::Node* element) const;

    xml::Node* findRequestNode(const xml::Document& message, const String& messageType) const;

    void generateFault(Buffer& output, size_t& httpStatusCode, String& httpStatusText, String& contentType,
                       const HTTPException& e, bool jsonOutput);

    void RESTtoSOAP(Strings& url, const char* startOfMessage, xml::Document& message) const;
};

/// @}

} // namespace sptk

#endif
