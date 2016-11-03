/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSRequest.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __SPTK_WSREQUEST_H__
#define __SPTK_WSREQUEST_H__

#include <sptk5/cxml>
#include <sptk5/cthreads>

namespace sptk
{

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Namespace defined within WSDL document
class WSNameSpace
{
    String  m_alias;        ///< Namespace alias
    String  m_location;     ///< Namespace location
public:
    
    /// @brief Constructor
    /// @param alias String, Namespace alias
    /// @param location String, Namespace location
    WSNameSpace(String alias="", String location="")
    : m_alias(alias), m_location(location)
    {}
    
    /// @brief Constructor
    /// @param other const WSNameSpace&, Other namespace
    WSNameSpace(const WSNameSpace& other)
    : m_alias(other.m_alias), m_location(other.m_location)
    {}
    
    /// @brief Get namespace alias
    /// @return Namespace alias
    const String& getAlias() const { return m_alias; }
    
    /// @brief Get namespace location
    /// @return Namespace location
    const String& getLocation() const { return m_location; }
};
    
/// @brief Parser of WSDL requests
class WSRequest : public Synchronized
{
    WSNameSpace   m_soapNamespace;        ///< Detected SOAP Envelope namespace
    WSNameSpace   m_requestNamespace;     ///< Detected request namespace

protected:
    /// @brief Internal SOAP body processor
    ///
    /// Receives incoming SOAP body of Web Service requests, and returns
    /// application response.
    /// This method is abstract and overwritten in derived generated classes.
    /// @param requestNode sptk::XMLElement*, Incoming and outgoing SOAP element
    virtual void requestBroker(XMLElement* requestNode) THROWS_EXCEPTIONS = 0;

public:
    /// @brief Constructor
    WSRequest() {}

    /// @brief Destructor
    virtual ~WSRequest() {}

    /// @brief Processes incoming requests
    ///
    /// The processing results are stored in the same request XML
    /// @param request sptk::XMLDocument*, Incoming request and outgoing response
    void processRequest(sptk::XMLDocument* request) THROWS_EXCEPTIONS;

    /// @brief Returns service title (for service handshake)
    ///
    /// Application should overwrite this method to return mor appropriate text
    virtual std::string title() const
    {
        return "Generic SPTK WS Request Broker";
    }

    /// @brief Returns service default HTML page
    ///
    /// Application should overwrite this method to return mor appropriate text
    virtual std::string defaultPage() const
    {
        return "index.html";
    }

    /// @brief Returns SOAP envelope namespace
    virtual const WSNameSpace& soapNameSpace()
    {
        SYNCHRONIZED_CODE;
        return m_soapNamespace;
    }

    /// @brief Returns request namespace
    virtual const WSNameSpace& requestNameSpace()
    {
        SYNCHRONIZED_CODE;
        return m_requestNamespace;
    }
};

}
#endif
