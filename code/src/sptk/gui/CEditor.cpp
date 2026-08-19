/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/gui/CEditor.h>

using namespace sptk;

void CEditor::ctor_init()
{
    auto* buff = new Fl_Text_Buffer;
    box(FL_THIN_DOWN_BOX);
    buffer(buff);
    m_lastCursorPosition = -1;
    m_wrapMode = false;
}

CEditor::CEditor(int layoutSize, CLayoutAlign layoutAlign)
    : Fl_Text_Editor(0, 0, 100, 100)
    , CLayoutClient(this, layoutSize, layoutAlign)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CEditor::CEditor(int x, int y, int w, int h, const char* l)
    : Fl_Text_Editor(x, y, w, h, l)
    , CLayoutClient(this, w, CLayoutAlign::NONE)
{
    ctor_init();
}
#endif

CEditor::~CEditor()
{
    delete mBuffer;
    mBuffer = nullptr;
}

bool CEditor::preferredSize(int& w, int& h)
{
    if (h < 50)
    {
        h = 50;
    }
    if (w < 50)
    {
        w = 50;
    }
    return false;
}

void CEditor::cursorRowCol(int& row, int& col)
{
    position_to_linecol(mCursorPos, &row, &col);
}

int CEditor::handle(int event)
{
    if ((event == FL_KEYUP || event == FL_RELEASE) && m_lastCursorPosition != mCursorPos)
    {
        m_lastCursorPosition = mCursorPos;
        do_callback();
    }
    return Fl_Text_Editor::handle(event);
}

void CEditor::wrapMode(bool wm)
{
    m_wrapMode = wm;
    wrap_mode(wm, 0);
    maintain_absolute_top_line_number(wm);
}
