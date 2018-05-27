/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CEvent.h - description                                 ║
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

#ifndef __CEVENT_H__
#define __CEVENT_H__

#include <sptk5/sptk.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Event type constants
 */
enum CEvent {
    /**
     * Empty event (no event)
     */
    CE_NONE,

    /**
     * Control data changed
     */
    CE_DATA_CHANGED,

    /**
     * Control received focus
     */
    CE_FOCUS,

    /**
     * Control lost focus
     */
    CE_UNFOCUS,

    /**
     * Control visibility changed to show
     */
    CE_SHOW,

    /**
     * Control visibility changed to hide
     */
    CE_HIDE,

    /**
     * Keyboard event in control
     */
    CE_KEYBOARD,

    /**
     * Mouse clicked on control
     */
    CE_MOUSE_CLICK,

    /**
     * Mouse double-clicked on control
     */
    CE_MOUSE_DOUBLE_CLICK,

    /**
     * Mouse dragged on control
     */
    CE_MOUSE_DRAG,

    /**
     * Mouse moved over control
     */
    CE_MOUSE_MOVE,

    /**
     * Mouse released over control
     */
    CE_MOUSE_RELEASE,

    /**
     * Mouse wheel over control
     */
    CE_MOUSE_WHEEL,

    /**
     * Progress event (if control supports it)
     */
    CE_PROGRESS,

    /**
     * Keyboard button pressed
     */
    CE_BUTTON_PRESSED,

    /**
     * List event - item added (if control supports it)
     */
    UC_ADD_ITEM,

    /**
     * List event - item edited or changed (if control supports it)
     */
    UC_EDIT_ITEM,

    /**
     * List event - item deleted (if control supports it)
     */
    UC_DELETE_ITEM,

    /**
     * List event - list refreshed (if control supports it)
     */
    UC_REFRESH,

    /**
     * Last event number (no event)
     */
    CE_LAST_EVENT

};

/**
 * Event type information
 */
class CEventInfo {
    /**
     * Event type
     */
    CEvent        m_event;

    /**
     * Event argument
     */
    int32_t       m_argument;


public:
    /**
     * Constructor
     * @param eventType CEvent, event type
     * @param eventArg CEvent, event argument
     */
    CEventInfo(CEvent eventType=CE_NONE,int32_t eventArg=0) {
        m_event = eventType;
        m_argument = eventArg;
    }

    /**
     * Reports event type
     */
    CEvent  type() const {
        return m_event;
    }

    /**
     * Reports event argument
     */
    int32_t    argument() const {
        return m_argument;
    }
};
/**
 * @}
 */
}

#endif
