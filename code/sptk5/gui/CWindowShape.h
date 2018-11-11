/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWindowShape.h - description                           ║
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

#ifndef __CWINDOWSHAPE_H__
#define __CWINDOWSHAPE_H__

#include <vector>

#include <sptk5/gui/CShapePoint.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CWindow;

/**
 * @brief Window non-rectangular shape support
 *
 * Window shape is defined with the array of points that should be modified at every resize of the window.
 * The derived window should modify the shape points array by ovewriting shapeResize() method.
 * The window resize support is activated only if window resizable() is true.
 */
class CWindowShape {
protected:
    /**
     * Window to change shape
     */
    sptk::CWindow*           m_window;

    /**
     * Resizing mouse zone where mouse was pushed
     */
    int                      m_resizingZone;

    /**
     * Last window width during the shape generation
     */
    int                      m_lastW;

    /**
     * Last window height during the shape generation
     */
    int                      m_lastH;

    /**
     * X coordinate of the last FL_PUSH event
     */
    int                      m_pushedX;

    /**
     * Y coordinate of the last FL_PUSH event
     */
    int                      m_pushedY;

    /**
     * Vector of points defining window shape
     */
    std::vector<CShapePoint> m_shapePoints;

    /**
     * Is the shape size changed?
     */
    bool                     m_shapeChanged;

    /**
     * Shape border width, the default is 6
     */
    int                      m_borderWidth;

    /**
     * Is no border flag set for the window?
     */
    bool                     m_borderCleared;

    /**
     * Is the shape extension activated?
     */
    bool                     m_shapeExtension;


    /**
     * @brief Computes mouse zone from window coordinates
     * @param mouseX int, mouse x-coordinate in the window
     * @param mouseY int, mouse y-coordinate in the window
     */
    int  mouseZone(int mouseX,int mouseY) const;

    /**
     * @brief Resizes the window based on mouse zone and mouse screen coordinates
     * @param mouseX int, mouse x-coordinate in the screen
     * @param mouseY int, mouse y-coordinate in the screen
     */
    void changeSize(int mouseX,int mouseY);

    /**
     * @brief Changes the window cursor based on the mouse position in the window
     *
     * Designed to emulate cursor changes on the window border for shaped windows.
     * Should be uncoditionally called inside handle() for the window.
     */
    virtual int  shapeCursorHandle(int event);

public:
    /**
     * @brief Constructor
     * @param window sptk::CWindow*, window to change shape
     */
    explicit CWindowShape(sptk::CWindow* window);

    /**
     * @brief Destructor
     */
    virtual ~CWindowShape() {}

    /**
     * @brief Returns true if the window is resizing as a result of border drag
     *
     * Can be used to simplified drawing during the resize
     */
    bool shapeResizing() {
        return m_resizingZone != 0;
    }

protected:

    /**
     * @brief Initializes shape extension
     *
     * Allows window to use shape extension. Should be called
     * before the window is first shown.
     */
    void initShapeExtension();

    /**
     * @brief Returns true if border was cleared for the shape
     */
    bool borderCleared() const { return m_borderCleared; }

    /**
     * @brief Sets the shape to the window during window->draw()
     */
    void shapeApply();

    /**
     * @brief Calls resizeShape() from inside CWindow::resize()
     */
    void resizingShape(int w,int h) {
        if (m_lastW == w && m_lastH == h)
            return;
        shapeResize(w,h);
        m_shapeChanged = true;
        m_lastW = w;
        m_lastH = h;
    }

    /**
     * @brief Shape resize
     *
     * This method should be overwritten in the derived class
     * to adapt m_shapePoints[] to the window size.
     */
    virtual void shapeResize(int w,int h) {}
};
/**
 * @}
 */
}

#endif
