/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDataSource.cpp  -  description
                             -------------------
    begin                : Jun 20 2003
    copyright            : (C) 1999-2008 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CDataSource.h>

using namespace std;
using namespace sptk;

bool CDataSource::load() {
   // Loading data into DS
   return loadData();
}

bool CDataSource::save() { 
   // Storing data from DS
   return saveData();
}

void CDataSource::rowToXML(CXmlNode& node,bool compactXmlMode) const {
   uint32_t cnt = fieldCount();
   for (uint32_t i = 0; i < cnt; i++) {
     const CField& field = operator[](i);
     field.toXML(node,compactXmlMode);
   }
}

void CDataSource::toXML(CXmlNode& parentNode,std::string nodeName,bool compactXmlMode) {
   try {
      open();
      while (!eof()) {
         CXmlNode& node = *(new CXmlElement(parentNode,nodeName));
         rowToXML(node,compactXmlMode);
         next();
      }
      close();
   }
   catch (...) {
      close();
      throw;
   }
}
