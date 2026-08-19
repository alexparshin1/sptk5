/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/FieldList.h>
#include <sptk5/Variant.h>

class Fl_Group;

namespace sptk {
/**
 * @addtogroup utility Utility Classes.
 * @{
 */

/**
 * @brief Universal data source for many SPTK widgets.
 *
 * It's designed as a base class for multiple datasources available in SPTK.
 * The main idea is to provide the simple interface that allows opening the datasource
 * with certain parameters and reading or writing the datasource fields. And don't you forget to close it :).
 */
class SP_EXPORT DataSource
{
    friend class Fl_Group;

public:
    /**
     * @brief Default constructor.
     */
    DataSource() = default;

    /**
     * @brief Destructor.
     */
    virtual ~DataSource() = default;

    /**
     * @brief Field access by the field index, non-const version.
     *
     * Purely virtual. Should be implemented in derived class.
     * @param fieldIndex        Field index.
     * @returns field reference.
     */
    virtual Field& operator[](size_t fieldIndex) = 0;

    /**
     * @brief Field access by the field name.
     *
     * Purely virtual. Should be implemented in derived class.
     * @param fieldName         Field name.
     * @returns field reference.
     */
    virtual Field& operator[](const String& fieldName) = 0;

    /**
     * @brief Returns field count in the datasource.
     *
     * Purely virtual. Should be implemented in derived class.
     * @returns field count.
     */
    virtual size_t fieldCount() const = 0;

    /**
     * @brief Returns record count in the datasource.
     *
     * Purely virtual. Should be implemented in derived class.
     * @returns record count.
     */
    virtual size_t recordCount() const = 0;

    /**
     * @brief Reads the field by name from the datasource.
     *
     * Purely virtual. Should be implemented in derived class.
     * @param fieldName         Field name.
     * @param fieldValue        Field value.
     */
    virtual bool readField(const char* fieldName, Variant& fieldValue) = 0;

    /**
     * @brief Writes the field by name from the datasource.
     *
     * Purely virtual. Should be implemented in derived class.
     * @param fieldName         Field name.
     * @param fieldValue        Field value.
     */
    virtual bool writeField(const char* fieldName, const Variant& fieldValue) = 0;

    /**
     * @brief Opens the datasource. Implemented in derved class.
     */
    virtual bool open() = 0;

    /**
     * @brief Closes the datasource. Implemented in derved class.
     */
    virtual bool close() = 0;

    /**
     * @brief Moves to the first record of the datasource. Implemented in derived class.
     */
    virtual bool first()
    {
        return false;
    }

    /**
     * @brief Moves to the next record of the datasource. Implemented in derved class.
     */
    virtual bool next() = 0;

    /**
     * @brief Moves to the prior record of the datasource. Implemented in derved class.
     */
    virtual bool prior()
    {
        return false;
    }

    /**
     * @brief Moves to the last record of the datasource. Implemented in derved class.
     */
    virtual bool last()
    {
        return false;
    }

    /**
     * @brief Moves to the specified record position of the datasource. Implemented in derved class.
     */
    virtual bool find(const String& /*fieldName*/, const Variant& /*position*/)
    {
        return false;
    }

    /**
     * @brief Returns true if there are no more records in the datasource. Implemented in derved class.
     */
    virtual bool eof() const = 0;

    /**
     * @brief Loads data into the datasource.
     */
    bool load();

    /**
     * @brief Saves data from the datasource.
     */
    bool save();

    /**
     * @brief Saves dataset row data into XDoc.
     *
     * If the compactXmlMode is true, the node would have fields presented as attributues.
     * Otherwise, the fields are stored as subnodes.
     * @param node              XDoc node to fill in.
     * @param compactXmlMode    Compact XML flag.
     * @param nullLargeData     Set text data longer than 256 bytes to null.
     */
    void exportRowTo(const xdoc::SNode& node, bool compactXmlMode, bool nullLargeData = false);

    /**
     * @brief Saves data into XDoc.
     *
     * Opens the dataset, reads every row, and closes dataset.
     * For every row in the dataset, creates the node with the name nodeName.
     * If the compactXmlMode is true, the nodes would have fields presented as attributues.
     * Otherwise, the fields are stored as subnodes.
     * @param parentNode        XDoc node to add subnodes to.
     * @param nodeName          Name for subnodes.
     * @param compactXmlMode    Compact XML flag.
     */
    virtual void exportTo(xdoc::Node& parentNode, const String& nodeName, bool compactXmlMode);

protected:
    /**
     * @brief Loads datasource data. Should be implemented in derived class.
     */
    virtual bool loadData()
    {
        return true;
    }

    /**
     * @brief Saves data from datasource. Should be implemented in derived class.
     */
    virtual bool saveData()
    {
        return true;
    }
};

/**
 * @}
 */
} // namespace sptk
