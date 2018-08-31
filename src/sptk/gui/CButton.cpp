/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CButton.cpp - description                              ║
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

#include <FL/fl_draw.H>
#include <sptk5/gui/CThemes.h>
#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CLayoutManager.h>

using namespace sptk;
using namespace std;

void CBaseButton::image(CButtonKind buttonKind, CIconSize iconSize, const char* label)
{
    const char* name;
    const char* l;
    switch (buttonKind) {
        default:
            name = nullptr;
            l = "??";
            break;
        case SP_ADD_BUTTON:
            name = "add";
            l = "Add";
            break;
        case SP_DELETE_BUTTON:
            name = "delete";
            l = "Delete";
            break;
        case SP_EDIT_BUTTON:
            name = "edit";
            l = "Edit";
            break;
        case SP_BROWSE_BUTTON:
            name = "browse";
            l = "Browse";
            break;
        case SP_CALENDAR_BUTTON:
            name = "calendar";
            l = "Date";
            break;
        case SP_REFRESH_BUTTON:
            name = "refresh";
            l = "Refresh";
            break;
        case SP_CANCEL_BUTTON:
            name = "cancel";
            l = "Cancel";
            break;
        case SP_HELP_BUTTON:
            name = "help";
            l = "Help";
            break;
        case SP_NO_BUTTON:
            name = "no";
            l = "No";
            break;
        case SP_OK_BUTTON:
            name = "ok";
            l = "Ok";
            break;
        case SP_OPEN_BUTTON:
            name = "open";
            l = "Open";
            break;
        case SP_PRINT_BUTTON:
            name = "print";
            l = "Print";
            break;
        case SP_SAVE_BUTTON:
            name = "save";
            l = "Save";
            break;
        case SP_COPY_BUTTON:
            name = "copy";
            l = "Insert";
            break;
        case SP_LEFT_BUTTON:
            name = "left";
            l = "Left";
            break;
        case SP_NEW_BUTTON:
            name = "new";
            l = "New";
            break;
        case SP_NEXT_BUTTON:
            name = "next";
            l = "Next";
            break;
        case SP_PRINTER_BUTTON:
            name = "printer";
            l = "Printer";
            break;
        case SP_PRIOR_BUTTON:
            name = "prior";
            l = "Prior";
            break;
        case SP_RIGHT_BUTTON:
            name = "right";
            l = "Right";
            break;
        case SP_SEARCH_BUTTON:
            name = "search";
            l = "Search";
            break;
        case SP_SEND_BUTTON:
            name = "send";
            l = "Send";
            break;
        case SP_STEPLEFT_BUTTON:
            name = "stepleft";
            l = "Step Left";
            break;
        case SP_STEPRIGHT_BUTTON:
            name = "stepright";
            l = "Step Right";
            break;
        case SP_VIEW_BUTTON:
            name = "view";
            l = "View";
            break;
        case SP_HOME_BUTTON:
            name = "home";
            l = "Home";
            break;
        case SP_EXEC_BUTTON:
            name = "exec";
            l = "Exec";
            break;
        case SP_STOP_BUTTON:
            name = "stop";
            l = "Stop";
            break;
        case SP_EXIT_BUTTON:
            name = "exit";
            l = "Exit";
            break;
        case SP_SAVE_AS_BUTTON:
            name = "saveas";
            l = "Save As..";
            break;
        case SP_CONFIGURE_BUTTON:
            name = "configure";
            l = "Settings";
            break;
    }
    if (!label)
        label = l;
    m_image = nullptr;
    if (name)
        m_image = CThemes::getIconImage(name, iconSize);
    if (iconSize == IS_LARGE_ICON)
        this->label(label);
    m_kind = buttonKind;
    m_iconSize = iconSize;
}

void CBaseButton::label(const char* lbl)
{
    m_label = lbl;
    Fl_Button::label(m_label.c_str());
}

#ifdef __COMPATIBILITY_MODE__
CBaseButton::CBaseButton(CButtonKind kind,int x,int y,int w,const char *l,CThemeButtonType tbt)
: Fl_Button(x,y,w,30), CLayoutClient(this,w,SP_ALIGN_NONE) {
   m_default = false;
   m_type = tbt;
   image(kind,IS_LARGE_ICON,l);
   box(FL_THIN_UP_BOX);
   color(FL_LIGHT3);
}
#endif

CBaseButton::CBaseButton(CButtonKind kind, CLayoutAlign layoutAlign, bool is_small, const char* l, CThemeButtonType tbt)
: Fl_Button(0, 0, 20, 20), CLayoutClient(this, 20, layoutAlign)
{
    m_default = false;
    m_type = tbt;
    if (is_small)
        image(kind, IS_SMALL_ICON);
    else image(kind, IS_LARGE_ICON, l);
    box(FL_THIN_UP_BOX);
}

CBaseButton::CBaseButton(const char* l, CLayoutAlign layoutAlign, CThemeButtonType tbt)
: Fl_Button(0, 0, 20, 20), CLayoutClient(this, 20, layoutAlign),
  m_kind(SP_UNDEFINED_BUTTON),
  m_iconSize(IS_SMALL_ICON)
{
    m_default = false;
    m_type = tbt;
    m_image = nullptr;
    label(l);
    box(FL_THIN_UP_BOX);
}

void CBaseButton::defaultButton(bool isDefault)
{
    m_default = isDefault;
    if (isDefault)
        box(FL_UP_BOX);
    else box(FL_THIN_UP_BOX);
    redraw();
}

void CBaseButton::drawFocusLine(int xs, int ys, int xe, int ye)
{
    // Since FLTK under Win32 doesn't support the dotted line,
    // draw it with dots.
    int xp = xs;
    int yp = ys;
    int xsign = ((xs < xe) - (xs > xe));
    int ysign = ((ys < ye) - (ys > ye));
    if (xsign == 0 && ysign == 0) return;
    int dx = xsign * 2;
    int dy = ysign * 2;
    for (; xp * xsign <= xe * xsign && yp * ysign <= ye * ysign; xp += dx, yp += dy)
        fl_point(xp, yp);
}

void CBaseButton::drawFocus(bool usingTheme)
{
    fl_color(FL_FOREGROUND_COLOR);
    int r = 0;
    if (usingTheme) r = CThemes::buttonFocusRadius();
    if (r > h() / 2) r = h() / 2;
    int xx = x() + 3;
    int yy = y() + 3;
    int ww = w() - 7;
    int hh = h() - 7;
    if (m_default) {
        xx += 2;
        yy += 2;
        ww -= 6;
        hh -= 5;
    }
    drawFocusLine(xx + r, yy, xx + ww - r, yy);
    drawFocusLine(xx + ww - r, yy, xx + ww, yy + r);
    drawFocusLine(xx + ww, yy + r, xx + ww, yy + hh - r);
    drawFocusLine(xx + ww, yy + hh - r, xx + ww - r, yy + hh);
    drawFocusLine(xx + ww - r, yy + hh, xx + r, yy + hh);
    drawFocusLine(xx + r, yy + hh, xx, yy + hh - r);
    drawFocusLine(xx, yy + hh - r, xx, yy + r);
    drawFocusLine(xx, yy + r, xx + r, yy);
}

void CBaseButton::draw()
{
    if (m_kind != SP_UNDEFINED_BUTTON)
        image(m_kind, m_iconSize, m_label.c_str());
#if FL_MAJOR_VERSION > 1
    align(FL_ALIGN_INSIDE);
    image(m_image);
    //Fl_Button::draw();
#else
    align(Fl_Align(align() | FL_ALIGN_INSIDE));
    if (type() == FL_HIDDEN_BUTTON) return;

    int xx = x();
    int yy = y();
    int offset = 0;
    if (m_default) offset = 2;
    int X = xx + offset;
    int Y = yy + offset;
    int W = w() - offset * 2;
    int H = h() - offset * 2;

    bool highlited = (Fl::belowmouse() == this) && active_r();

    bool usingTheme = CThemes::drawButton(m_type, X, Y, W, H, highlited, value() != 0, m_default);

    fl_push_clip(X + 1, Y + 1, W - 2, H - 2);
    if (!m_image)
        draw_label();
    else {
        int pressedOffset = value() ? 1 : 0;
        xx = X + 4 + pressedOffset;
        yy = Y + (H - m_image->h()) / 2 + pressedOffset;
        int ww = W - 6;
        int hh = H - 6;
        int tx = xx + m_image->w() + 2;
        int tw = ww - (tx - xx) - 2;
        if (!label()) { // center image
            xx = x() + (w() - m_image->w() - 1) / 2 + pressedOffset;
            m_image->draw(xx, yy, m_image->w(), m_image->h(), 0, 0);
        } else {
            m_image->draw(xx, yy, ww, hh, 0, 0);
        }
        fl_color(FL_FOREGROUND_COLOR);
        fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
        int wt = tw;
        int ht = 0;
        fl_measure(label(), wt, ht);
        int ty = Y + H / 2 - ht / 2;
        fl_draw(label(), tx, ty, wt, ht, Fl_Align(FL_ALIGN_LEFT | FL_ALIGN_TOP));
        if (Fl::focus() == this) {
            //draw_focus(FL_FLAT_BOX, x() + 2, y() + 2, w() - 4, h() - 4);
            drawFocus(usingTheme);
        }
    }
    fl_pop_clip();
#endif
}

int CBaseButton::handle(int event)
{
    switch (event) {
        case FL_FOCUS: {
            CControl* control = nullptr;
            try {
                control = dynamic_cast<CControl*>(parent());
            }
            catch (...) {}

            if (!control && parent())
                try {
                    control = dynamic_cast<CControl*>(parent()->parent());
                    if (control)
                        control->notifyFocus();
                }
                catch (...) {}
        }
            break;

        case FL_ENTER:
        case FL_LEAVE:
            redraw();
            break;

        default:
            break;
    }
    return Fl_Button::handle(event);
}

bool CBaseButton::preferredSize(int& w, int& h)
{
    h = 0;

    int bh = 0;
    int bw = 0;
    int br = 0;
    CThemes::sizeButton(m_type, bw, bh);
    if (bw) {
        br = CThemes::buttonFocusRadius();
        if (m_default) {
            bh += 4;
            bw += 4;
        }
    }

    if (m_image)
        h = m_image->h() + 6;

    fl_font(labelfont(), labelsize());
    int lw = 0, lh = 0;
    fl_measure(label(), lw, lh);
    if (lh + 8 > h) h = lh + 8;
    if (lw) lw += 8;
    int ww = lw + 6 + br;
    if (m_image)
        ww += m_image->w();
    if (m_default) {
        h += 4;
        ww += 4;
    }
    if (bh > h) h = bh;
    if (w < ww) w = ww;
    if (w < bw) w = bw;
    return false;
}

CLayoutClient* CButton::creator(xml::Node* node)
{
    auto button = new CButton("", SP_ALIGN_TOP, THM_BUTTON_NORMAL);
    button->load(node);
    return button;
}

void CButton::load(const xml::Node* node)
{
    CLayoutClient::load(node, LXM_LAYOUT);
    string iconName = node->getAttribute("icon");
    string iconType = node->getAttribute("size", "large");
    CIconSize iconSize;
    switch (iconType[0]) {
        case 'c':
            iconSize = IS_COMBO_ICON;
            break;
        case 's':
            iconSize = IS_SMALL_ICON;
            break;
        default:
        case 'l':
            iconSize = IS_LARGE_ICON;
            break;
    }
    if (!iconName.empty()) {
        Fl_Image* image = CThemes::getIconImage(iconName, iconSize);
        buttonImage(image);
    }
}

void CButton::save(xml::Node* node) const
{
    CLayoutClient::save(node, LXM_LAYOUT);
}
