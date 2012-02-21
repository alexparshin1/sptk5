/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          clistviewrows.cpp  -  description
                             -------------------
    begin                : Tue Jul 23 2002
    copyright            : (C) 2002-2012 by Alexey Parshin. All rights reserved.
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <sptk5/gui/CListViewRows.h>
#include <sptk5/CException.h>

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <algorithm>

using namespace std;
using namespace sptk;

CListViewRows::CListViewRows() {
   m_sortColumn = -1;
   m_sortAscending = true;
   m_sortColumnType = VAR_STRING;
   m_fullHeight = 0;
}

CListViewRows::~CListViewRows() {
   clear();
}

void CListViewRows::truncate(unsigned cnt) {
   unsigned rowCount = m_rows.size();
   if (cnt < rowCount) {
      for (unsigned i = cnt; i < rowCount; i++) {
         CPackedStrings *row = (CPackedStrings *)m_rows[i];
         m_fullHeight -= row->height;
         delete (CPackedStrings *)row;
      }
      m_rows.resize(cnt);
   }
}

unsigned CListViewRows::add(CPackedStrings *ss) {
   int lineNumber = size();
   m_rows.push_back(ss);
   m_fullHeight += ss->height;
   return lineNumber;
}

unsigned CListViewRows::insert(unsigned position,CPackedStrings *ss) {
   m_rows.insert(m_rows.begin()+position,ss);
   m_fullHeight += ss->height;
   return position;
}

unsigned CListViewRows::update(unsigned index,CPackedStrings *ss) {
   CPackedStrings *s = (CPackedStrings *)m_rows[index];
   int oldh = s->height;
   if (s) delete s;
   m_fullHeight += ss->height - oldh;
   m_rows[index] = ss;
   return index;
}

void CListViewRows::clear() {
   for (unsigned i = 0; i < m_rows.size(); i++)
      delete (CPackedStrings *) m_rows[i];
   m_rows.clear();
   m_fullHeight = 0;
}

void CListViewRows::remove(unsigned index) {
   if (index < m_rows.size()) {
      CPackedStrings *row = (CPackedStrings *)m_rows[index];
      m_fullHeight -= row->height;
      delete row;
      m_rows.erase(m_rows.begin()+index);
   }
}

int CListViewRows::currentSortColumn;

bool CListViewRows::compare_strings(const PPackedStrings&a,const PPackedStrings&b) {
   return strcmp((*a)[currentSortColumn],(*b)[currentSortColumn]) < 0;
}

bool CListViewRows::compare_integers(const PPackedStrings&a,const PPackedStrings&b) {
   int i1 = atoi((*a)[currentSortColumn]);
   int i2 = atoi((*b)[currentSortColumn]);
   return i1 < i2;
}

bool CListViewRows::compare_floats(const PPackedStrings&a,const PPackedStrings&b) {
   double d1 = atof((*a)[currentSortColumn]);
   double d2 = atof((*b)[currentSortColumn]);
   return d1 < d2;
}

bool CListViewRows::compare_dates(const PPackedStrings&a,const PPackedStrings&b) {
   CDateTime d1((*a)[currentSortColumn]);
   CDateTime d2((*b)[currentSortColumn]);
   return d1 < d2;
}

bool CListViewRows::compare_datetimes(const PPackedStrings&a,const PPackedStrings&b) {
   CDateTime d1((*a)[currentSortColumn]);
   CDateTime d2((*b)[currentSortColumn]);
   return d1 < d2;
}

void CListViewRows::sort() {
   fl_cursor(FL_CURSOR_WAIT);
   Fl::check();

   int m_size = m_rows.size();
   if (m_sortColumn >= 0 && m_size > 1) {
      CListViewRows::currentSortColumn = m_sortColumn;
      switch (m_sortColumnType) {
         case VAR_BOOL:
         case VAR_INT:
            std::sort(m_rows.begin(),m_rows.end(),compare_integers);
            break;
         case VAR_FLOAT:
            std::sort(m_rows.begin(),m_rows.end(),compare_floats);
            break;
         case VAR_DATE: 
            std::sort(m_rows.begin(),m_rows.end(),compare_dates);
            break;
         case VAR_DATE_TIME: 
            std::sort(m_rows.begin(),m_rows.end(),compare_datetimes);
            break;
         default:
            std::sort(m_rows.begin(),m_rows.end(),compare_strings);
            break;
      }
      if (!m_sortAscending) {
         // reversing sort order for the descending sort
         unsigned cnt = m_rows.size();
         unsigned mid = cnt / 2;
         unsigned j = cnt - 1;
         for (unsigned i = 0; i < mid; i++, j--) {
            CPackedStrings *item = m_rows[i];
            m_rows[i] = m_rows[j];
            m_rows[j] = item;
         }
      }
   }
   fl_cursor(FL_CURSOR_DEFAULT);
   Fl::check();
}

void CListViewRows::sortColumn(int column,CVariantType columnType,bool sortNow) {
   m_sortColumn = column;
   m_sortColumnType = columnType;
   if (sortNow) sort();
}

void CListViewRows::sortAscending(bool ascending,bool sortNow) {
   m_sortAscending = ascending;
   if (sortNow) sort();
}

int CListViewRows::indexOf(CPackedStrings * ss) const {
   CPSVector::const_iterator itor = find(m_rows.begin(),m_rows.end(),ss);
   if (itor == m_rows.end()) return -1;
   return distance(m_rows.begin(),itor);
}
