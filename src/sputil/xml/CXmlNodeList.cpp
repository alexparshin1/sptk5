/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlNodeList.cpp  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 1999-2016 by Alexey S.Parshin
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
#include <sptk5/cxml>

using namespace std;
using namespace sptk;

void CXmlNodeList::clear()
{
    for (iterator itor = begin(); itor != end(); itor++)
        delete *itor;
    CXmlNodeVector::clear();
}

CXmlNodeList::iterator CXmlNodeList::findFirst(const char* nodeName)
{
    iterator itor;
    for (itor = begin(); itor != end(); itor++) {
        CXmlNode *anode = *itor;
        if (anode->name() == nodeName)
            break;
    }
    return itor;
}

CXmlNodeList::iterator CXmlNodeList::findFirst(const string& nodeName)
{
    iterator itor;
    for (itor = begin(); itor != end(); itor++) {
        CXmlNode *anode = *itor;
        if (anode->name() == nodeName)
            break;
    }
    return itor;
}

CXmlNodeList::const_iterator CXmlNodeList::findFirst(const char* nodeName) const
{
    const_iterator itor;
    for (itor = begin(); itor != end(); itor++) {
        CXmlNode *anode = *itor;
        if (anode->name() == nodeName)
            break;
    }
    return itor;
}

CXmlNodeList::const_iterator CXmlNodeList::findFirst(const string& nodeName) const
{
    const_iterator itor;
    for (itor = begin(); itor != end(); itor++) {
        CXmlNode *anode = *itor;
        if (anode->name() == nodeName)
            break;
    }
    return itor;
}
