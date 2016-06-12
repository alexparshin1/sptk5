/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFieldList.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <string.h>

#include <sptk5/Exception.h>
#include <sptk5/CFieldList.h>

using namespace std;
using namespace sptk;

CFieldList::CFieldList(bool indexed,bool compactXmlMode)  { 
   m_userData = 0;
   m_compactXmlMode = compactXmlMode;
   if (indexed)
      m_index = new CFieldMap;
   else 
      m_index = 0L;
}

CFieldList::~CFieldList() { 
   clear(); 
   if (m_index) 
      delete m_index; 
}

void CFieldList::clear() {
   uint32_t cnt = (uint32_t) m_list.size();
   if (cnt) {
       for (uint32_t i = 0; i < cnt; i++) 
          delete m_list[i];

       m_list.clear();
       if (m_index)
          m_index->clear();
   }
}

CField& CFieldList::push_back(const char *fname,bool checkDuplicates) {
   bool duplicate = false;
   if (checkDuplicates) {
       if (m_index) {
          CFieldMap::iterator itor = m_index->find(fname);
          if (itor != m_index->end())
             duplicate = true;
       } else {
          try {
             CField *pfld = fieldByName(fname);
             duplicate = (pfld != 0L);
          }
          catch (...) {
          }
       }
   }

   if (duplicate)
      throw Exception("Attempt to duplicate field name");

   CField *field = new CField(fname);

   m_list.push_back(field);

   if (m_index)
      (*m_index)[fname] = field;

   return *field;
}

CField& CFieldList::push_back(CField *field) {
   m_list.push_back(field);

   if (m_index)
      (*m_index)[field->m_name] = field;

   return *field;
}

CField *CFieldList::fieldByName(const char *fname) const {
   if (m_index) {
      CFieldMap::const_iterator itor = m_index->find(fname);
      if (itor != m_index->end())
         return itor->second;
   } else {
      uint32_t cnt = (uint32_t) m_list.size();
      for (uint32_t i = 0; i < cnt; i++) {
         CField *field = (CField *)m_list[i];
         if (strcmp(field->m_name.c_str(),fname) == 0)
            return field;
      }
   }
   throw Exception("Field name '" + std::string(fname) + "' not found");
}

void CFieldList::toXML(CXmlNode& node) const {
   CFieldVector::const_iterator itor = m_list.begin();
   CFieldVector::const_iterator iend = m_list.end();
   for ( ; itor != iend; itor++) {
     CField *field = *itor;
     field->toXML(node,m_compactXmlMode);
   }
}

CField& CFieldList::next() {
   CField *currentField = *m_fieldStreamItor;
   m_fieldStreamItor++;
   if (m_fieldStreamItor == m_list.end()) m_fieldStreamItor = m_list.begin();
   return *currentField;
}

CFieldList& operator >> (CFieldList& fieldList, string& data) {
   data = fieldList.next().asString();
   return fieldList;
}

CFieldList& operator >> (CFieldList& fieldList, int& data) {
   data = fieldList.next();
   return fieldList;
}

CFieldList& operator >> (CFieldList& fieldList, double& data) {
   data = fieldList.next();
   return fieldList;
}

CFieldList& operator >> (CFieldList& fieldList, CDateTime& data) {
   data = fieldList.next();
   return fieldList;
}

CFieldList& operator >> (CFieldList& fieldList, CBuffer& data) {
   data = fieldList.next();
   return fieldList;
}

CFieldList& operator >> (CFieldList& fieldList, bool& data) {
   data = fieldList.next().asBool();
   return fieldList;
}

CFieldList& operator >> (CFieldList& fieldList, CXmlNode& fields) {
   fieldList.toXML(fields);
   return fieldList;
}
