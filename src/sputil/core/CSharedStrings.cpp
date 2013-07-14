/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSharedStrings.cpp  -  description
                             -------------------
    begin                : June 1 2006
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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
#include <sptk5/CException.h>
#include <sptk5/CSharedStrings.h>
#include <sptk5/string_ext.h>

using namespace sptk;

CSharedStrings::CSharedStrings() {
    shareString("");
}

const std::string& CSharedStrings::shareString(const char* str) {
    CSIMap::iterator itor = m_stringIdMap.find(str);
    if (itor == m_stringIdMap.end()) {
        std::pair<CSIMap::iterator,bool> insertResult;
        insertResult = m_stringIdMap.insert(CSIMap::value_type(str,0));
        itor = insertResult.first;
    }
    itor->second++;
    return itor->first;
}

void CSharedStrings::releaseString(const char* str) throw(std::exception) {
    CSIMap::iterator itor = m_stringIdMap.find(str);
    if (itor == m_stringIdMap.end())
        throw CException("The string "+std::string(str)+" isn't registered in SST");
    if (itor->second > 1)
        itor->second--;
    else
        m_stringIdMap.erase(itor);
}

void CSharedStrings::clear() {
    m_stringIdMap.clear();
    shareString("");
}
