/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       check_buttons.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cgui>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

CInput*        i;
CCheckButtons* cb;
Fl_Box*        bx;

void button_cb(Fl_Widget* b, void*)
{
    if (strcmp(b->label(), "Get Choices") == 0)
    {
        i->data(cb->data());
    }
    else
    {
        cb->data(i->data());
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Initialize themes
        CThemes themes;

        CWindow w(400, 250);

        CCheckButtons cbl("Check Boxes: ");
        cbl.buttons(Strings("first,second,third,a very long choice text meant to occupy two rows,*", ","));
        cb = &cbl;

        CInput input("Test");
        i = &input;

        CGroup  g("", 10, CLayoutAlign::BOTTOM);
        CButton btn1("Set Choices", CLayoutAlign::RIGHT);
        btn1.callback(button_cb);

        CButton btn2("Get Choices", CLayoutAlign::RIGHT);
        btn2.callback(button_cb);

        w.end();
        w.show(argc, argv);

        CThemes::set("Keramic");
        w.reLayout();

        Fl::run();

        return EXIT_SUCCESS;
    }
    catch (const Exception& e)
    {
        CERR(e.what());
        return 1;
    }
}
