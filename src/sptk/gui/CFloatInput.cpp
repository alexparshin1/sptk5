/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CFloatInput.cpp  -  description
                             -------------------
    begin                : Fri Oct 27 2006
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#include <sptk5/sptk.h>

#include <FL/fl_draw.H>

#include <sptk5/gui/CFloatInput.h>
#include <sptk5/gui/CInput.h>

using namespace sptk;

CFloatInput::CFloatInput(const char *label,int layoutSize,CLayoutAlign layoutAlignment)
        : CInput(label,layoutSize,layoutAlignment) {
    controlType(FL_FLOAT_INPUT);
    m_minValue = m_maxValue = 0;
    m_decimalPlaces = 4;
}

#ifdef __COMPATIBILITY_MODE__
CFloatInput::CFloatInput(int x,int y,int w,int h,const char * label)
        : CInput(x,y,w,h,label) {
    controlType(FL_FLOAT_INPUT);
    m_minValue = m_maxValue = 0;
    m_decimalPlaces = 4;
}
#endif

CLayoutClient* CFloatInput::creator(CXmlNode *node) {
    CFloatInput* widget = new CFloatInput("",10,SP_ALIGN_TOP);
    widget->load(node,LXM_LAYOUTDATA);
    return widget;
}

void CFloatInput::save(CQuery *updateQuery) {
    if (!m_fieldName.length())
        return;
    CParam& param = updateQuery->param(m_fieldName.c_str());
    param.setFloat( data() );
}

bool CFloatInput::valid() const {
    if (m_limited) {
        double val = data();
        return val >= m_minValue && val <= m_maxValue;
    }
    return true;
}

void CFloatInput::setLimits(bool limited,double min,double max) {
    m_limited = limited;

    m_minValue = min;
    m_maxValue = max;
}
