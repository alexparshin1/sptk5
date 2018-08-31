/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CThemesGtk.cpp - description                           ║
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

#include <sptk5/gui/CThemeImageCollection.h>
#include <sptk5/cutils>
#include <sptk5/cgui>

#include "ThemeUtils.h"
#include "CGtkThemeLoader.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

using namespace std;
using namespace sptk;

void CThemes::loadGtkButton(xml::Node* imageNode, std::map<CThemeImageState, std::string>& buttonFileNames)
{
    static const Strings buttonStates("NORMAL|ACTIVE|DFRAME|PRELIGHT", "|"); /// DFRAME is a stub

    bool defaultFrame = imageNode->getAttribute("detail", "").str() == "buttondefault";

    String fileName = imageNode->getAttribute("file");
    if (fileName.empty())
        fileName = imageNode->getAttribute("overlay_file").str();

    String state = upperCase(imageNode->getAttribute("state", "NORMAL"));
    //string border = imageNode->getAttribute("border");
    String shadow = upperCase(imageNode->getAttribute("shadow", "OUT"));
    if (shadow == "ETCHED_IN")
        return;
    if (fileName[0] == '/') fileName = fileName.substr(1, 255);
    int buttonState = buttonStates.indexOf(state);
    if (defaultFrame)
        buttonState = THM_DEFAULT_FRAME;
    else if (buttonState > -1 && shadow == "IN")
        buttonState++;
    if (buttonState > -1 && fileName.find(".png") != STRING_NPOS)
        buttonFileNames[CThemeImageState(buttonState)] = m_themeFolder + fileName;
}

void CThemes::loadGtkButtonFileNames(
        xml::Document& xml, string XPath, map<CThemeImageState, string>& buttonFileNames, string orientation)
{
    xml::NodeVector buttonImages;

    buttonFileNames.clear();
    xml.select(buttonImages, XPath);
    for (auto imageNode : buttonImages) {
        if (!orientation.empty() && imageNode->getAttribute("arrow_direction").str() != orientation)
            continue;
        loadGtkButton(imageNode, buttonFileNames);
    }

    if (buttonFileNames[THM_IMAGE_ACTIVE].empty())
        buttonFileNames[THM_IMAGE_ACTIVE] = buttonFileNames[THM_IMAGE_NORMAL];

    if (buttonFileNames[THM_IMAGE_NORMAL_HIGHLITED].empty())
        buttonFileNames[THM_IMAGE_NORMAL_HIGHLITED] = buttonFileNames[THM_IMAGE_NORMAL];

    if (buttonFileNames[THM_IMAGE_ACTIVE_HIGHLITED].empty())
        buttonFileNames[THM_IMAGE_ACTIVE_HIGHLITED] = buttonFileNames[THM_IMAGE_ACTIVE];
}

void CThemes::loadGtkButtons(xml::Document& xml, const String& styleName, CThemeImageCollection& buttons,
                             const String& function)
{
    string XPath("/styles/style[@name='" + styleName + "']/engine[@name='pixmap']/image");
    buttons.loadFromGtkTheme(xml, XPath, "function", function);
}

void CThemes::loadGtkTheme(const String& gtkThemeName)
{
    CGtkThemeParser gtkThemeLoader;
    const String& testThemeName(gtkThemeName);

    try {
        gtkThemeLoader.load(testThemeName);
    }
    catch (...) {
        return;
    }

    m_gtkTheme = true;

    m_themeFolder = gtkThemeLoader.themeFolder();

    Buffer buffer;
    gtkThemeLoader.xml().save(buffer, true);

    try { // Debug
        buffer.saveToFile("/svn/sptk5/trunk/" + testThemeName + ".xml");
    }
    catch (...) {}

    xml::Document& xml = gtkThemeLoader.xml();

    /// Load theme colors
    m_colors.loadFromGtkTheme(xml);

    loadGtkButtons(xml, "button", m_normalButtons, "BOX");
    loadGtkButtons(xml, "checkbutton", m_checkButtons, "CHECK");
    loadGtkButtons(xml, "radiobutton", m_radioButtons, "OPTION");
    loadGtkButtons(xml, "combobutton", m_comboButtons, "BOX");

    loadGtkScrollbars(xml);

    m_progressBar[0].loadFromGtkTheme(xml,
                                      "/styles/style[@name='progressbar']/engine[@name='pixmap']/image[@detail='trough']",
                                      "orientation", "HORIZONTAL");
    m_progressBar[1].loadFromGtkTheme(xml,
                                      "/styles/style[@name='progressbar']/engine[@name='pixmap']/image[@detail='bar']",
                                      "orientation", "HORIZONTAL");

    xml::NodeVector bgImageNodes;
    xml.select(bgImageNodes, "/styles/style/bg_pixmap");
    if (!bgImageNodes.empty()) {
        string fileName = CThemeImageCollection::gtkFullFileName(bgImageNodes[0]->getAttribute("NORMAL"));
        m_background[3] = loadValidatePNGImage(fileName, true);
    }
}
