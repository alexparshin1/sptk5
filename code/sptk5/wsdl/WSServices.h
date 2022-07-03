/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/wsdl/WSRequest.h>

namespace sptk {

/**
 * Collection of references to available services
 */
class SP_EXPORT WSServices
{
public:
    /**
     * Constructor
     *
     * Registers default service for empty location
     * @param defaultService    Default service
     */
    explicit WSServices(const SWSRequest& defaultService);

    /**
     * Destructor
     */
    ~WSServices() = default;

    /**
     * Copy constructor
     *
     * Registers default service for empty location
     * @param other             Other services object
     */
    WSServices(const WSServices& other);

    /**
     * Set a service for given location
     * @param location          Path to the resource, not including resource name
     * @param service           Service that is processing that location requests
     */
    void set(const sptk::String& location, const SWSRequest& service);

    /**
     * Get service for given location that is processing that location requests
     * @param location          Path to the resource, not including resource name
     * @return matching service, or default service if there is no match
     */
    WSRequest& get(const sptk::String& location) const;

private:
    mutable std::mutex m_mutex;
    std::map<String, SWSRequest> m_services;

    void assign(const WSServices& other);
};

} // namespace sptk
