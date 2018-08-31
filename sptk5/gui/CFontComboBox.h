/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFontComboBox.h - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CFONTCOMBOBOX_H__
#define __CFONTCOMBOBOX_H__

#include <sptk5/gui/CComboBox.h>
#include <string>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Font combobox widget
 *
 * A combobox filled with the system fonts names
 */
class CFontComboBox : public CComboBox {
public:
    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CFontComboBox(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);
#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CFontComboBox(int x,int y,int w,int h,const char *label = 0);
#endif
    /**
     * Loads list of system fonts
     */
    void loadFonts();

    /**
     * Returns currently selected font name
     */
    std::string fontName() const {
        return data();
    }

    /**
     * Sets currently selected font name
     */
    void fontName(const char *fn) {
        data(fn);
    }

    /**
     * Returns font index as Fl_Font
     */
    Fl_Font font() const
    {
        CPackedStrings* theSelectedRow = selectedRow();
        if (theSelectedRow)
            return (Fl_Font) theSelectedRow->argument();
        return FL_HELVETICA;
    }

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual String className() const {
        return "font_combo";
    }
};
/**
 * @}
 */
}
#endif
