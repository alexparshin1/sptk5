/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          font_combo.cpp  -  description
                             -------------------
    begin                : Sun Jun 8, 2003
    copyright            : (C) 1999-2013 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/Fl.H>
#include <stdio.h>

#include <sptk5/cgui>

using namespace sptk;

CMemoInput *memoInput;

void font_cb(Fl_Widget *fc, void *data) {
   CFontComboBox *fontCombo = dynamic_cast<CFontComboBox *>(fc);
   if (!fontCombo) return;
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
   
    Fl::run();
   
    return 0;
}
