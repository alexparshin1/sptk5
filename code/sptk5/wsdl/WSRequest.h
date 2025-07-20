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

#include <sptk5/cthreads>
#include <sptk5/net/HttpAuthentication.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * Namespace defined within WSDL document
 */
class SP_EXPORT WSNameSpace
{
public:
    /**
     * Constructor
     * @param alias             Namespace alias
     * @param location          Namespace location
     */
    WSNameSpace(const String& alias = "", const String& location = "")
        : m_alias(alias)
        , m_location(location)
    {
    }

    /**
     * Constructor
     * @param other             Other namespace
     */
    WSNameSpace(const WSNameSpace& other)
        : m_alias(other.m_alias)
        , m_location(other.m_location)
    {
    }

    /**
     * Destructor
     */
    ~WSNameSpace() noexcept = default;

    /**
     * Assignment
     * @param other             Other namespace
     * @return
     */
    WSNameSpace& operator=(const WSNameSpace& other)
    {
        if (&other != this)
        {
            std::scoped_lock lock(m_mutex);
            m_alias = other.m_alias;
            m_location = other.m_location;
        }
        return *this;
    }

    /**
     * Get namespace alias
     * @return Namespace alias
     */
    String getAlias() const
    {
        std::scoped_lock lock(m_mutex);
        return m_alias;
    }

    /**
     * Get namespace location
     * @return Namespace location
     */
    String getLocation() const
    {
        std::scoped_lock lock(m_mutex);
        return m_location;
    }

private:
    mutable std::mutex m_mutex;    ///< Mutex to protect internal data
    String             m_alias;    ///< Namespace alias
    String             m_location; ///< Namespace location
};

/**
 * Parser of WSDL requests
 */
class SP_EXPORT WSRequest
    : public std::mutex
{
public:
    /**
     * Constructor
     * @param logEngine        Optional log engine for error messages
     */
    explicit WSRequest(String targetNamespace, LogEngine* logEngine = nullptr)
        : m_logEngine(logEngine)
        , m_targetNamespace(targetNamespace)
    {
    }

    /**
     * Destructor
     */
    virtual ~WSRequest() = default;

    /**
     * Processes incoming requests
     *
     * The processing results are stored in the same request XML
     * @param xmlContent           Incoming request and outgoing response
     */
    void processRequest(const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                        HttpAuthentication* authentication, String& requestName);

    /**
     * Returns service title (for service handshake)
     *
     * Application should overwrite this method to return mor appropriate text
     */
    [[nodiscard]] virtual String title() const
    {
        return "Generic SPTK WS Request Broker";
    }

    /**
     * Returns service default HTML page
     *
     * Application should overwrite this method to return mor appropriate text
     */
    [[nodiscard]] virtual String defaultPage() const
    {
        return "index.html";
    }

    /**
     * @return service WSDL specifications
     */
    virtual String wsdl() const
    {
        return String("Not defined");
    }

    /**
     * @return service OpenAPI specifications
     */
    virtual String openapi() const
    {
        return String("Not defined");
    }

    static String tagName(const String& nodeName);

    LogEngine* getLogEngine()
    {
        return m_logEngine;
    }

protected:
    using RequestMethod = std::function<void(const xdoc::SNode&, const xdoc::SNode&,
                                             HttpAuthentication*, const WSNameSpace&)>;

    /**
     * Internal SOAP body processor
     *
     * Receives incoming SOAP body of Web Service requests, and returns
     * application response.
     * This method is abstract and overwritten in derived generated classes.
     * @param xmlContent        Incoming and outgoing SOAP element
     * @param authentication    Optional setRequestMethods(move(requestMethods));HTTP authentication
     * @param requestNameSpace  Request SOAP element namespace
     */
    virtual void requestBroker(const String& requestName, const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                               HttpAuthentication* authentication, const WSNameSpace& requestNameSpace);

    /**
     * Default error handling
     *
     * Forms server response in case of error. The response should contain error information.
     * @param xmlContent       Incoming XML request, or nullptr if JSON
     * @param jsonContent      Incoming JSON request, or nullptr if XML
     * @param error            Error description
     * @param errorCode        Optional HTTP error code, or 0
     */
    virtual void handleError(const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                             const String& error, int errorCode) const;

    /**
     * Default error logging
     *
     * Logs error information to default logger.
     * @param requestName      Request name
     * @param error            Error description
     * @param errorCode        Optional HTTP error code, or 0
     */
    virtual void logError(const String& requestName, const String& error, int errorCode) const;

    /**
     * Find SOAP body node
     * @param soapEnvelope
     * @return
     */
    xdoc::SNode findSoapBody(const xdoc::SNode& soapEnvelope, const WSNameSpace& soapNamespace);

    void setRequestMethods(std::map<String, RequestMethod>&& requestMethods);

private:
    LogEngine*                      m_logEngine;       ///< Optional logger, or nullptr
    std::map<String, RequestMethod> m_requestMethods;  ///< Map of requset names to methods
    String                          m_targetNamespace; ///< SOAP service target namespace
};

using SWSRequest = std::shared_ptr<WSRequest>;

} // namespace sptk
