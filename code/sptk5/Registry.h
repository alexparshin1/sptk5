/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Registry.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_REGISTRY_H__
#define __SPTK_REGISTRY_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>
#include <sptk5/cxml>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

class Registry;

/**
 * @brief Registry modes
 *
 * Modes for the registry. User registry is stored in home directory
 * (if home directory is avalable). Program registry is stored in the program's
 * startup directory.
 */
enum RegistryMode
{
    USER_REGISTRY = 1,
    PROGRAM_REGISTRY = 2
};


/**
 * @brief Registry
 *
 * Works with INI and XML configuration-files.
 * Class allows to read both INI and XML files and write the output file
 * in the same format, or change the output format before saving the file.
 */
class SP_EXPORT Registry: public xml::Document
{
    /**
     * The registry file name
     */
    String m_fileName;


    /**
     * @brief Prepares program registry directory
     */
    void prepareDirectory();

    /**
     * @brief Saves a node into Strings
     *
     * The node is saved in INI file format.
     * @param outputData CStrings&, the string list to save data into
     * @param node xml::Node*, the XML node to save
     * @param currentPath string, current path to the parent node
     */
    void save(Strings& outputData, Node* node, std::string currentPath);

    /**
     * @brief Cleans the node recursively
     *
     * Removes the empty children nodes. The empty nodes are nodes without children and attributes.
     * @param node xml::Node*, the node to clean
     */
    void clean(Node* node);

public:

    /**
     * @brief Constructor
     *
     * @param fileName const std::string&, the registry file name w/o path
     * @param programGroupName std::string, the name of the program group to generate a directory name for the registry files.
     * Should be a single phrase without '\\' or '/'
     * @param mode RegistryMode, see RegistryMode for details
     */
    Registry(const String& fileName, const String& programGroupName, RegistryMode mode = USER_REGISTRY);

    /**
     * @brief Destructor
     */
    virtual ~Registry();

    /**
     * @brief Sets the registry file name
     */
    void fileName(const String& fname)
    {
        m_fileName = fname;
    }

    /**
     * @brief Returns the registry file name
     */
    const std::string fileName()
    {
        return m_fileName;
    }

    /**
     * @brief Loads registry from the file. Usually it's the first action with registry.
     */
    virtual void load();

    /**
     * @brief Loads registry from the string list
     * @param data const Strings&, source string
     */
    virtual void load(const Strings& data);

    /**
     * @brief Loads registry from std::string.
     * @param data const std::string&, source string
     */
    virtual void load(const std::string& data)
    {
        load(data.c_str());
    }

    /**
     * @brief Loads registry from buffer
     * @param data const char*, source string
     */
    virtual void load(const char* data);

    /**
     * @brief Loads registry from XML node
     *
     * Any XML node that has subnodes is considered as section.
     * Nested sections make paths with the elements separated with "/".
     * @param data const XMLDoc&, the XML document to load data from
     */
    virtual void load(const xml::Document& data);

    /**
     * @brief Loads registry from buffer.
     * @param buffer const CBuffer&, source buffer
     */
    virtual void load(const Buffer &buffer)
    {
        clear();
        xml::Document::load(buffer);
    }

    /**
     * @brief Saves registry to the file.
     */
    virtual void save();

    /**
     * @brief Saves registry to the the string list
     */
    virtual void save(Strings& data);

    /**
     * @brief Saves registry to XML node
     *
     * Nested sections with paths with the elements separated with "/" make nested XML nodes.
     * @param data const XMLDoc&, the XML document to load data from
     */
    virtual void save(xml::Document& data) const;

    /**
     * @brief Saves registry to buffer.
     * @param buffer CBuffer&, Output buffer to save document
     * @param formatXML bool, If true then XML is formatted with indentation
     */
    virtual void save(Buffer &buffer, bool formatXML=false) const
    {
        xml::Document::save(buffer, formatXML);
    }

    /**
     * @brief Saves registry to buffer.
     * @param buffer CBuffer&, Output buffer to save document
     * @param indent int, formatting indent (spaces)
     */
    virtual void save(Buffer &buffer, int indent) const
    {
        xml::Document::save(buffer, indent);
    }

public:

    /**
     * Get current user home directory
	 * @return current user home directory
     */
    static String homeDirectory();
};
/**
 * @}
 */
}
#endif
