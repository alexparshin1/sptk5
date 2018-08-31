/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CProgressBar.h - description                           ║
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

#ifndef __CPROGRESSBAR_H__
#define __CPROGRESSBAR_H__

#include <sptk5/gui/CDataControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Progress bar widget
 *
 * The extended version of FLTK's standard Fl_Progress.
 * Added the CLayoutClient interface and preferred size computations.
 */
class CProgressBar : public CInput {

    /**
     * Constructor initializer
     * @param label const char *, widget label
     */
    void ctor_init(const char *label);

public:
    /**
     * Constructor in SPTK style
     * @param lbl const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CProgressBar(const char *lbl,int layoutSize=20,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CProgressBar(int x,int y,int w,int h,const char * label=0);
#endif

    /**
     * Computes the optimal progress bar size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w,int& h);

    /**
     * Defines the progress bar minimum value
     * The default is 0
     */
    void minimum(float min);

    /**
     * Returnes the progress bar minimum value
     */
    float minimum() const;

    /**
     * Defines the progress bar maximum value
     * The default is 100
     */
    void maximum(float max);

    /**
     * Returnes the progress bar maximum value
     */
    float maximum() const;

    /**
     * Sets progress bar units, default is "%"
     */
    void units(const char *un);

    /**
     * Returns progress bar units
     */
    std::string units() const;

    /**
     * Show the progress text or not?
     * The default is true
     */
    void showText(bool flag=true);

    /**
     * Universal data connection, returns data from date control.
     * Data is returned as VAR_FLOAT.
     */
    virtual Variant data() const;

    /**
     * Universal data connection, sets data to date control
     */
    virtual void     data(const Variant v);

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);
};
/**
 * @}
 */
}
#endif
