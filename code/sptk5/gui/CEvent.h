/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/sptk.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Event type constants
 */
enum class CEvent : uint8_t
{
    /**
     * Empty event (no event)
     */
    NONE,

    /**
     * Control data changed
     */
    DATA_CHANGED,

    /**
     * Control received focus
     */
    FOCUS,

    /**
     * Control lost focus
     */
    UNFOCUS,

    /**
     * Control visibility changed to show
     */
    SHOW,

    /**
     * Control visibility changed to hide
     */
    HIDE,

    /**
     * Keyboard event in control
     */
    KEYBOARD,

    /**
     * Mouse clicked on control
     */
    MOUSE_CLICK,

    /**
     * Mouse double-clicked on control
     */
    MOUSE_DOUBLE_CLICK,

    /**
     * Mouse dragged on control
     */
    MOUSE_DRAG,

    /**
     * Mouse moved over control
     */
    MOUSE_MOVE,

    /**
     * Mouse released over control
     */
    MOUSE_RELEASE,

    /**
     * Mouse wheel over control
     */
    MOUSE_WHEEL,

    /**
     * Progress event (if control supports it)
     */
    PROGRESS,

    /**
     * Keyboard button pressed
     */
    BUTTON_PRESSED,

    /**
     * List event - item added (if control supports it)
     */
    ADD_ITEM,

    /**
     * List event - item edited or changed (if control supports it)
     */
    EDIT_ITEM,

    /**
     * List event - item deleted (if control supports it)
     */
    DELETE_ITEM,

    /**
     * List event - list refreshed (if control supports it)
     */
    REFRESH,

    /**
     * Last event number (no event)
     */
    LAST_EVENT
};

/**
 * Event type information
 */
class CEventInfo
{
    /**
     * Event type
     */
    CEvent m_event;

    /**
     * Event argument
     */
    int32_t m_argument;


public:
    /**
     * Constructor
     * @param eventType CEvent, event type
     * @param eventArg CEvent, event argument
     */
    explicit CEventInfo(CEvent eventType = CEvent::NONE, int32_t eventArg = 0)
        : m_event(eventType)
        , m_argument(eventArg)
    {
    }

    /**
     * Assignment
     * @param eventType CEvent, event type
     */
    CEventInfo& operator=(CEvent eventType)
    {
        m_event = eventType;
        return *this;
    }

    /**
     * Reports event type
     */
    CEvent type() const
    {
        return m_event;
    }

    /**
     * Reports event argument
     */
    int32_t argument() const
    {
        return m_argument;
    }
};
/**
 * @}
 */
} // namespace sptk
