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

#include "sptk5/HomeDirectory.h"

using namespace std;
using namespace sptk;

String HomeDirectory::location()
{
#ifndef _WIN32
    const char* hdir = std::getenv("HOME");
    if (hdir == nullptr)
        hdir = ".";
    String homeDir = trim(hdir);
    if (homeDir.empty())
        homeDir = ".";
    homeDir += "/";
#else
    char* hdrive = getenv("HOMEDRIVE");
    char* hdir = getenv("HOMEPATH");
    if (!hdir && !hdrive)
    {
        const char* wdir = getenv("WINDIR");
        if (wdir == nullptr)
            return "C:\\";
        return String(wdir) + String("\\");
    }

    string homeDrive;
    string homeDir;
    if (hdrive)
        homeDrive = hdrive;
    if (hdir)
        homeDir = hdir;
    if (homeDir == "\\")
        homeDir = homeDrive + "\\";
    else
        homeDir = homeDrive + homeDir;
    homeDir += "\\Local Settings\\Application Data\\Programs\\";
#endif

    return homeDir;
}
