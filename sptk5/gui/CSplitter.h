/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CSplitter.h - description                              ║
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

#ifndef __CSPLITTER_H__
#define __CSPLITTER_H__

#include <sptk5/sptk.h>
#include <sptk5/gui/CDataControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Splitter widget
 *
 * CSplitter implements 'grab and resize' bar.
 * It works only with SPTK-style widgets derived from CLayoutClient,
 * and placed on a group derived from CLayoutManager (CGroup,CScroll).
 */
class CSplitter : public CBox
{
    Fl_Widget      *m_chainedWidget;           /// Pointer on the widget that will be resized
    CLayoutClient  *m_chainedWidgetLayout;     /// Pointer on the CLayoutClient of the resized widget
    bool            m_dragging {false};        /// The flag indicating the dragging state
    int             m_lastDragX {0};           /// Where did we grab the bar (x-coordinate) ?
    int             m_lastDragY {0};           /// Where did we grab the bar (y-coordinate) ?
protected:
    void findChainedControl();                 /// After the splitter is grabbed finds out which widget is resizing
public:
    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CSplitter(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

    /**
     * Custom handle() method to process widget-specific events
     */
    int handle(int event);

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual String className() const {
        return "splitter";
    }
};
/**
 * @}
 */
}
#endif
