/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlNodeList.h  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CXML_NODELIST_H__
#define __CXML_NODELIST_H__

#include <string>
#include <vector>
#include <algorithm>

namespace sptk {

/// @addtogroup XML
/// @{

class CXmlNode;

/// @brief The vector of CXmlNode *
typedef std::vector<CXmlNode *> CXmlNodeVector;

/// @brief XML node list
///
/// The CXmlNodeList interface provides the an ordered collection of nodes,
/// The items in the NodeList are accessible via an integral index, starting from 0.
class SP_EXPORT CXmlNodeList : public CXmlNodeVector 
{
public:
    /// @brief Constructor
    CXmlNodeList()
    {}

    /// @brief Destructor
    ~CXmlNodeList()
    {
        clear();
    }

    /// @brief Clears the list of XML nodes and releases all the allocated memory
    void clear();

    /// @brief Finds the first node node in the list with the matching name
    /// @param nodeName const char*, a node name
    /// @returns node iterator, or end()
    iterator findFirst(const char* nodeName);

    /// @brief Finds the first node node in the list with the matching name
    /// @param nodeName const std::string&, a node name
    /// @returns node iterator, or end()
    iterator findFirst(const std::string& nodeName);

    /// @brief Finds the first node node in the list with the matching name
    /// @param nodeName const char*, a node name
    /// @returns node iterator, or end()
    const_iterator findFirst(const char* nodeName) const;

    /// @brief Finds the first node node in the list with the matching name
    /// @param nodeName const std::string&, a node name
    /// @returns node iterator, or end()
    const_iterator findFirst(const std::string& nodeName) const;
/*
    /// Replaces the item
    /// @param pos uint32_t, replace position
    /// @param item CXmlNode *, item to insert
    void replace(uint32_t pos, CXmlNode *item);

    /// Removes the item
    /// @param pos uint32_t, remove item position
    void remove(uint32_t pos);

    /// Removes the item
    /// @param item CXmlNode *, item to remove
    bool remove(CXmlNode *item);
*/
}
;
/// @}
}
#endif
