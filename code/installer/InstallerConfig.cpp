/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       InstallerConfig.cpp - installer configuration          ║
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

#include "InstallerConfig.h"

#include <sptk5/Buffer.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;

void InstallerConfig::load(const filesystem::path& configFile)
{
    Buffer buf;
    buf.loadFromFile(configFile);

    xdoc::Document doc;
    doc.load(buf);
    const auto& root = doc.root();

    application = root->getString("application");
    version = root->getString("version");
    description = root->getString("description");
    installDirectory = root->getString("install_directory");
    sidebarImage = root->getString("sidebar_image");

    for (const auto& optNode: root->nodes("options"))
    {
        InstallOption opt;
        opt.name = optNode->getString("name");
        opt.value = optNode->getString("value");
        options.push_back(opt);
    }

    auto pkgNode = root->findFirst("packages");
    if (pkgNode)
    {
        for (const auto& child: pkgNode->nodes())
            packages[string(child->getName())] = child->getString();
    }
}
