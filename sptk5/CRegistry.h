/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRegistry.h  -  description
                             -------------------
    begin                : Tue Mar 26 2002
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

#ifndef __CREGISTRY_H__
#define __CREGISTRY_H__

#include <sptk5/sptk.h>
#include <sptk5/CStrings.h>
#include <sptk5/cxml>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

class CRegistry;

/// @brief Registry modes
///
/// Modes for the registry. User registry is stored in home directory
/// (if home directory is avalable). Program registry is stored in the program's
/// startup directory.
enum CRegistryMode
{
    USER_REGISTRY = 1,
    PROGRAM_REGISTRY = 2
};


/// @brief Registry
///
/// Works with INI and XML configuration-files.
/// Class allows to read both INI and XML files and write the output file
/// in the same format, or change the output format before saving the file.
class SP_EXPORT CRegistry: public CXmlDoc
{
    std::string m_fileName;            ///< The registry file name

    /// @brief Prepares program registry directory
    void prepareDirectory();

    /// @brief Saves a node into CStrings
    ///
    /// The node is saved in INI file format.
    /// @param outputData CStrings&, the string list to save data into
    /// @param node CXmlNode*, the XML node to save
    /// @param currentPath string, current path to the parent node
    void save(CStrings& outputData, CXmlNode* node, std::string currentPath);

    /// @brief Cleans the node recursively
    ///
    /// Removes the empty children nodes. The empty nodes are nodes without children and attributes.
    /// @param node CXmlNode*, the node to clean
    void clean(CXmlNode* node);

public:

    /// @brief Constructor
    ///
    /// @param fileName std::string, the registry file name w/o path
    /// @param programGroupName std::string, the name of the program group to generate a directory name for the registry files.
    /// Should be a single phrase without '\\' or '/'
    /// @param mode CRegistryMode, see CRegistryMode for details
    CRegistry(std::string fileName, std::string programGroupName, CRegistryMode mode = USER_REGISTRY);

    /// @brief Destructor
    virtual ~CRegistry();

    /// @brief Sets the registry file name
    void fileName(std::string fname)
    {
        m_fileName = fname;
    }

    /// @brief Returns the registry file name
    const std::string fileName()
    {
        return m_fileName;
    }

    /// @brief Loads registry from the file. Usually it's the first action with registry.
    virtual void load();

    /// @brief Loads registry from the string list
    virtual void load(const CStrings& data);

    /// @brief Loads registry from buffer
    virtual void load(const char* data);

    /// @brief Loads registry from XML node
    ///
    /// Any XML node that has subnodes is considered as section.
    /// Nested sections make paths with the elements separated with "/".
    /// @param data const CXmlDoc&, the XML document to load data from
    virtual void load(const CXmlDoc& data);

    /// @brief Loads registry from buffer.
    /// @param buffer const CBuffer&, source buffer
    virtual void load(const CBuffer &buffer)
    {
        clear();
        CXmlDoc::load(buffer);
    }

    /// @brief Saves registry to the file.
    virtual void save();

    /// @brief Saves registry to the the string list
    virtual void save(CStrings& data);

    /// @brief Saves registry to XML node
    ///
    /// Nested sections with paths with the elements separated with "/" make nested XML nodes.
    /// @param data const CXmlDoc&, the XML document to load data from
    virtual void save(CXmlDoc& data) const;

    /// @brief Saves registry to buffer.
    /// @param buffer CBuffer&, a buffer to save document
    /// @param indent int, how many indent spaces at start
    virtual void save(CBuffer &buffer, int indent = 0) const
    {
        CXmlDoc::save(buffer, indent);
    }

public:

    /// Finds out the user's home directory
    static std::string homeDirectory();
};
/// @}
}
#endif
