/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include "sptk5/net/LogDetails.h"
#include "sptk5/wsdl/protocol/BaseWebServiceProtocol.h"
#include <sptk5/cnet>
#include <sptk5/net/HttpResponseStatus.h>
#include <sptk5/net/URL.h>
#include <sptk5/wsdl/WSServices.h>

namespace sptk {
/// @addtogroup wsdl WSDL-related Classes
/// @{

/// WebService connection handler
///
/// Uses WSRequest service object to parse WS request and
/// reply, then closes connection.
class SP_EXPORT WSWebServiceProtocol : public BaseWebServiceProtocol
{
public:
    /**
     * Constructor
     * @param httpReader        Connection socket
     * @param url               Method URL
     * @param headers           Connection HTTP headers
     * @param service           Services that handle request
     * @param host              Listener's hostname
     * @param port              Listener's port
     * @param allowCORS         Allow CORS
     * @param keepAlive         Keep alive
     */
    WSWebServiceProtocol(HttpReader& httpReader, const URL& url, WSServices& services,
                         Host host, bool allowCORS, bool keepAlive, bool suppressHttpStatus);

    /*
     * Process method
     *
     * Calls WebService request through service object
     */
    RequestInfo process() override;

    void generateFault(Buffer& output, HttpResponseStatus& httpStatus, String& contentType,
                       const HTTPException& e, bool jsonOutput) const override;

protected:
    std::shared_ptr<HttpAuthentication> getAuthentication() override;

private:
    HttpReader& m_httpReader;         ///< HTTP reader
    Host        m_host;               ///< Listener's host
    bool        m_allowCORS;          ///< Allow CORS?
    bool        m_keepAlive;          ///< Allow keep-alive connections
    bool        m_suppressHttpStatus; ///< If true, then HTTP status is 202 Accepted even if HttpException raised
    LogDetails  m_logDetails;         ///< Log details
};

/// @}

} // namespace sptk
