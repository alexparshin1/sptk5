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

#include <sptk5/Buffer.h>
#include <sptk5/DateTime.h>
#include <sptk5/Variant.h>
#include <sptk5/xdoc/Node.h>

#include <string>

namespace sptk {
/**
 * @addtogroup utility Utility Classes.
 * @{.
 */

class Query;

class FieldList;

/**
 * @brief Data field for CDataSource.
 *
 * Contains field name, field type, field data, and field format information.
 */
class SP_EXPORT Field
    : public Variant
{
    friend class FieldList;

public:
    /**
     * @brief Combination of field view attributes.
     */
    struct View
    {
        signed int width : 10;    ///< Field width.
        unsigned   precision : 5; ///< Field precision.
        unsigned   flags : 16;    ///< Field flags like alignment, etc.
        bool       visible : 1;   ///< Is field visible?
    };

    /**
     * @brief Constructor.
     * @param name               Field name.
     */
    explicit Field(const String& name);

    /**
     * @brief Copy constructor.
     * @param other              Another field object.
     */
    Field(const Field& other) = default;

    /**
     * @brief Move constructor.
     * @param other              Another field object.
     */
    Field(Field&& other) noexcept = default;

    ~Field() noexcept override = default;

    /**
     * @brief Combination of field view attributes.
     */
    View& view()
    {
        return m_view;
    };

    /**
     * @brief Returns field name.
     */
    [[nodiscard]] const String& fieldName() const
    {
        return m_name;
    }

    /**
     * @brief Copy assignment operation.
     */
    Field& operator=(const Field& other) = default;

    /**
     * @brief Move assignment operation.
     */
    Field& operator=(Field&& other) noexcept = default;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const Variant& C) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const bool value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const int32_t value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const int64_t value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const double value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const char* value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const String& value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const DateTime& value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const MoneyData& value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const uint8_t* value) override;

    /**
     * @brief Assignment operation.
     */
    Field& operator=(const Buffer& value) override;

    /**
     * @brief Better (than in base class) conversion method.
     */
    [[nodiscard]] String asString() const override;

    /**
     * @brief Exports the field data into the XML node.
     *
     * If the compactXmlMode flag is true, the field is exported as an attribute.
     * Otherwise, the field is exported as subnodes.
     * For the fields of the VAR_TEXT type, the subnode is created containing the CDATA section.
     * @param node              Node to export field data into.
     * @param compactXmlMode    When exporting to XML, export fields as attributes.
     * @param detailedInfo      Export extra field info such as size and type.
     * @param nullLargeData     Set text data longer than 256 bytes to null.
     */
    void exportTo(const xdoc::SNode& node, bool compactXmlMode, bool detailedInfo = false, bool nullLargeData = false) const;

    [[nodiscard]] String displayName() const
    {
        return m_displayName;
    }

    void displayName(const String& name)
    {
        m_displayName = name;
    }

protected:
    [[nodiscard]] virtual String doubleDataToString() const;

private:
    String m_name;        ///< Field name.
    View   m_view {};     ///< Combination of field view attributes.
    String m_displayName; ///< Optional display field name.

    [[nodiscard]] String epochDataToDateTimeString(bool dateOnly) const;
};

using SField = std::shared_ptr<Field>;

/**
 * @}.
 */
} // namespace sptk
