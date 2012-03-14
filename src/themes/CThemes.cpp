/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CThemes.cpp  -  description
                             -------------------
    begin                : Fri Nov 08 2003
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <FL/fl_draw.h>
#include <FL/Enumerations.h>
#include <sptk5/sptk.h>

#include <sptk5/gui/CThemeColorCollection.h>
#include <sptk5/CDirectoryDS.h>
#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CThemes.h>
#include <sptk5/gui/CTreeControl.h>
#include <sptk5/gui/CPngImage.h>
#include <sptk5/CRegistry.h>
#include <sptk5/string_ext.h>

#include <sptk5/gui/default_icons.h>

#include <iostream>
#include <sstream>
#include <FL/Fl_Image.H>

#include "ThemeUtils.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

using namespace std;
using namespace sptk;

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);

string CThemes::m_name;
CFontsVector CThemes::screenFonts;
CFontsMap CThemes::m_fonts;
CFrames CThemes::m_frames;
CTabImages CThemes::m_tabImages;

Fl_Box_Draw_F *CThemes::m_flatBox;
Fl_Box_Draw_F *CThemes::m_thinUpBox;
Fl_Box_Draw_F *CThemes::m_upBox;
Fl_Box_Draw_F *CThemes::m_thinDownBox;
Fl_Box_Draw_F *CThemes::m_downBox;

Fl_Box_Draw_F *CThemes::m_thinUpFrame;
Fl_Box_Draw_F *CThemes::m_upFrame;
Fl_Box_Draw_F *CThemes::m_thinDownFrame;
Fl_Box_Draw_F *CThemes::m_downFrame;

CPngImage *CThemes::m_background[7];
CThemeImageCollection CThemes::m_progressBar[2];

CThemeImageCollection CThemes::m_normalButtons;
CThemeImageCollection CThemes::m_comboButtons;
CThemeImageCollection CThemes::m_checkButtons;
CThemeImageCollection CThemes::m_radioButtons;
CThemeColorCollection CThemes::m_colors;
bool CThemes::m_flatButtons;
bool CThemes::m_gtkTheme;

CXmlDoc *CThemes::m_registry;
CTar CThemes::m_tar;
CIconMap CThemes::m_icons[4]; /// Four different sets of icons
bool CThemes::m_desaturateInactiveButtons;
int CThemes::m_buttonFocusRadius;
string CThemes::m_themeFolder;

CThemes themes;

CThemes::CThemes() {
    m_flatBox = Fl::get_boxtype(FL_FLAT_BOX);
    m_thinUpBox = Fl::get_boxtype(FL_THIN_UP_BOX);
    m_upBox = Fl::get_boxtype(FL_UP_BOX);
    m_thinDownBox = Fl::get_boxtype(FL_THIN_DOWN_BOX);
    m_downBox = Fl::get_boxtype(FL_DOWN_BOX);

    m_thinUpFrame = Fl::get_boxtype(FL_THIN_UP_FRAME);
    m_upFrame = Fl::get_boxtype(FL_UP_FRAME);
    m_thinDownFrame = Fl::get_boxtype(FL_THIN_DOWN_FRAME);
    m_downFrame = Fl::get_boxtype(FL_DOWN_FRAME);

    m_registry = new CXmlDoc;
    m_desaturateInactiveButtons = false;
    m_buttonFocusRadius = 0;

    for (unsigned i = 0; i < 7; i++)
        m_background[i] = 0L;

    if (!screenFonts.size()) {
        int k = Fl::set_fonts("*");
        for (int i = 0; i < k; i++) {
            int attributes = 0;
            string fontName = Fl::get_font_name((Fl_Font) i, &attributes);
            CFont* font = new CFont(fontName, 10, 0, i, attributes);
            screenFonts.push_back(font);
        }
    }

    set("Default");
}

CThemes::~CThemes() {
    if (m_registry)
        delete m_registry;
}

CIcon* CThemes::getIcon(std::string iconName, CIconSize iconSize) {
    CIconMap::iterator itor = m_icons[iconSize].find(iconName);
    if (itor == m_icons[iconSize].end()) {
        cerr << "Can't find icon '" << iconName << "' for size " << iconSize << endl;
        return 0L;
    }
    return itor->second;
}

CPngImage* CThemes::getIconImage(std::string iconName, CIconSize iconSize) {
    CIcon* icon = getIcon(iconName, iconSize);
    if (icon)
        return icon->image();
    return 0;
}

void CThemes::registerIcon(CIcon* icon, CIconSize iconSize) {
    m_icons[iconSize].insert(icon);
}

const CStrings& CThemes::searchDirectories() {
    static CStrings sd;
    if (sd.size())
        return sd;
#ifdef _WIN32
    char windir[128];
    GetWindowsDirectory(windir, 128);
    sd.push_back(windir + string("\\.themes.sptk"));
#else
    sd.push_back("/usr/share/sptk");
    sd.push_back("/usr/local/share/sptk");
#endif

    return sd;
}

void CThemes::reset() {
    m_gtkTheme = false;
    m_flatButtons = false;

    Fl::set_boxtype(FL_FLAT_BOX, m_flatBox, 0, 0, 0, 0);
    Fl::set_boxtype(FL_THIN_UP_BOX, m_thinUpBox, 1, 1, 2, 2);
    Fl::set_boxtype(FL_UP_BOX, m_upBox, 2, 2, 4, 4);
    Fl::set_boxtype(FL_THIN_DOWN_BOX, m_thinDownBox, 1, 1, 2, 2);
    Fl::set_boxtype(FL_DOWN_BOX, m_downBox, 2, 2, 4, 4);

    Fl::set_boxtype(FL_THIN_UP_FRAME, m_thinUpFrame, 1, 1, 2, 2);
    Fl::set_boxtype(FL_UP_FRAME, m_upFrame, 2, 2, 4, 4);
    Fl::set_boxtype(FL_THIN_DOWN_FRAME, m_thinDownFrame, 1, 1, 2, 2);
    Fl::set_boxtype(FL_DOWN_FRAME, m_downFrame, 2, 2, 4, 4);

    CBuffer defaultThemeBuffer(default_icons, default_icons_len);
    m_tar.read(defaultThemeBuffer);
    m_registry->load(m_tar.file("theme.ini"));

    m_normalButtons.clear();
    m_comboButtons.clear();
    m_radioButtons.clear();
    m_checkButtons.clear();

    m_scrollBar[THM_SCROLLBAR_VERTICAL].clear();
    m_scrollBar[THM_SCROLLBAR_HORIZONTAL].clear();

    m_icons[IS_COMBO_ICON].clear();
    m_icons[IS_SMALL_ICON].clear();
    m_icons[IS_LARGE_ICON].clear();

    m_themeFolder = "";

    m_scrollBarTroughBorder = 1;
    m_scrollBarSliderWidth = 14;
    m_scrollBarStepperSize = 14;
}

CThemeImageCollection& CThemes::sizeToButtonImages(CThemeButtonType sz) {
    switch (sz) {
        case THM_BUTTON_COMBO:
            return m_comboButtons;
        case THM_BUTTON_CHECK:
            return m_checkButtons;
        case THM_BUTTON_RADIO:
            return m_radioButtons;
        case THM_BUTTON_NORMAL:
            break;
    }
    return m_normalButtons;
}

void CThemes::replaceImage(CPngImage *images[], int ndx, string fileName) {
    if (images[ndx])
        delete images[ndx];
    try {
        images[ndx] = loadValidatePNGImage(fileName);
    } catch (...) {
        images[ndx] = 0;
    }
}

void CThemes::set(string theThemeName) {
    string themeName(theThemeName);
    if (themeName == "" || themeName.find("GTK:") == 0)
        themeName = "Default";

    if (theThemeName == m_name)
        return;

    reset();

    const CStrings& dirs = searchDirectories();
    bool defaultTheme = false;
    for (unsigned dn = 0; dn < dirs.size() && !defaultTheme; dn++) {
        string fileName = dirs[dn] + "/" + themeName + ".tar";

        defaultTheme = lowerCase(themeName) == "default";
        m_registry->clear();

        m_desaturateInactiveButtons = false;
        int scrollBarButtonSize = 0;
        if (!defaultTheme) {
            try {
                m_tar.read(fileName);
                m_registry->load(m_tar.file("theme.ini"));
                m_name = themeName;
            } catch (...) {
                continue;
            }
        } else {
            CBuffer defaultThemeBuffer(default_icons, default_icons_len);
            m_tar.read(defaultThemeBuffer);
            m_registry->load(m_tar.file("theme.ini"));
            m_name = themeName;
        }

        try {
            CXmlNode::iterator itor = m_registry->begin();
            for (; itor != m_registry->end(); itor++) {
                CXmlNode* iconsNode = *itor;
                if (iconsNode->name() != "icons")
                    continue;
                string iconsSizeStr = iconsNode->getAttribute("size", "large") + " ";
                CIconSize iconsSize;
                switch (iconsSizeStr[0]) {
                    case 'c':
                        iconsSize = IS_COMBO_ICON;
                        break;
                    case 's':
                        iconsSize = IS_SMALL_ICON;
                        break;
                    case 'd':
                        iconsSize = IS_DIALOG_ICON;
                        break;
                    default:
                        iconsSize = IS_LARGE_ICON;
                        break;
                }
                m_icons[iconsSize].load(m_tar, iconsNode);
            }

            CXmlNode* buttonsNode = m_registry->findOrCreate("buttons", false);
            m_desaturateInactiveButtons = buttonsNode->getAttribute("DesaturateInactive", "N");
            m_buttonFocusRadius = buttonsNode->getAttribute("FocusRadius", "0");
            m_flatButtons = buttonsNode->getAttribute("FlatInactive", "N");

            CXmlNode* tabsNode = m_registry->findOrCreate("tabs", false);
            m_tabImages.load(m_tar, tabsNode);

            CXmlNode* fontsTopic = m_registry->findOrCreate("fonts", false);
            m_fonts.clear();
            for (itor = fontsTopic->begin(); itor != fontsTopic->end(); itor++) {
                CXmlNode* fontInfo = *itor;
                string fontName = fontInfo->getAttribute("name", "Arial");
                CFont* screenFont = screenFonts.find(fontName);
                if (!screenFont)
                    continue;
                string object = fontInfo->getAttribute("object", "Arial");
                CFont* font = new CFont(fontName,
                        fontInfo->getAttribute("size", "10"),
                        fontInfo->getAttribute("color", "0"),
                        screenFont->index(),
                        screenFont->attributes()
                        );
                m_fonts[object] = font;
            }
            CFontsMap::iterator ftor = m_fonts.find("default");
            if (ftor == m_fonts.end()) {
                CFont* font = screenFonts[0];
                m_fonts["default"] = new CFont(font->name(), 10, 0, font->index(), font->attributes());
            }

            CXmlNode* framesNode = m_registry->findOrCreate("frames", false);
            m_frames.load(m_tar, framesNode);

        } catch (...) {
        }

        m_colors.loadFromSptkTheme(*m_registry);

        unsigned i;
        for (i = 0; i < 7; i++)
            replaceImage(m_background, i, "background" + int2string(i) + ".png");

        m_progressBar[0].loadFromSptkTheme(CStrings("progress0",","));
        m_progressBar[1].loadFromSptkTheme(CStrings("progress1",","));

        m_normalButtons.loadFromSptkTheme(CStrings("button0,button1,button2,button3,button4",","));
        m_comboButtons.loadFromSptkTheme(CStrings("combo_button0,combo_button1,combo_button2,combo_button3",","));
        m_checkButtons.loadFromSptkTheme(CStrings("check_button0,check_button1,check_button2,check_button3",","));
        m_radioButtons.loadFromSptkTheme(CStrings("radio_button0,radio_button1,radio_button2,radio_button3",","));

        CTreeItem::treeOpened = getIconImage("tree_opened", IS_SMALL_ICON); ///< Default image of the opened tree
        CTreeItem::treeClosed = getIconImage("tree_closed", IS_SMALL_ICON); ///< Default image of the closed tree
        CTreeItem::folderOpened = getIconImage("folder_opened", IS_SMALL_ICON); ///< Default image of the opened floder
        CTreeItem::folderClosed = getIconImage("folder_closed", IS_SMALL_ICON); ///< Default image of the closed floder
        CTreeItem::document = getIconImage("document", IS_SMALL_ICON); ///< Default image of the document
        if (!CTreeItem::folderOpened)
            CTreeItem::folderOpened = CTreeItem::treeOpened;
        if (!CTreeItem::folderClosed)
            CTreeItem::folderClosed = CTreeItem::treeClosed;

        try {
            CXmlNode& topic = *m_registry->findFirst("scrollbars", false);
            if (&topic)
                scrollBarButtonSize = topic.getAttribute("ButtonSize", "16");
        } catch (...) {}

        m_scrollBar[THM_SCROLLBAR_VERTICAL].load(THM_SCROLLBAR_VERTICAL, m_desaturateInactiveButtons);
        m_scrollBar[THM_SCROLLBAR_HORIZONTAL].load(THM_SCROLLBAR_HORIZONTAL, m_desaturateInactiveButtons);

        Fl::set_boxtype(FL_FLAT_BOX, drawFlatBox, 0, 0, 0, 0);

        Fl_Boxtype stdBoxes[] = {FL_UP_BOX, FL_THIN_UP_BOX, FL_DOWN_BOX, FL_THIN_DOWN_BOX};
        Fl_Boxtype stdFrames[] = {FL_UP_FRAME, FL_THIN_UP_FRAME, FL_DOWN_FRAME, FL_THIN_DOWN_FRAME};
        Fl_Box_Draw_F * boxFuncs[] = {drawUpBox, drawThinUpBox, drawDownBox, drawThinDownBox};
        Fl_Box_Draw_F * frameFuncs[] = {drawUpFrame, drawThinUpFrame, drawDownFrame, drawThinDownFrame};
        int defaultFrameWidth[] = {2, 1, 2, 1};
        for (i = 0; i < 4; i++) {
            Fl_Boxtype frameType = stdFrames[i];
            int frameWidth = defaultFrameWidth[i];
            CFrame* frame = m_frames.find(frameType);
            if (frame) {
                frameWidth = frame->frameWidth();
                Fl::set_boxtype(frameType, frameFuncs[i], frameWidth, frameWidth, frameWidth * 2, frameWidth * 2);
            }
            Fl::set_boxtype(stdBoxes[i], boxFuncs[i], frameWidth, frameWidth, frameWidth * 2, frameWidth * 2);
        }

        break;
    }

    if (theThemeName.find("GTK:") == 0)
        loadGtkTheme(theThemeName.substr(4,255));
}

CPngImage *CThemes::imageForColor(Fl_Color clr) {
    if (clr >= FL_DARK3 && clr <= FL_LIGHT3) {
        int ndx = 0;
        switch (clr) {
            case FL_DARK3:
                break;
            case FL_DARK2:
                ndx = 1;
                break;
            case FL_DARK1:
                ndx = 2;
                break;
            case FL_BACKGROUND_COLOR:
                ndx = 3;
                break;
            case FL_LIGHT1:
                ndx = 4;
                break;
            case FL_LIGHT2:
                ndx = 5;
                break;
            case FL_LIGHT3:
                ndx = 6;
                break;
            default:
                return 0;
        }
        return m_background[ndx];
    }
    return 0L;
}

void CThemes::paintBackground(int xx, int yy, int ww, int hh, CPngImage *image) {
    if (!image)
        return;
    if (ww < 0 || hh < 0)
        return;
    int dx = image->w();
    int dy = image->h();
    fl_push_clip(xx, yy, ww, hh);
    int maxx = xx + ww;
    int maxy = yy + hh;
    /// Image alignments
    yy = (yy / dy) * dy;
    xx = (xx / dx) * dx;
    for (int xi = xx; xi < maxx; xi += dx) {
        for (int yi = yy; yi < maxy; yi += dy)
            image->draw(xi, yi, dx, dy, 0, 0);
    }
    fl_pop_clip();
}

void CThemes::drawThemeBox(int x, int y, int w, int h, Fl_Color clr, Fl_Boxtype frameType) {
    CPngImage *image = imageForColor(clr);
    if (image) {
        paintBackground(x + Fl::box_dx(frameType), y + Fl::box_dy(frameType), w - Fl::box_dw(frameType), h - Fl::box_dh(frameType), image);
    } else {
        fl_color(clr);
        fl_rectf(x, y, w, h);
    }
    fl_draw_box(frameType, x, y, w, h, clr);
}

void CThemes::drawFlatBox(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeBox(x, y, w, h, clr, FL_NO_BOX);
}

void CThemes::drawThinUpBox(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeBox(x, y, w, h, clr, FL_THIN_UP_FRAME);
}

void CThemes::drawUpBox(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeBox(x, y, w, h, clr, FL_UP_FRAME);
}

void CThemes::drawThinDownBox(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeBox(x, y, w, h, clr, FL_THIN_DOWN_FRAME);
}

void CThemes::drawDownBox(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeBox(x, y, w, h, clr, FL_DOWN_FRAME);
}

void CThemes::drawThemeFrame(int x, int y, int w, int h, Fl_Boxtype frameType) {
    CFrame* frame = m_frames.find(frameType);
    if (frame)
        frame->drawResized(x, y, w, h, false);
    else
        fl_draw_box(frameType, x, y, w, h, FL_BLACK);
}

void CThemes::drawThinUpFrame(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeFrame(x, y, w, h, FL_THIN_UP_FRAME);
}

void CThemes::drawUpFrame(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeFrame(x, y, w, h, FL_UP_FRAME);
}

void CThemes::drawThinDownFrame(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeFrame(x, y, w, h, FL_THIN_DOWN_FRAME);
}

void CThemes::drawDownFrame(int x, int y, int w, int h, Fl_Color clr) {
    drawThemeFrame(x, y, w, h, FL_DOWN_FRAME);
}

bool CThemes::sizeButton(CThemeButtonType sz, int& w, int& h) {
    CThemeImageCollection& buttons = sizeToButtonImages(sz);
    CPngImage *image = buttons.image(THM_IMAGE_NORMAL);
    if (!image) {
        image = buttons.overlayImage(THM_IMAGE_NORMAL);
        if (!image)
            return false;
    }
    if (sz != THM_BUTTON_COMBO) {
        if (w < image->h() + CThemes::m_buttonFocusRadius)
            w = image->h() + CThemes::m_buttonFocusRadius;
    }
    h = image->h();
    return true;
}

bool CThemes::drawButton(CThemeButtonType sz, int x, int y, int& w, int& h, bool highlited, bool pressed, bool defaultButton) {
    int ndx = THM_IMAGE_NORMAL;

    if (pressed)
        ndx = THM_IMAGE_ACTIVE;

    CPngImage *image;
    CThemeImageCollection& buttons = sizeToButtonImages(sz);
    if (highlited)
        ndx |= THMF_HIGHLIGHTED;

    image = buttons.image(CThemeImageState(ndx));

    if (!image || image->w() < 10) {
        if (defaultButton)
            fl_draw_box(FL_THIN_DOWN_BOX, x - 2, y - 2, w + 4, h + 4, FL_BACKGROUND_COLOR);
        CThemeColorState colorState = THM_COLOR_NORMAL;
        if (pressed)
            colorState = THM_COLOR_ACTIVE;
        else if (highlited)
            colorState = THM_COLOR_PRELIGHT;
        fl_draw_box(pressed?FL_THIN_DOWN_BOX:FL_THIN_UP_BOX, x, y, w, h, m_colors.bgColor(colorState));
        return true;
    }

    //h = image->h();

    if (m_flatButtons && !highlited) {
        if (defaultButton)
            fl_draw_box(FL_DOWN_BOX, x - 2, y - 2, w + 4, h + 4, FL_BACKGROUND_COLOR);
        else
            fl_draw_box(FL_FLAT_BOX, x, y, w, h, FL_BACKGROUND_COLOR);
    } else {
        CPngImage *defaultFrameImage = buttons.image(THM_DEFAULT_FRAME);
        if (defaultButton && defaultFrameImage)
            defaultFrameImage->drawResized(x - 2, y - 2, w + 4, h + 4, buttons.border(), CPngImage::PDM_STRETCH, false);
        image->drawResized(x, y, w, h, buttons.border(), CPngImage::PDM_STRETCH, true);
    }

    return true;
}

bool CThemes::drawCheckOrRadioButton(CThemeImageCollection& buttonImages, int x, int y, bool checked, bool highlited) {
    int ndx = THM_IMAGE_NORMAL;
    if (checked)
        ndx = THM_IMAGE_ACTIVE;

    if (highlited)
        ndx |= THMF_HIGHLIGHTED;

    CPngImage* image = buttonImages.overlayImage(CThemeImageState(ndx));
    if (!image)
        image = buttonImages.image(CThemeImageState(ndx));

    if (!image || image->w() < 10)
        return false;

    image->draw(x, y);

    return true;
}

bool CThemes::drawCheckButton(int x, int y, bool checked, bool highlited) {
    return drawCheckOrRadioButton(m_checkButtons, x, y, checked, highlited);
}

bool CThemes::drawRadioButton(int x, int y, bool checked, bool highlited) {
    return drawCheckOrRadioButton(m_radioButtons, x, y, checked, highlited);
}

bool CThemes::sizeProgressBar(int& w, int& h) {
    CPngImage *image = m_progressBar[0].image(THM_IMAGE_NORMAL);
    if (image && image->data()) {
        w = image->w();
        h = image->h();
        return true;
    } else {
        w = h = 0;
        return false;
    }
}

bool CThemes::drawProgressBar(int x, int y, int w, float percent) {
    if (percent > 100)
        percent = 100;
    if (percent < 0)
        percent = 0;
    
    CPngImage*                  partImage[2];  // 0 - trough, 1 - bar
    int                         border[2] = { 0, 0 };
    CPngImage::CPatternDrawMode drawMode[2] = { CPngImage::PDM_TILE, CPngImage::PDM_TILE };
    
    for (unsigned i = 0; i < 2; i++) {
        partImage[i] = m_progressBar[i].image(THM_IMAGE_NORMAL);
        if (m_progressBar[i].stretch()) {
            drawMode[i] = CPngImage::PDM_STRETCH;
            border[i] = m_progressBar[i].border(0);
        }
    }
            
    if (partImage[0] && partImage[1]) {
        int frameHeight = partImage[0]->h();
        int frameWidth = partImage[0]->w();
        int barHeight = partImage[1]->h();
        int barWidth = partImage[1]->w();
        int xspace = (frameWidth - barWidth) / 2;
        int yspace = (frameHeight - barHeight) / 2;
        int psize = int((w - xspace * 2) * percent / 100 + 0.5);
        partImage[0]->drawResized(x, y, w, frameHeight, border[0], drawMode[0], true);
        partImage[1]->drawResized(x + xspace, y + yspace, psize, barHeight, border[1], drawMode[1], true);
        return true;
    } else {
        return false;
    }
}

CStrings CThemes::availableThemes() {
    CStrings themes;
    themes.push_back("Default");
    //themes.push_back("GTK");
    const CStrings& dirs = searchDirectories();
    for (unsigned i = 0; i < dirs.size(); i++) {
        try {
            CDirectoryDS dir;
            dir.directory(dirs[i]);

            dir.showPolicy(DDS_HIDE_DIRECTORIES | DDS_HIDE_DOT_FILES);
            dir.pattern("*.tar");
            dir.open();
            while (!dir.eof()) {
                std::string fileName = dir["Name"];
                size_t pos = fileName.rfind(".tar");
                if (pos != STRING_NPOS) {
                    string themeName = fileName.substr(0, pos);
                    if (themes.indexOf(themeName) < 0)
                        themes.push_back(themeName);
                }
                dir.next();
            }
            dir.close();
        } catch (...) {
            continue;
        }
    }
    
    /// GTK2 themes
    CStrings gtkDirs;
    gtkDirs.push_back("/usr/share/themes");
    gtkDirs.push_back(CRegistry::homeDirectory() + ".themes");
    for (unsigned i = 0; i < gtkDirs.size(); i++) {
        try {
            CDirectoryDS dir;
            dir.directory(gtkDirs[i]);

            dir.showPolicy(DDS_HIDE_FILES | DDS_HIDE_DOT_FILES);
            //dir.pattern("*.tar");
            dir.open();
            while (!dir.eof()) {
                std::string dirName = dir["Name"];
                std::string gtkThemeDirName = gtkDirs[i] + "/" + dirName + "/gtk-2.0";
                if (access(gtkThemeDirName.c_str(),X_OK) == 0) {
                    string themeName = "GTK:" + dirName;
                    if (themes.indexOf(themeName) < 0)
                        themes.push_back(themeName);
                }
                dir.next();
            }
            dir.close();
        } catch (...) {
            continue;
        }
    }
    return themes;
}

bool CThemes::drawTab(int x, int y, int w, int h, bool active) {
    CTabImage* tabImage;
    if (active)
        tabImage = m_tabImages.tabImage("active");
    else
        tabImage = m_tabImages.tabImage("inactive");
    if (!tabImage)
        return false;
    tabImage->draw(x, y, w, h);
    return true;
}

bool CThemes::drawTabFrame(int x, int y, int w, int h) {
    return false;
}
