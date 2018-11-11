/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CThemeColorCollection.cpp - description                ║
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

#include <FL/Fl.H>

#include <sptk5/gui/CThemeColorCollection.h>

using namespace std;
using namespace sptk;

std::map<String, gtk_color_function>*   CThemeColorCollection::m_gtkColorFunctionMap;
std::map<String, Fl_Color>              CThemeColorCollection::m_colorMap;

CThemeColorCollection::CThemeColorCollection() noexcept
{
    if (!m_gtkColorFunctionMap)
    try {
        m_gtkColorFunctionMap = new std::map<String, gtk_color_function>;
        (*m_gtkColorFunctionMap)["darker"] = darker;
        (*m_gtkColorFunctionMap)["lighter"] = lighter;
        (*m_gtkColorFunctionMap)["shade"] = shade;
        (*m_gtkColorFunctionMap)["mix"] = mix;
    }
    catch (...) {}
}

static Fl_Color colorFromHexString(const String& colorStr)
{
    char* eptr;
    auto rgbColor = (uint32_t) strtol(colorStr.c_str(), &eptr, 16);

    if (colorStr.length() == 3) {
        uint32_t blue, green, red;
        blue = rgbColor % 16 * 16;
        rgbColor /= 16;
        green = rgbColor % 16 * 16;
        rgbColor /= 16;
        red = rgbColor % 16 * 16;
        rgbColor = (red * 256 + green) * 256 + blue;
    }

    return (Fl_Color) (rgbColor << 8);
}

static void splitArguments(const String& expression, Strings& arguments)
{
    int subExpressionLevel = 0;

    const char* ptr1 = expression.c_str();
    const char* ptr2 = nullptr;
    while ((ptr2 = strpbrk(ptr1, "(,"))) {
        if (*ptr2 == '(') {
            const char* ptr = ptr2 + 1;
            subExpressionLevel++;
            while (subExpressionLevel) {
                ptr2 = strpbrk(ptr, "()");
                if (ptr2 == nullptr)
                    break;
                if (*ptr2 == '(')
                    subExpressionLevel++;
                else
                    subExpressionLevel--;
                ptr = ptr2 + 1;
            }
            ptr2 = strpbrk(ptr, ",");
            if (!ptr2)
                break;
        }

        String parts(ptr1, ptr2 - ptr1, 0);
        parts = trim(parts);
        if (parts[0] == '\"')
            parts = parts.substr(1, parts.length() - 2);
        arguments.push_back(parts);

        ptr1 = ptr2 + 1;
    }

    arguments.push_back(ptr1);
}

static void splitExpression(const String& colorValue, String& function, String& arguments)
{
    size_t pos1 = colorValue.find('(');
    size_t pos2 = colorValue.rfind(')');
    if (pos1 != STRING_NPOS && pos2 != STRING_NPOS) {
        function = trim(colorValue.substr(0, pos1 - 1));
        arguments = trim(colorValue.substr(pos1 + 1, pos2 - pos1 - 1));
    }
}

Fl_Color CThemeColorCollection::passby(const String& expression)
{
    return gtkColorFunction(expression);
}

Fl_Color CThemeColorCollection::lighter(const String& expression)
{
    return fl_lighter(gtkColorFunction(expression));
}

Fl_Color CThemeColorCollection::darker(const String& expression)
{
    return fl_darker(gtkColorFunction(expression));
}

Fl_Color CThemeColorCollection::mix(const String& expression)
{
    Strings args;
    splitArguments(expression, args);
    double weight = string2double(args[0]);
    Fl_Color color1 = gtkColorFunction(args[1]);
    Fl_Color color2 = gtkColorFunction(args[2]);
    return fl_color_average(color1, color2, float(weight));
}

static unsigned char shadeColorComponent(unsigned colorComponent, double multiplier)
{
    colorComponent = unsigned(colorComponent * multiplier);
    if (colorComponent > 255)
        colorComponent = 255;
    return (unsigned char) colorComponent;
}

Fl_Color CThemeColorCollection::shade(const String& expression)
{
    Strings args;
    splitArguments(expression, args);
    if (args.size() < 2)
        return fl_darker(gtkColorFunction(expression));
    double multiplier = string2double(args[0]);
    if (multiplier < 0.001)
        multiplier = 1;

    Fl_Color color = gtkColorFunction(args[1]);
    unsigned char r = shadeColorComponent((unsigned char) (color >> 24), multiplier);
    unsigned char g = shadeColorComponent((unsigned char) (color >> 16), multiplier);
    unsigned char b = shadeColorComponent((unsigned char) (color >> 8), multiplier);

    return fl_rgb_color(r, g, b);
}

Fl_Color CThemeColorCollection::gtkColorFunction(const String& expression)
{
    String function, arguments;
    String colorValue(trim(expression));
    switch (colorValue[0]) {
        case '#': // Direct color value
            return colorFromHexString(colorValue.c_str() + 1);
        case '@': // Color from color map
            return m_colorMap[colorValue.c_str() + 1];
        default:  // Color function
            splitExpression(colorValue, function, arguments);
            if (function.empty()) {
                cerr << "Color expression '" << expression << "' isn't understood" << endl;
                return FL_BLACK;
            }
            auto itor = m_gtkColorFunctionMap->find(function);
            if (itor == m_gtkColorFunctionMap->end()) {
                cerr << "Color function '" << function << "' isn't supported" << endl;
                return passby(arguments);
            } else {
                gtk_color_function cfunction = itor->second;
                return cfunction(arguments);
            }
    }
}

void CThemeColorCollection::loadColor(xml::Node* colorNode, CThemeColorIndex colorIndex)
{
    static const Strings colorStateNames("NORMAL,PRELIGHT,SELECTED,ACTIVE,INSENSITIVE", ",");
    auto itor = colorNode->attributes().begin();
    for (; itor != colorNode->attributes().end(); ++itor) {
        xml::Node* colorStateNode = *itor;
        CThemeColorState colorState = (CThemeColorState) colorStateNames.indexOf(colorStateNode->name());
        if (colorState == THM_COLOR_UNDEFINED)
            continue;
        Fl_Color color = gtkColorFunction(colorStateNode->value());
        m_colors[colorIndex][colorState] = color;
    }
}

static const char* colorNames[THM_MAX_COLOR_INDEX] = {"fg", "bg", "base", "text"};

void CThemeColorCollection::loadFromSptkTheme(xml::Document& sptkTheme)
{
    loadColorMap(sptkTheme, "/color_scheme");
    for (unsigned colorIndex = 0; colorIndex < THM_MAX_COLOR_INDEX; colorIndex++) {
        string colorXPath = string("/color_scheme/") + colorNames[colorIndex];
        xml::NodeVector colorNodes;
        sptkTheme.select(colorNodes, colorXPath);
        if (colorNodes.size() == 1) {
            xml::Node* colorNode = *(colorNodes.begin());
            loadColor(colorNode, CThemeColorIndex(colorIndex));
        }
    }
}

void CThemeColorCollection::loadFromGtkTheme(xml::Document& gtkTheme)
{
    loadColorMap(gtkTheme, "/gtk_color_scheme");

    string stylesXPath = "/styles/style";
    xml::NodeVector styleNodes;
    gtkTheme.select(styleNodes, stylesXPath);
    xml::Node* defaultStyleNode = *styleNodes.begin();
    for (auto styleNode : styleNodes) {
        String styleName(styleNode->getAttribute("name").str());
        if (styleName == "default" || styleName.find("-default") != STRING_NPOS) {
            defaultStyleNode = styleNode;
            break;
        }
    }

    for (unsigned colorIndex = 0; colorIndex < THM_MAX_COLOR_INDEX; colorIndex++) {
        String colorXPath(colorNames[colorIndex]);
        xml::NodeVector colorNodes;
        defaultStyleNode->select(colorNodes, colorXPath);
        size_t elements = colorNodes.size();
        if (elements == 1) {
            xml::Node* colorNode = *(colorNodes.begin());
            loadColor(colorNode, CThemeColorIndex(colorIndex));
        }
    }

    Fl::set_color(FL_FOREGROUND_COLOR, fgColor(THM_COLOR_NORMAL));
    Fl::set_color(FL_BACKGROUND_COLOR, bgColor(THM_COLOR_NORMAL));
    Fl::set_color(FL_BACKGROUND2_COLOR, baseColor(THM_COLOR_NORMAL));
    Fl::set_color(FL_INACTIVE_COLOR, fgColor(THM_COLOR_INSENSITIVE));
    Fl::set_color(FL_SELECTION_COLOR, bgColor(THM_COLOR_SELECTED));
}

void CThemeColorCollection::loadColorMap(xml::Document& gtkTheme, const String& colorMapXPath)
{
    m_colorMap.clear();

    xml::NodeVector colorMapNodes;
    gtkTheme.select(colorMapNodes, colorMapXPath);
    if (colorMapNodes.empty())
        return;

    xml::Node* colorMapNode = *(colorMapNodes.begin());

    Strings colorMapStrings(colorMapNode->getAttribute("colors"), "\\n");

    for (const auto& colorMapString : colorMapStrings) {
        Strings colorInfo(colorMapString, ":#");
        if (colorInfo.size() != 2)
            continue;
        m_colorMap[colorInfo[0]] = colorFromHexString(colorInfo[1]);
    }
}
