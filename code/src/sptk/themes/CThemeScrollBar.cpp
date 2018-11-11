/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CThemeScrollBar.cpp - description                      ║
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

#include <FL/fl_draw.H>
#include <sptk5/gui/CThemeScrollBar.h>
#include <cmath>

using namespace std;
using namespace sptk;

CScrollBarImages CThemeScrollBar::m_scrollBar[2];
int CThemeScrollBar::m_scrollBarTroughBorder = 1;
int CThemeScrollBar::m_scrollBarSliderWidth = 14;
int CThemeScrollBar::m_scrollBarStepperSize = 14;

void CScrollBarImages::load(CThemeScrollBarType atype, bool /*desaturateInactiveButtons*/, bool /*externalFile*/)
{
    static const char* scrollbarParts[] = {"-trough", "-slider", "-grip"};

    m_orientation = atype;
    string scrollBarDir;
    if (atype == THM_SCROLLBAR_VERTICAL)
        scrollBarDir = "vscrollbar";
    else
        scrollBarDir = "hscrollbar";
    clear();
    unsigned i;
    /// Load scrollbar images
    for (i = 0; i < 3; i++)
        m_parts[i].loadFromSptkTheme(Strings(scrollBarDir + scrollbarParts[i], "|"));

    CThemeImageCollection& trough = m_parts[0];
    CPngImage* troughImage = trough.image(THM_IMAGE_NORMAL);

    /// Load scrollbar button images
    for (i = 0; i < 2; i++) {
        Strings buttonNames;
        for (unsigned j = 0; j < 2; j++) {
            stringstream fileName;
            fileName << scrollBarDir << "-button" << i;
            if (j == 1)
                fileName << "-active";
            buttonNames.push_back(fileName.str());
        }
        m_steppers[i].loadFromSptkTheme(buttonNames);
        CPngImage* button0 = m_steppers[0].image(THM_IMAGE_NORMAL);
        if (button0) {
            CThemeScrollBar::m_scrollBarTroughBorder = 0;
            if (atype == THM_SCROLLBAR_VERTICAL) {
                CThemeScrollBar::m_scrollBarStepperSize = button0->h();
                CThemeScrollBar::m_scrollBarSliderWidth = button0->w();
                if (troughImage)
                    CThemeScrollBar::m_scrollBarTroughBorder =
                            (troughImage->w() - CThemeScrollBar::m_scrollBarSliderWidth) / 2;
            } else {
                CThemeScrollBar::m_scrollBarStepperSize = button0->w();
                CThemeScrollBar::m_scrollBarSliderWidth = button0->h();
                if (troughImage)
                    CThemeScrollBar::m_scrollBarTroughBorder =
                            (troughImage->h() - CThemeScrollBar::m_scrollBarSliderWidth) / 2;
            }
        }
    }
}

void CScrollBarImages::clear()
{
    for (unsigned i = 0; i < 2; i++) {
        m_parts[i].clear();
        m_steppers[i].clear();
    }
}

bool CThemeScrollBar::sizeScrollBar(int& w, int& h)
{
    bool vertical = w < h;
    int troughWidth = m_scrollBarSliderWidth + m_scrollBarTroughBorder * 2;
    int minLength = m_scrollBarStepperSize * 2 + 10;
    if (vertical) {
        w = troughWidth;
        if (h < minLength)
            h = minLength;
    } else {
        h = troughWidth;
        if (w < minLength)
            w = minLength;
    }
    return true;
}

void CThemeScrollBar::loadGtkScrollbarButtons(
        xml::Document& xml, const String& orientation, CThemeImageCollection& buttonImages)
{
    String XPath("/styles/style[@name='scrollbars']/engine[@name='pixmap']/image[@function='STEPPER']");
    buttonImages.loadFromGtkTheme(xml, XPath, "arrow_direction", orientation);
}

void CThemeScrollBar::loadGtkScrollbarTroughs(xml::Document& xml)
{
    static const char* orientation[2] = {"VERTICAL", "HORIZONTAL"};
    for (unsigned i = 0; i < 2; i++) {
        CThemeImageCollection* images = &m_scrollBar[i].m_parts[CScrollBarImages::TROUGH];
        images->loadFromGtkTheme(xml,
                                 "/styles/style[@name='scrollbars']/engine[@name='pixmap']/image[@detail='trough']",
                                 "orientation", orientation[i]);
    }
}

void CThemeScrollBar::loadGtkScrollbarSliders(xml::Document& xml)
{
    static const char* orientation[2] = {"VERTICAL", "HORIZONTAL"};

    for (unsigned i = 0; i < 2; i++) {
        CThemeImageCollection* images = &m_scrollBar[i].m_parts[CScrollBarImages::SLIDER];
        images->loadFromGtkTheme(xml,
                                 "/styles/style[@name='scrollbars']/engine[@name='pixmap']/image[@function='SLIDER']",
                                 "orientation", orientation[i]);
    }
}

void CThemeScrollBar::loadGtkScrollbars(xml::Document& xml)
{
    xml::NodeVector scrollBarDefaults;
    xml.select(scrollBarDefaults, "/styles/style/GtkRange");
    if (!scrollBarDefaults.empty()) {
        xml::Node* node = scrollBarDefaults[0];
        m_scrollBarTroughBorder = node->getAttribute("trough_border", "1");
        m_scrollBarSliderWidth = node->getAttribute("slider_width", "14");
        m_scrollBarStepperSize = node->getAttribute("stepper_size", "14");
    }
    loadGtkScrollbarButtons(xml, "UP", m_scrollBar[THM_SCROLLBAR_VERTICAL].m_steppers[0]);
    loadGtkScrollbarButtons(xml, "DOWN", m_scrollBar[THM_SCROLLBAR_VERTICAL].m_steppers[1]);
    loadGtkScrollbarButtons(xml, "LEFT", m_scrollBar[THM_SCROLLBAR_HORIZONTAL].m_steppers[0]);
    loadGtkScrollbarButtons(xml, "RIGHT", m_scrollBar[THM_SCROLLBAR_HORIZONTAL].m_steppers[1]);
    loadGtkScrollbarTroughs(xml);
    loadGtkScrollbarSliders(xml);
}

static void drawImageWithOverlay(int x, int y, CPngImage* image, CPngImage* overlay)
{
    if (image) {
        image->draw(x, y);
        if (overlay)
            overlay->draw(x + (image->w() - overlay->w()) / 2, y + (image->h() - overlay->h()) / 2);
    }
}

bool CThemeScrollBar::drawScrollBar(
        int x, int y, int w, int h, float minimum, float maximum, float value, float sliderSize, int activeZone,
        bool active)
{
    bool vertical = w < h;

    // Selecting the group of images
    CScrollBarImages* scrollbar = &m_scrollBar[THM_SCROLLBAR_HORIZONTAL];
    if (vertical)
        scrollbar = &m_scrollBar[THM_SCROLLBAR_VERTICAL];

    // Painting the background
    CThemeImageCollection& trough = scrollbar->m_parts[CScrollBarImages::TROUGH];
    CPngImage* normalImage = trough.image(THM_IMAGE_NORMAL);
    if (normalImage)
        normalImage->drawResized(x, y, w, h, m_scrollBarStepperSize * 2, CPngImage::PDM_STRETCH, true);
    else
        fl_draw_box(FL_THIN_DOWN_BOX, x, y, w, h, FL_BACKGROUND2_COLOR);

    // Painting the buttons
    CPngImage* firstButtonSrc = scrollbar->m_steppers[0].image(THM_IMAGE_NORMAL);
    CPngImage* firstButtonOvl = scrollbar->m_steppers[0].overlayImage(THM_IMAGE_NORMAL);
    CPngImage* secondButtonSrc = scrollbar->m_steppers[1].image(THM_IMAGE_NORMAL);
    CPngImage* secondButtonOvl = scrollbar->m_steppers[1].overlayImage(THM_IMAGE_NORMAL);
    switch (activeZone) {
        case 1:
            firstButtonSrc = scrollbar->m_steppers[0].image(THM_IMAGE_ACTIVE);
            firstButtonOvl = scrollbar->m_steppers[0].overlayImage(THM_IMAGE_NORMAL);
            break;
        case 2:
            secondButtonSrc = scrollbar->m_steppers[1].image(THM_IMAGE_ACTIVE);
            secondButtonOvl = scrollbar->m_steppers[1].overlayImage(THM_IMAGE_NORMAL);
            break;
        default:
            break;
    }

    int bsize = vertical ? w - 2 : h - 2;

    if (firstButtonSrc && secondButtonSrc) {
        // Images available
        drawImageWithOverlay(x, y, firstButtonSrc, firstButtonOvl);
        if (vertical)
            drawImageWithOverlay(x, y + h - secondButtonSrc->h(), secondButtonSrc, secondButtonOvl);
        else
            drawImageWithOverlay(x + w - secondButtonSrc->w(), y, secondButtonSrc, secondButtonOvl);
    } else {
        // Images not available
        if (vertical) {
            fl_draw_box(activeZone == 1 ? FL_FLAT_BOX : FL_THIN_UP_BOX, x + 1, y + 1, bsize, bsize,
                        FL_BACKGROUND_COLOR);
            fl_draw_box(activeZone == 2 ? FL_FLAT_BOX : FL_THIN_UP_BOX, x + 1, y + h - bsize - 1, bsize, bsize,
                        FL_BACKGROUND_COLOR);
            if (active)
                fl_color(FL_FOREGROUND_COLOR);
            else
                fl_color(fl_inactive(FL_FOREGROUND_COLOR));
            int w1 = (w - 4) / 3;
            if (w1 < 1)
                w1 = 1;
            int x1 = x + (w - 2 * w1 - 1) / 2;
            int yy1 = y + (w - w1 - 1) / 2;
            fl_polygon(x1, yy1 + w1, x1 + 2 * w1, yy1 + w1, x1 + w1, yy1);
            yy1 += h - w;
            fl_polygon(x1, yy1, x1 + w1, yy1 + w1, x1 + 2 * w1, yy1);
        } else {
            fl_draw_box(activeZone == 1 ? FL_FLAT_BOX : FL_THIN_UP_BOX, x + 1, y + 1, bsize, bsize,
                        FL_BACKGROUND_COLOR);
            fl_draw_box(activeZone == 2 ? FL_FLAT_BOX : FL_THIN_UP_BOX, x + w - bsize - 1, y + 1, bsize, bsize,
                        FL_BACKGROUND_COLOR);
            if (active)
                fl_color(FL_FOREGROUND_COLOR);
            else
                fl_color(fl_inactive(FL_FOREGROUND_COLOR));
            int w1 = (h - 4) / 3;
            if (w1 < 1)
                w1 = 1;
            int x1 = x + (h - w1 - 1) / 2;
            int yy1 = y + (h - 2 * w1 - 1) / 2;
            fl_polygon(x1, yy1 + w1, x1 + w1, yy1 + 2 * w1, x1 + w1, yy1);
            x1 += (w - h);
            fl_polygon(x1, yy1, x1, yy1 + 2 * w1, x1 + w1, yy1 + w1);
        }
    }

    // Draw the slider
    double val;
    if (minimum == maximum)
        val = 0.5;
    else {
        val = (value - minimum) / (maximum - minimum);
        if (val > 1.0)
            val = 1.0;
        else if (val < 0.0)
            val = 0.0;
    }

    int ww = (vertical ? h : w) - bsize * 2;
    auto S = int(std::round(sliderSize * ww));
    if (S < 10)
        S = 10;
    int xx = (int)std::round(val * (ww - S)) + bsize;

    int xsl, ysl, wsl, hsl;
    CPngImage* sliderImage = scrollbar->m_parts[CScrollBarImages::SLIDER].image(THM_IMAGE_NORMAL);
    CPngImage* gripImage = scrollbar->m_parts[CScrollBarImages::SLIDER].overlayImage(THM_IMAGE_NORMAL);
    if (vertical) {
        ysl = y + xx;
        hsl = S;
        wsl = m_scrollBarSliderWidth;
        xsl = x + m_scrollBarTroughBorder;
        if (sliderImage) {
            sliderImage->drawResized(xsl, ysl, wsl, hsl, bsize, CPngImage::PDM_STRETCH, true);
            if (gripImage && gripImage->h() <= hsl + bsize * 3 / 2)
                gripImage->draw(xsl + (wsl - gripImage->w()) / 2, ysl + (hsl - gripImage->h()) / 2);
        } else
            fl_draw_box(FL_THIN_UP_BOX, x + 1, ysl + 1, bsize, hsl - 2, FL_BACKGROUND_COLOR);
    } else {
        xsl = x + xx;
        wsl = S;
        hsl = m_scrollBarSliderWidth;
        ysl = y + m_scrollBarTroughBorder;
        if (sliderImage) {
            sliderImage->drawResized(xsl, ysl, wsl, hsl, bsize, CPngImage::PDM_STRETCH, true);
            if (gripImage && gripImage->w() <= wsl + bsize * 3 / 2)
                gripImage->draw(xsl + (wsl - gripImage->w()) / 2, ysl + (hsl - gripImage->h()) / 2);
        } else
            fl_draw_box(FL_THIN_UP_BOX, xsl + 1, y + 1, wsl - 2, bsize, FL_BACKGROUND_COLOR);
    }

    return true;
}
