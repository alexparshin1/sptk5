/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/Variant.h>
#include <sptk5/db/QueryParameterList.h>
#include <sptk5/wsdl/WSArray.h>
#include <sptk5/wsdl/WSBasicTypes.h>
#include <sptk5/wsdl/WSFieldIndex.h>
#include <sptk5/xdoc/Node.h>

namespace sptk {
/**
 * @addtogroup wsdl WSDL-related Classes.
 * @{
 */

/**
 * @brief Base type for all user WSDL types.
 */
class SP_EXPORT WSComplexType
    : public WSType
{
public:
    /**
     * @brief Default constructor.
     * @param name              Element name.
     * @param optional          Element optionality flag.
     */
    explicit WSComplexType(const char* name, const bool optional = false)
        : WSType(name)
        , m_optional(optional)
    {
    }

    /**
     * @brief Copy constructor.
     * @param other             Other object.
     */
    WSComplexType(const WSComplexType& other) = default;

    /**
     * @brief Move constructor.
     * @param other             Other object.
     */
    WSComplexType(WSComplexType&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    ~WSComplexType() override = default;

    /**
     * @brief Copy assignment.
     * @param other             Other object.
     */
    WSComplexType& operator=(const WSComplexType& other) = default;

    /**
     * @brief Move assignment.
     * @param other             Other object.
     */
    WSComplexType& operator=(WSComplexType&& other) noexcept = default;

    /**
     * @brief Return class name.
     */
    [[nodiscard]] String className() const override
    {
        return "WSComplexType";
    }

    /**
     * @brief Clear content and releases allocated memory.
     */
    void clear() override
    {
        _clear();
    }

    /**
     * @brief Copy data from another object.
     * @param other             Object to copy from.
     */
    [[maybe_unused]] void copyFrom(const WSComplexType& other);

    /**
     * @brief Loads type data from request XML node.
     * @param input             XML node.
     * @param nullLargeData     Set null for elements with data size > 256 bytes.
     */
    void load(const xdoc::SNode& input, bool nullLargeData = false) override;

    /**
     * @brief Load data from FieldList.
     *
     * Only simple WSDL type members are loaded.
     * @param input             Query field list containing CMqType data.
     * @param nullLargeData     Set null for elements with data size > 256 bytes.
     */
    virtual void load(const FieldList& input, bool nullLargeData = false);

    /**
     * @brief Unload data to existing XML node.
     * @param output            Existing XML node.
     */
    virtual void unload(const xdoc::SNode& output) const;

    /**
     * @brief Unload data to Query's parameters.
     * @param output            Query parameters.
     */
    virtual void unload(QueryParameterList& output) const;

    /**
     * @brief Unload a single element or attribute to a DB query parameter.
     * @param output            Query parameters.
     * @param paramName         Quermy parameter name.
     * @param elementOrAttribute Complex type element (not an array!).
     */
    static void unload(QueryParameterList& output, const char* paramName, const WSBasicType* elementOrAttribute);

    /**
     * @brief Unload data to new XML node.
     * @param parent            Parent XML node where the new node is created.
     * @param name              Optional name for the child element.
     */
    void exportTo(const xdoc::SNode& parent, const char* name = nullptr) const override;

    /**
     * @brief True if data was not loaded, or if all the fields are null.
     */
    [[nodiscard]] bool isNull() const override;

    /**
     * @brief True is element is optional.
     */
    [[nodiscard]] virtual bool isOptional() const
    {
        return m_optional;
    }

    /**
     * @brief Print element as XML text.
     * @param asJSON            Output is JSON (true) or XML (false).
     * @param formatted         Formatted output flag.
     * @return object presentation as JSON or XML string.
     */
    [[nodiscard]] virtual String toString(bool asJSON = true, bool formatted = false) const;

    /**
     * @brief Throw SOAPException is the object is null.
     * @param parentTypeName    Parent object type name.
     */
    void throwIfNull(const String& parentTypeName) const;

    /**
     * @brief If the object is exportable, it's included during export to JSON or XML.
     * @param flag              Exportable flag.
     */
    [[maybe_unused]] void exportable(const bool flag)
    {
        m_exportable = flag;
    }

    /**
     * @brief Get the field pointer by the name.
     * @param fieldName Field name.
     * @return Field pointer.
     */
    [[nodiscard]] WSType* getField(const String& fieldName) const
    {
        auto* field = m_fields.find(fieldName);
        if (field == nullptr)
        {
            throw Exception("Field '" + fieldName + "' not found");
        }
        return field;
    }

    /**
     * @brief Check if the class restrictions are violated.
     */
    virtual void checkRestrictions() const
    {
        // Implement in derived class
    }

protected:
    /**
     * @return true if the object is loaded.
     */
    [[nodiscard]] bool loaded() const
    {
        return m_loaded;
    }

    /**
     * @brief Set loaded flag.
     * @param flag              If it is true, then the object is loaded.
     */
    void setLoaded(const bool flag)
    {
        m_loaded = flag;
    }

    /**
     * @brief Internal clear data.
     */
    virtual void _clear()
    {
        m_fields.forEach([](WSType* field)
                         {
                             field->clear();
                             return true;
                         });
    }

    [[nodiscard]] const WSFieldIndex& getFields() const
    {
        return m_fields;
    }

    void setElements(const Strings& fieldNames, const std::initializer_list<WSType*> fields)
    {
        m_fields.setElements(fieldNames, fields);
    }

    void setAttributes(const Strings& fieldNames, const std::initializer_list<WSType*> fields)
    {
        m_fields.setAttributes(fieldNames, fields);
    }

private:
    bool         m_optional {false};  ///< Element optionality flag.
    bool         m_loaded {false};    ///< Is data loaded flag.
    bool         m_exportable {true}; ///< Is this object exportable?.
    WSFieldIndex m_fields;            ///< All fields.

    static bool loadField(const FieldList& input, bool nullLargeData, WSType* field);
};

/**
 * @}
 */

} // namespace sptk
