/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       InstallerConfig.h - installer configuration            ║
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

#include <sptk5/Strings.h>

#include <filesystem>
#include <map>
#include <vector>

/**
 * @brief A single installation option, as defined in the configuration file
 */
struct InstallOption
{
    sptk::String name;
    sptk::String value;
};

/**
 * @brief Installer configuration.
 *
 * Besides the data loaded from the configuration file, it carries the choices
 * made by the user: installDirectory is overwritten by DirectoryPage, and
 * selectedOptions is filled in by OptionsPage.
 */
struct InstallerConfig
{
    sptk::String                         application {"Application"};
    sptk::String                         version {"1.0.0"};
    sptk::String                         description;
    sptk::String                         installDirectory {"/opt/app"};
    sptk::String                         sidebarImage;
    std::vector<InstallOption>           options;
    std::map<sptk::String, sptk::String> packages;

    /**
     * @brief Options picked on the options page, filled in by the wizard
     */
    sptk::Strings selectedOptions;

    /**
     * @brief Loads the configuration from a JSON file
     * @param configFile        Configuration file name
     */
    void load(const std::filesystem::path& configFile);
};
