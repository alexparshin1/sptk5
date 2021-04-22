/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/cnet>
#include <sptk5/net/URL.h>
#include <sptk5/net/HttpResponseStatus.h>
#include <sptk5/wsdl/WSServices.h>
#include "WSProtocol.h"
#include "sptk5/wsdl/protocol/BaseWebServiceProtocol.h"

namespace sptk {

class BaseWebServiceProtocol : public WSProtocol
{
    xml::Node* getFirstChildElement(const xml::Node* element) const;

public:
    BaseWebServiceProtocol(TCPSocket* socket, const HttpHeaders& headers, sptk::WSServices& services, const URL& url);

protected:
    WSServices&         m_services;
    const URL           m_url;

    virtual std::shared_ptr<HttpAuthentication> getAuthentication() = 0;

    virtual void generateFault(Buffer& output, HttpResponseStatus& httpStatus, String& contentType,
                               const HTTPException& e, bool jsonOutput) const = 0;

    void RESTtoSOAP(const URL& url, const char* startOfMessage, xml::Document& message) const;

    xml::Node* findRequestNode(const xml::Document& message, const String& messageType) const;

    void processXmlContent(const char* startOfMessage, xml::Document& xmlContent, json::Document& jsonContent) const;

    void processJsonContent(const char* startOfMessage, json::Document& jsonContent,
                            RequestInfo& requestInfo, HttpResponseStatus& httpStatus,
                            String& contentType) const;

/**
 * Process request message, and store response to output
 * @param output                Output buffer
 * @param xmlContent            Input message
 * @param authentication        Authentication
 * @param requestIsJSON         Request is in JSON format
 * @param httpResponseStatus    Output HTTP response status
 * @param contentType           Output content type
 */
String processMessage(Buffer& output, xml::Document& xmlContent, json::Document& jsonContent,
                      const SHttpAuthentication& authentication, bool requestIsJSON,
                      HttpResponseStatus& httpResponseStatus, String& contentType) const;
};

}

