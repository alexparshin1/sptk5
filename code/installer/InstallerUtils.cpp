/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       InstallerUtils.cpp - installer helper functions        ║
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

#include "InstallerUtils.h"

#include <array>
#include <cstdio>
#include <map>

using namespace std;
using namespace sptk;

String htmlEscape(const String& text)
{
    String escaped;
    for (char ch: text)
    {
        switch (ch)
        {
            case '&':
                escaped += "&amp;";
                break;
            case '<':
                escaped += "&lt;";
                break;
            case '>':
                escaped += "&gt;";
                break;
            default:
                escaped += ch;
                break;
        }
    }
    return escaped;
}

filesystem::path normalizeDirectory(const String& directory)
{
    String name = directory.trim();
    while (name.length() > 1 && (name.back() == '/' || name.back() == '\\'))
        name.pop_back();
    if (name.empty())
        return {};
    return filesystem::path(name.c_str()).lexically_normal();
}

filesystem::path existingAncestor(const filesystem::path& directory)
{
    error_code ec;
    for (filesystem::path path = directory; !path.empty(); path = path.parent_path())
    {
        if (filesystem::is_directory(path, ec))
            return path;
        if (path.parent_path() == path)
            break;
    }
    return {};
}

bool isDirectoryWritable(const filesystem::path& directory)
{
    static map<string, bool> probeCache;
    static unsigned          probeIndex = 0;

    auto [itor, inserted] = probeCache.try_emplace(directory.string(), false);
    if (!inserted)
        return itor->second;

    error_code ec;
    auto       probe = directory / (".spinst_write_test." + to_string(++probeIndex));
    if (filesystem::create_directory(probe, ec))
    {
        filesystem::remove(probe, ec);
        itor->second = true;
    }
    return itor->second;
}

String formatByteSize(uintmax_t bytes)
{
    static constexpr array units {"bytes", "KB", "MB", "GB", "TB"};

    auto   value = static_cast<double>(bytes);
    size_t unit = 0;
    while (value >= 1024 && unit < units.size() - 1)
    {
        value /= 1024;
        unit++;
    }

    char buffer[64];
    snprintf(buffer, sizeof(buffer), unit == 0 ? "%.0f %s" : "%.1f %s", value, units[unit]);
    return String(buffer);
}
