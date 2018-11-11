/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTabs.h - description                                  ║
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

#ifndef __CTABS_H__
#define __CTABS_H__

#include <sptk5/sptk.h>
#include <sptk5/gui/CGroup.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CTabGroup;

/**
 * @brief Tabs widget
 *
 * It can show the tabs in several rows if they are too wide to fit.
 */
class SP_EXPORT CTabs : public CGroup {
    /**
     * Tabs as a separate group
     */
    CTabGroup*  m_tabs;

    /**
     * The internal index inside tabs auto-color table
     */
    int32_t     m_autoColorIndex;


    /**
     * auto-color table
     */
    static const Fl_Color AutoColorTable[16];

    /**
     * Removes an empty last page prior to adding a new page
     */
    void        removeEmptyLastPage();

protected:
    /**
     * Sets default parameters for a new page
     */
    virtual void prepareNewPage(Fl_Group *page,bool autoColor);

public:
    /**
     * @brief Constructor in SPTK style
     * @param label const char*, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CTabs(const char* label=0L,int32_t layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char* label
     */
    CTabs(int x,int y,int w,int h,const char* label = 0);
#endif

    /**
     * @brief Destructor
     */
    ~CTabs();

    /**
     * @brief Creates a new CGroup page
     * @param label             Tab and page label
     * @param autoColor         Auto-color assignment on/off
     */
    virtual Fl_Group* newPage(const char* label,bool autoColor);

    /**
     * @brief Creates a new CScroll page
     * @param label             Tab and page label
     * @param autoColor         Auto-color assignment on/off
     */
    virtual Fl_Group* newScroll(const char* label,bool autoColor);

    /**
     * @brief Custom draw method
     */
    virtual void draw();

    /**
     * @brief Show/hide tabs flag
     *
     * @param show bool, if true the tabs would be visible
     */
    void showTabs(bool show);

    /**
     * @brief Returns currently selected page
     */
    Fl_Group *page();

    /**
     * Selects page
     */
    uint32_t page(Fl_Widget *page);

    /**
     * @brief Returns tabs count
     */
    uint32_t pageCount() const;

    /**
     * @brief Returns selected page number
     */
    uint32_t pageNumber() const;

    /**
     * @brief Sets selected page by number
     * @param pageNumber uint32_t, a page number to select
     */
    void pageNumber(uint32_t pageNumber);

    /**
     * @brief Removes a page completely
     * @param page Fl_Group*, page to remove
     */
    void remove(Fl_Group* page);

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual String className() const {
        return "tabs";
    }
};
/**
 * @}
 */
}
#endif
