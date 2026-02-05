/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
#include <utility>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes.
 * @{
 */

/**
 * @brief Namespace defined within the WSDL document.
 */
class SP_EXPORT WSNameSpace
{
public:
    /**
     * @brief Constructor.
     * @param alias             Namespace alias.
     * @param location          Namespace location.
     */
    explicit WSNameSpace(String alias = "", String location = "")
        : m_alias(std::move(alias))
        , m_location(std::move(location))
    {
    }

    /**
     * @brief Constructor.
     * @param other             Another namespace.
     */
    WSNameSpace(const WSNameSpace& other)
        : m_alias(other.m_alias)
        , m_location(other.m_location)
    {
    }

    /**
     * @brief Destructor.
     */
    ~WSNameSpace() noexcept = default;

    /**
     * @brief Assignment.
     * @param other             Another namespace.
     * @return.
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
     * @brief Get namespace alias.
     * @return Namespace alias.
     */
    String getAlias() const
    {
        std::scoped_lock lock(m_mutex);
        return m_alias;
    }

    /**
     * @brief Get namespace location.
     * @return Namespace location.
     */
    String getLocation() const
    {
        std::scoped_lock lock(m_mutex);
        return m_location;
    }

private:
    mutable std::mutex m_mutex;    ///< Mutex to protect internal data.
    String             m_alias;    ///< Namespace alias.
    String             m_location; ///< Namespace location.
};

/**
 * @brief Parser of WSDL requests.
 */
class SP_EXPORT WSRequest
    : public std::mutex
{
public:
    /**
     * @brief Constructor.
     * @param targetNamespace   Target namespace.
     * @param logEngine         Optional log engine for error messages.
     */
    explicit WSRequest(String targetNamespace, std::shared_ptr<LogEngine> logEngine = nullptr)
        : m_logEngine(std::move(logEngine))
        , m_targetNamespace(std::move(targetNamespace))
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~WSRequest() = default;

    /**
     * @brief Processes incoming requests.
     *
     * The processing results are stored in the same request.
     * @param xmlContent        Incoming request and outgoing response in XML format.
     * @param jsonContent       Incoming request and outgoing response in JSON format.
     * @param authentication    Request authentication object.
     * @param requestName       Request name.
     */
    void processRequest(const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                        HttpAuthentication* authentication, String& requestName);

    /**
     * @brief Returns service title (for service handshake).
     *
     * Application should overwrite this method to return mor appropriate text.
     */
    [[nodiscard]] virtual String title() const
    {
        return "Generic SPTK WS Request Broker";
    }

    /**
     * @brief Returns service default HTML page.
     *
     * Application should overwrite this method to return mor appropriate text.
     */
    [[nodiscard]] virtual String defaultPage() const
    {
        return "index.html";
    }

    /**
     * @return service WSDL specifications.
     */
    [[nodiscard]] virtual String wsdl() const
    {
        return String("Not defined");
    }

    /**
     * @return service OpenAPI specifications.
     */
    [[nodiscard]] virtual String openapi() const
    {
        return String("Not defined");
    }

    [[nodiscard]] static String tagName(const String& nodeName);

    [[nodiscard]] std::shared_ptr<LogEngine> getLogEngine() const
    {
        return m_logEngine;
    }

protected:
    using RequestMethod = std::function<void(const xdoc::SNode&, const xdoc::SNode&,
                                             HttpAuthentication*, const WSNameSpace&)>;

    /**
     * @brief Internal SOAP body processor.
     *
     * Receives incoming SOAP body of Web Service requests, and returns application response.
     * This method is abstract and overwritten in derived generated classes.
     * @param requestName       Request name.
     * @param xmlContent        Incoming and outgoing SOAP element.
     * @param jsonContent       Incoming and outgoing JSON element.
     * @param authentication    HTTP authentication.
     * @param requestNameSpace  Request SOAP element namespace.
     */
    virtual void requestBroker(const String& requestName, const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                               HttpAuthentication* authentication, const WSNameSpace& requestNameSpace);

    /**
     * @brief Default error handling.
     *
     * Forms the server response in case of error. The response should contain error information.
     * @param xmlContent       Incoming XML request, or nullptr if JSON.
     * @param jsonContent      Incoming JSON request, or nullptr if XML.
     * @param error            Error description.
     * @param errorCode        Optional HTTP error code, or 0.
     */
    virtual void handleError(const xdoc::SNode& xmlContent, const xdoc::SNode& jsonContent,
                             const String& error, int errorCode) const;

    /**
     * @brief Default error logging.
     *
     * Logs error information to default logger.
     * @param requestName      Request name.
     * @param error            Error description.
     * @param errorCode        Optional HTTP error code, or 0.
     */
    virtual void logError(const String& requestName, const String& error, int errorCode) const;

    /**
     * @brief Find the SOAP body node.
     * @param soapEnvelope      SOAP envelope node.
     * @param soapNamespace     SOAP namespace.
     * @return SOAP Body node.
     */
    xdoc::SNode findSoapBody(const xdoc::SNode& soapEnvelope, const WSNameSpace& soapNamespace);

    void setRequestMethods(std::map<String, RequestMethod>&& requestMethods);

private:
    std::shared_ptr<LogEngine>      m_logEngine;       ///< Optional logger, or nullptr.
    std::map<String, RequestMethod> m_requestMethods;  ///< Map of requset names to methods.
    String                          m_targetNamespace; ///< SOAP service target namespace.
};

using SWSRequest = std::shared_ptr<WSRequest>;

} // namespace sptk
