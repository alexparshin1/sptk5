/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFontComboBox.cpp - description                        ║
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

#include <sptk5/sptk.h>

#include <FL/Fl.H>

#include <sptk5/gui/CLayoutManager.h>
#include <sptk5/gui/CColumn.h>
#include <sptk5/gui/CDBListView.h>
#include <sptk5/gui/CFontComboBox.h>

using namespace std;
using namespace sptk;

void CFontComboBox::loadFonts() {
   Strings fontNames;
   int k = Fl::set_fonts("*");
   for (int i = 0; i < k; i++) {
      int t;
      String fontIdString(string(Fl::get_font_name((Fl_Font)i,&t)),i);
      fontNames.push_back(fontIdString);
   }
   addRows("Fonts",fontNames);
   columns()[unsigned(0)].width(150);
   sortColumn(0);
   m_list->autoRowHeight(true);
}

CFontComboBox::CFontComboBox(const char * label,int layoutSize,CLayoutAlign layoutAlignment)
: CComboBox(label,layoutSize,layoutAlignment) {
   loadFonts();
}

#ifdef __COMPATIBILITY_MODE__ 
CFontComboBox::CFontComboBox(int x,int y,int w,int h,const char *l)
: CComboBox(x,y,w,h,l) {
   loadFonts();
}
#endif

CLayoutClient* CFontComboBox::creator(xml::Node *node) {
    auto widget = new CFontComboBox("",10,SP_ALIGN_TOP);
    widget->load(node,LXM_LAYOUTDATA);
    return widget;
}
