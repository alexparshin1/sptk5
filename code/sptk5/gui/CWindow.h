/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWindow.h - description                                ║
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

#ifndef __CWINDOW_H__
#define __CWINDOW_H__

#include <FL/Fl_Double_Window.H>
#include <sptk5/gui/CLayoutManager.h>
#include <sptk5/Strings.h>
#include <sptk5/gui/CWindowShape.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Window widget
 *
 * Extended version of FLTK's standard Fl_Double_Window, with added
 * CLayoutManager capabilities
 */
class CWindow : public Fl_Double_Window, public CLayoutManager, public CWindowShape
{
public:

    /**
     * @brief Constructor
     * @param w int, window width
     * @param h int, window height
     * @param label int, window label
     */
    CWindow(int w, int h, const char* label = 0L)
            : Fl_Double_Window(w, h, label), CLayoutManager(this, 10, SP_ALIGN_NONE), CWindowShape(this)
    {}

    /**
     * @brief Constructor
     * @param x int, window x-position
     * @param y int, window y-position
     * @param w int, window width
     * @param h int, window height
     * @param label int, window label
     */
    CWindow(int x, int y, int w, int h, const char* label = 0L)
            : Fl_Double_Window(x, y, w, h), CLayoutManager(this, 10, SP_ALIGN_NONE), CWindowShape(this)
    {}

    /**
     * @brief Draws a window, including an optional background image
     */
    void draw() override;

    /**
     * @brief Custom show method
     */
    void show() override
    {
        Fl_Double_Window::show();
    }

    /**
     * @brief Custom show method
     * @param argc int, number of parameters in argv[]
     * @param argv char*[], an array of program command line arguments
     */
    void show(int argc, char* argv[])
    {
        Fl_Double_Window::show(argc, argv);
    }

    /**
     * @brief Custom hide method
     */
    void hide() override;

    /**
     * @brief Relayouts window's widgets that have CLayoutClient interface
     */
    virtual void relayout()
    {
        resize(x(), y(), w(), h());
    }

    /**
     * @brief Removes all the widgets inside the window
     */
    void clear() override
    {
        Fl_Double_Window::clear();
    }

    /**
     * @brief Resizes the window and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    void resize(int x, int y, int w, int h) override;

    /**
     * @brief Computes the optimal window size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * @brief Custom window events handle
     *
     * Mostly used for supporting non-rectangular shape windows.
     */
    int handle(int event) override;

    /**
     * @brief Loads window coordinates and widgets from XML node
     *
     * @param node const xml::Node*, node to load data from
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be loaded
     */
    virtual void load(const xml::Node* node, CLayoutXMLmode xmlMode);

    /**
     * @brief Loads window coordinates and widgets from XML node
     *
     * @param node const xml::Node*, node to load data from
     */
    virtual void load(const xml::Node* node)
    {
        load(node, LXM_DATA);
    }

    /**
     * @brief Saves window coordinates and widgets into XML node
     *
     * @param node xml::Node*, node to save data into
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be loaded
     */
    virtual void save(xml::Node* node, CLayoutXMLmode xmlMode) const;

    /**
     * @brief Saves window coordinates and widgets into XML node
     *
     * @param node xml::Node*, node to save data into
     */
    virtual void save(xml::Node* node) const
    {
        save(node, LXM_DATA);
    }

    /**
     * @brief Loads the window position from XML node
     *
     * @param node const xml::Node&, node to load position from
     */
    void loadPosition(const xml::Node* node);

    /**
     * @brief Saves the window position into XML node
     *
     * @param node xml::Node&, node to save position into
     */
    void savePosition(xml::Node* node) const;

    /**
     * @brief Returns the current label
     */
    String label() const
    {
        return m_label;
    }

    /**
     * @brief Sets the new label
     *
     * @param lbl const char*, new label
     */
    void label(const char* lbl)
    {
        CLayoutClient::label(lbl);
    }

    /**
     * Sets label for the group, makes an internal copy of the string
     * @param lbl const string&, new label
     */
    void label(const std::string& lbl)
    {
        CLayoutClient::label(lbl);
    }

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    String className() const override
    {
        return "window";
    }
};
/**
 * @}
 */
}
#endif
