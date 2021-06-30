/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>

#include <FL/fl_draw.H>
#include <sptk5/gui/CPngImage.h>
#include <sptk5/gui/CIcon.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

using namespace std;
using namespace sptk;

void CIcon::load(const Buffer& imageData)
{
    if (m_image)
    {
        m_image->load(imageData);
    }
    else
    {
        m_image = new CPngImage(imageData);
        if (!m_image->data())
        {
            delete m_image;
            m_image = nullptr;
        }
    }
}

void CIconMap::clear()
{
    for (auto itor: *this)
    {
        CIcon* icon = itor.second;
        if (m_shared)
        {
            delete icon;
        }
        else if (!icon->m_shared)
        {
            delete icon;
        }
    }
    map<String, CIcon*, CaseInsensitiveCompare>::clear();
}

void CIconMap::insert(CIcon* icon)
{
    auto it = find(icon->name());
    if (it != end())
    {
        CIcon* oldIcon = it->second;
        delete oldIcon;
        it->second = icon;
    }
    else
    {
        (*this)[icon->name()] = icon;
    }
}

void CIconMap::load(Tar& tar, xml::Node* iconsNode)
{
    for (auto node: *iconsNode)
    {
        if (node->name() != "icon")
        {
            continue;
        }
        String iconName = (String) node->getAttribute("name");
        String fileName = (String) node->getAttribute("image");
        if (iconName.empty())
        {
            continue;
        }
        try
        {
            const Buffer& imageData = tar.file(fileName);
            CIcon* icon = nullptr;
            auto ftor = find(iconName);
            if (ftor == end())
            {
                // Icon not found, adding new one
                icon = new CIcon(iconName);
                insert(icon);
            }
            else
            {
                icon = ftor->second;
            }
            icon->load(imageData);
            if (!icon->image())
            {
                throw Exception("Can't load " + fileName);
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
        }
    }
}
