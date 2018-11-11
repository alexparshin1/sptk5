/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       font_combo.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cgui>

using namespace std;
using namespace sptk;

CMemoInput *memoInput;

void font_cb(Fl_Widget *fc, void *)
{
    auto fontCombo = dynamic_cast<CFontComboBox *>(fc);
    if (!fontCombo)
        return;
    if (fontCombo->eventType() == CE_DATA_CHANGED) {
        std::string fontName = fontCombo->fontName();
        fontName = fontName + "  " + fontName + "  ";
        fontName = fontName + "\n" + fontName + "\n";
        fontName = fontName + fontName;
        memoInput->textFont(fontCombo->font());
        memoInput->data(fontName);
        memoInput->redraw();
    }
}

int main(int argc, char *argv[])
{
    try {
        // Initialize themes
        CThemes themes;

        CWindow w(300, 200, "Font Combo test");
        w.resizable(w);
        w.color(0xC0C8FF00);
        w.layoutSpacing(4);

        CFontComboBox fontCombo("Fonts:");
        fontCombo.callback(font_cb);

        memoInput = new CMemoInput("Font Sample:", 10, SP_ALIGN_CLIENT);
        memoInput->data("This is just some text");

        w.end();
        w.show(argc, argv);

        return Fl::run();
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
