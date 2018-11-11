/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSRequest.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __SPTK_WSREQUEST_H__
#define __SPTK_WSREQUEST_H__

#include <sptk5/cxml>
#include <sptk5/cthreads>
#include <sptk5/net/HttpAuthentication.h>

namespace sptk
{

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * @brief Namespace defined within WSDL document
 */
class WSNameSpace
{
    mutable std::mutex  m_mutex;        ///< Mutex to protect internal data
    String              m_alias;        ///< Namespace alias
    String              m_location;     ///< Namespace location

public:

    /**
     * @brief Constructor
     * @param alias             Namespace alias
     * @param location          Namespace location
     */
    WSNameSpace(const String& alias="", const String& location="")
    : m_alias(alias), m_location(location)
    {}

    /**
     * @brief Constructor
     * @param other             Other namespace
     */
    WSNameSpace(const WSNameSpace& other)
    : m_alias(other.m_alias), m_location(other.m_location)
    {}

    /**
     * @brief Assignment
     * @param other             Other namespace
     * @return
     */
    WSNameSpace& operator = (const WSNameSpace& other)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_alias = other.m_alias;
        m_location = other.m_location;
        return *this;
    }

    /**
     * @brief Get namespace alias
     * @return Namespace alias
     */
    String getAlias() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_alias;
    }

    /**
     * @brief Get namespace location
     * @return Namespace location
     */
    String getLocation() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_location; 
    }
};

/**
 * @brief Parser of WSDL requests
 */
class WSRequest : public std::mutex
{
protected:
    /**
     * @brief Internal SOAP body processor
     *
     * Receives incoming SOAP body of Web Service requests, and returns
     * application response.
     * This method is abstract and overwritten in derived generated classes.
     * @param requestNode       Incoming and outgoing SOAP element
     * @param authentication    Optional HTTP authentication
     * @param requestNameSpace  Request SOAP element namespace
     */
    virtual void requestBroker(xml::Element* requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace) = 0;

public:
    /**
     * @brief Constructor
     */
    WSRequest() {}

    /**
     * @brief Destructor
     */
    virtual ~WSRequest() {}

    /**
     * @brief Processes incoming requests
     *
     * The processing results are stored in the same request XML
     * @param request           Incoming request and outgoing response
     */
    void processRequest(xml::Document* request, HttpAuthentication* authentication);

    /**
     * @brief Returns service title (for service handshake)
     *
     * Application should overwrite this method to return mor appropriate text
     */
    virtual std::string title() const
    {
        return "Generic SPTK WS Request Broker";
    }

    /**
     * @brief Returns service default HTML page
     *
     * Application should overwrite this method to return mor appropriate text
     */
    virtual std::string defaultPage() const
    {
        return "index.html";
    }
};

}
#endif
