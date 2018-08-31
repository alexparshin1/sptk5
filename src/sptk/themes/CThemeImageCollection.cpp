/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CThemeImageCollection.cpp - description                ║
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

#include <sptk5/sptk.h>

#include "ThemeUtils.h"
#include <sptk5/gui/CThemes.h>

using namespace std;
using namespace sptk;

CThemeImageCollection::CThemeImageCollection() noexcept
{
    m_stretch = true;
    for (unsigned i = 0; i < MAX_IMAGE_STATES; i++) {
        m_images[i] = nullptr;
        m_overlayImages[i] = nullptr;
    }
}

void CThemeImageCollection::clear()
{
    for (unsigned i = 0; i < MAX_IMAGE_STATES; i++) {
        if (m_images[i]) {
            delete m_images[i];
            m_images[i] = nullptr;
        }
        if (m_overlayImages[i]) {
            delete m_overlayImages[i];
            m_overlayImages[i] = nullptr;
        }
    }
    m_border[0] = 8;
    m_border[1] = 8;
    m_border[2] = 8;
    m_border[3] = 8;
}

CPngImage* CThemeImageCollection::image(CThemeImageState state) const
{
    if (m_images[state])
        return m_images[state];
    int bstate = state;
    if (bstate & THMF_HIGHLIGHTED)
        bstate -= THMF_HIGHLIGHTED;
    if (m_images[bstate])
        return m_images[bstate];
    return m_images[0];
}

CPngImage* CThemeImageCollection::overlayImage(CThemeImageState state) const
{
    if (m_overlayImages[state])
        return m_overlayImages[state];
    int bstate = state;
    if (bstate & THMF_HIGHLIGHTED)
        bstate -= THMF_HIGHLIGHTED;
    if (m_overlayImages[bstate])
        return m_overlayImages[bstate];
    return m_overlayImages[0];
}

void CThemeImageCollection::loadFromSptkTheme(const Strings& objectNames)
{
    clear();
    for (unsigned i = 0; i < objectNames.size() && i < MAX_IMAGE_STATES; i++) {
        if (!objectNames[i].empty())
            m_images[i] = loadValidatePNGImage(objectNames[i] + ".png", false);
    }
}

string CThemeImageCollection::gtkFullFileName(string fileName)
{
    if (fileName.empty())
        return fileName;
    if (fileName[0] == '/') fileName = fileName.substr(1, 255);
    return CThemes::themeFolder() + fileName;
}

void CThemeImageCollection::loadFromGtkTheme(
        xml::Document& gtkTheme, const String& imagesXPath, const String& attribute,
        const String& attributeValue)
{
    static const Strings buttonStates("NORMAL|ACTIVE|PRELIGHT", "|");

    xml::NodeVector images;

    gtkTheme.select(images, imagesXPath);
    bool borderInitted = false;
    string normalImageFileName;
    string normalOverlayFileName;
    for (auto imageNode : images) {
        if (!attribute.empty() && imageNode->getAttribute(attribute).str() != attributeValue)
            continue;

        bool defaultFrame = imageNode->getAttribute("detail", "").str() == "buttondefault";

        string fileName = gtkFullFileName(imageNode->getAttribute("file"));
        string overlayFileName = gtkFullFileName(imageNode->getAttribute("overlay_file"));

        String state = upperCase(imageNode->getAttribute("state", "NORMAL"));
        int buttonState = buttonStates.indexOf(state);

        if (normalImageFileName.empty() && (state == "NORMAL" || state == "ACTIVE"))
            normalImageFileName = fileName;

        if (normalOverlayFileName.empty() && (state == "NORMAL" || state == "ACTIVE"))
            normalOverlayFileName = overlayFileName;

        if (!borderInitted) {
            m_stretch = imageNode->getAttribute("stretch").str() == "TRUE";
            string border = imageNode->getAttribute("border", "{ 0, 0, 0, 0 }");
            size_t pos1 = border.find('{');
            size_t pos2 = border.find('}');
            if (pos1 != STRING_NPOS && pos2 != STRING_NPOS) {
                pos1++;
                border = border.substr(pos1, pos2 - pos1);
                Strings borderStrs(border, ",");
                for (unsigned i = 0; i < 4 && i < borderStrs.size(); i++)
                    m_border[i] = string2int(borderStrs[i]);
            }
            borderInitted = true;
        }

        String shadow = upperCase(imageNode->getAttribute("shadow", "OUT"));
        if (shadow == "ETCHED_IN")
            continue;

        if (defaultFrame)
            buttonState = THM_DEFAULT_FRAME;
        else if (buttonState > -1 && shadow == "IN")
            buttonState |= THMF_ACTIVE;

        if (buttonState > -1) {
            m_images[CThemeImageState(buttonState)] = nullptr;
            m_overlayImages[CThemeImageState(buttonState)] = nullptr;
            if (!fileName.empty() && fileName.find(".png") != STRING_NPOS)
                m_images[CThemeImageState(buttonState)] = loadValidatePNGImage(fileName, true);
            if (!overlayFileName.empty() && overlayFileName.find(".png") != STRING_NPOS)
                m_overlayImages[CThemeImageState(buttonState)] = loadValidatePNGImage(overlayFileName, true);
        }
    }

    if (!m_images[THM_IMAGE_NORMAL] && !normalImageFileName.empty())
        m_images[THM_IMAGE_NORMAL] = loadValidatePNGImage(normalImageFileName, true);

    if (!m_overlayImages[THM_IMAGE_NORMAL] && !normalOverlayFileName.empty())
        m_overlayImages[THM_IMAGE_NORMAL] = loadValidatePNGImage(normalOverlayFileName, true);
}
