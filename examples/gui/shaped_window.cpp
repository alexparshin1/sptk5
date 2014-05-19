/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          shaped_window.cpp  -  description
                             -------------------
    begin                : January 3, 2003
    copyright            : (C) 1999-2014 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.
 
   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 
   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

/***************************************************************************
   Annotation:
   This example source code demonstrates, how to define a non-rectangular
   window and make it work properly.
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/Fl.H>

#include <sptk5/cgui>
#include <math.h>
#include <iostream>

using namespace std;
using namespace sptk;

class CShapedWindow : public CWindow {
   CBox*     m_captionBox;
   CButton*  m_closeButton;
   CButton*  m_maximizeButton;
   CButton*  m_minimizeButton;
   bool      m_maximized;
   int       m_restoreX;
   int       m_restoreY;
   int       m_restoreW;
   int       m_restoreH;
   void  appendSector(int x, int y, int r, int a1, int a2, int da);
public:
   CShapedWindow(int x, int y, int w, int h, const char* label="");
   virtual void shapeResize(int ww, int hh); // This method should define points for the window shape
   
   void minimize();
   void maximize();
   void restore();
};

void exit_cb(Fl_Widget *w, void *) {
   w->window()->hide();
}

void theme_cb(Fl_Widget *w, void *) {
   try {
      CComboBox *themesCombo = (CComboBox *)w;
      std::string themeName = themesCombo->data();
      
      if (themesCombo->eventType() == CE_DATA_CHANGED) {
         CThemes::set
         (themeName);
         
         CWindow *window = (CWindow *)w->window();
         window->relayout();
         window->redraw();
      }
   } catch (exception& e) {
      spError(e.what());
   }
}

void close_cb(Fl_Widget *w, void *) {
   w->window()->hide();
}

void maximize_cb(Fl_Widget *w, void *) {
   CShapedWindow* shapedWindow = (CShapedWindow*) w->window();
   shapedWindow->maximize();
}

void minimize_cb(Fl_Widget *w, void *) {
   CShapedWindow* shapedWindow = (CShapedWindow*) w->window();
   shapedWindow->minimize();
}

CShapedWindow::CShapedWindow(int x, int y, int w, int h, const char* label) : CWindow(x, y, w, h, label) {
   m_maximized = false;
   
   box(FL_FLAT_BOX);
   //clear_border();
   
   CGroup* captionGroup = new CGroup;
   captionGroup->layoutSpacing(1);
   captionGroup->box(FL_DOWN_BOX);
   captionGroup->color(FL_BLUE);
   m_captionBox = new CBox("Shaped Window", 10, SP_ALIGN_CLIENT);
   m_captionBox->labelcolor(FL_WHITE);
   m_captionBox->labelcolor(FL_WHITE);
   m_captionBox->dragable(true);
   
   m_closeButton = new CButton("X");
   m_closeButton->labelfont(FL_HELVETICA_BOLD);
   m_closeButton->callback(close_cb);
   
   m_maximizeButton = new CButton("@-4square");
   m_maximizeButton->labeltype(FL_ENGRAVED_LABEL);
   m_maximizeButton->callback(maximize_cb);
   
   m_minimizeButton = new CButton("@-22>");
   m_minimizeButton->labeltype(FL_ENGRAVED_LABEL);
   m_minimizeButton->callback(minimize_cb);
   
   captionGroup->end();
   
   CToolBar* toolBar = new CToolBar;
   new CButton("Test");
   toolBar->end();
   
   // That group keeps togeteher the buttons. These
   // buttons use the default alignment for buttons -
   // SP_ALIGN_RIGHT, and the text/icon defined by the
   // button kind.
   CGroup* buttonGroup = new CGroup("", 10, SP_ALIGN_BOTTOM);
   buttonGroup->color(FL_LIGHT1);
   
   CButton* exitButton = new CButton(SP_EXIT_BUTTON);
   exitButton->callback(exit_cb);
   
   CComboBox* themesCombo = new CComboBox("Theme", 200, SP_ALIGN_LEFT);
   CStrings themes = CThemes::availableThemes();
   themesCombo->addRows("Theme", themes);
   themesCombo->callback(theme_cb);
   themesCombo->data("Default");
   
   buttonGroup->end();
   
   end();
   
   initShapeExtension();
   
   resizable(this);
   relayout();
}

void CShapedWindow::appendSector(int xc, int yc, int r, int a1, int a2, int da) {
   for (int angle = a1; angle != a2; angle += da) {
      double angle_r = angle / 180.0 * 3.1415926;
      int dx = int(r * cos(angle_r) + 0.5);
      int dy = int(r * sin(angle_r) + 0.5);
      m_shapePoints.push_back(CShapePoint(xc+dx, yc-dy));
   }
}

void CShapedWindow::shapeResize(int ww, int hh) {
   m_shapePoints.clear();
   int dx = 15;
   appendSector(dx, dx, dx, 180, 90, -15);
   appendSector(ww-dx, dx, dx, 90, 0, -15);
   appendSector(ww-dx, hh-dx, dx, 0, -90, -15);
   appendSector(dx, hh-dx, dx, -90, -180, -15);
}

void CShapedWindow::maximize() {
   if (!m_maximized) {
      m_maximized = true;
      m_maximizeButton->label("@-6menu");
      m_restoreX = x();
      m_restoreY = y();
      m_restoreW = w();
      m_restoreH = h();
      resize(Fl::x(), Fl::y()+40, Fl::w(), Fl::h()-80);
   } else
      restore();
}

void CShapedWindow::minimize() {
   m_restoreX = x();
   m_restoreY = y();
   m_restoreW = w();
   m_restoreH = h();
   iconize();
}

void CShapedWindow::restore() {
   m_maximized = false;
   m_maximizeButton->label("@-4square");
   resize(m_restoreX, m_restoreY, m_restoreW, m_restoreH);
   relayout();
   redraw();
}

int main(int argc, char *argv[]) {
    // Initialize themes
    CThemes themes;

    try {
        CShapedWindow* w = new CShapedWindow(100, 100, 300, 300);
      
        w->show(argc, argv);
      
        Fl::run();
    } catch (exception& e) {
        spError(e.what());
    } catch (...) {
        spError("Unknown Error");
    }
   
    return 0;
}
