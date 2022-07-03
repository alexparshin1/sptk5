/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <FL/Fl_Double_Window.H>
#include <sptk5/Strings.h>
#include <sptk5/cutils>
#include <sptk5/gui/CLayoutManager.h>
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
class SP_EXPORT CWindow
    : public Fl_Double_Window
    , public CLayoutManager
    , public CWindowShape
{
public:
    /**
     * @brief Constructor
     * @param w int, window width
     * @param h int, window height
     * @param label int, window label
     */
    CWindow(int w, int h, const char* label = 0L)
        : Fl_Double_Window(w, h, label)
        , CLayoutManager(this, 10, CLayoutAlign::NONE)
        , CWindowShape(this)
    {
    }

    /**
     * @brief Constructor
     * @param x int, window x-position
     * @param y int, window y-position
     * @param w int, window width
     * @param h int, window height
     * @param label int, window label
     */
    CWindow(int x, int y, int w, int h, const char* label = 0L)
        : Fl_Double_Window(x, y, w, h)
        , CLayoutManager(this, 10, CLayoutAlign::NONE)
        , CWindowShape(this)
    {
    }

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
    void load(const std::shared_ptr<xdoc::Node>& node, CLayoutXMLmode xmlMode) override;

    /**
     * @brief Loads window coordinates and widgets from XML node
     *
     * @param node const xml::Node*, node to load data from
     */
    void load(const std::shared_ptr<xdoc::Node>& node) override
    {
        load(node, CLayoutXMLmode::DATA);
    }

    /**
     * @brief Saves window coordinates and widgets into XML node
     *
     * @param node xml::Node*, node to save data into
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be loaded
     */
    void save(const std::shared_ptr<xdoc::Node>& node, CLayoutXMLmode xmlMode) const override;

    /**
     * @brief Saves window coordinates and widgets into XML node
     *
     * @param node xml::Node*, node to save data into
     */
    virtual void save(const xdoc::SNode& node) const
    {
        save(node, CLayoutXMLmode::DATA);
    }

    /**
     * @brief Loads the window position from XML node
     *
     * @param node const xml::Node&, node to load position from
     */
    void loadPosition(const xdoc::SNode& node);

    /**
     * @brief Saves the window position into XML node
     *
     * @param node xml::Node&, node to save position into
     */
    void savePosition(const xdoc::SNode& node) const;

    /**
     * @brief Returns the current label
     */
    const String& label() const override
    {
        return m_label;
    }

    using CLayoutClient::label;

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
} // namespace sptk
