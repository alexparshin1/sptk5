/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       InstallerConfig.h - installer configuration            ║
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

#include <sptk5/cutils>

namespace sptk {

class DownloadPackages
{
public:
    /**
     * @brief Constructor.
     * @param downloadUrl Download URL.
     */
    DownloadPackages(const std::string& downloadUrl = "https://www.sptk.net/download");

    /**
     * @brief Destructor.
     */
    virtual ~DownloadPackages() = default;

private:
    /**
     * @brief Get current OS version.
     * @return OS version.
     */
    std::string getOsVersion() const;

    /**
     * @brief Get list of packages for the current OS version.
     * @return List of packages.
     */
    std::string getOsVersion() const;

    std::string& m_downloadUrl;

    const std::string m_getOsVersionNamesScript = "";
};

} // namespace sptk
