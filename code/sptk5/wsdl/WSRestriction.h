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

#include <sptk5/RegularExpression.h>
#include <sptk5/Variant.h>
#include <sptk5/xdoc/Node.h>

namespace sptk {
/**
 * @brief WSDL Restriction.
 */
class SP_EXPORT WSRestriction
{
public:
    enum class Type : uint8_t
    {
        Unknown,
        Enumeration,
        Pattern
    };

    /**
     * @brief Constructor from WSDL (XML) definition.
     * @param typeName                  WSDL type name.
     * @param simpleTypeElement         Simple type XML node.
     */
    WSRestriction(String typeName, const xdoc::SNode& simpleTypeElement);

    /**
     * @brief Constructor from WSDL (XML) definition.
     * @param type                      Restriction type.
     * @param wsdlTypeName              WSDL type name.
     * @param enumerationsOrPatterns    Enumerations or patterns.
     */
    WSRestriction(Type type, String wsdlTypeName, const Strings& enumerationsOrPatterns);

    /**
     * @brief Get restriction type.
     * @return restriction type.
     */
    [[nodiscard]] Type type() const;

    /**
     * @brief Restriction check.
     *
     * Checks value to satisfy restriction.
     * If value violates restriction, throws exception.
     * @param typeName      Name of the checked type (for error messages).
     * @param value         Value to check.
     */
    void check(const String& typeName, const String& value) const;

    /**
     * @brief Generates restriction constructor for C++ skeleton.
     */
    [[nodiscard]] String generateConstructor(const String& variableName) const;

    /**
     * @brief Optional regular expression to match.
     * @return regular expression string.
     */
    [[nodiscard]] const std::vector<RegularExpression>& patterns() const
    {
        return m_patterns;
    }

    /**
     * @brief Optional enumeration to match.
     * @return enumeration.
     */
    [[nodiscard]] Strings enumeration() const
    {
        return m_enumeration;
    }

private:
    Type                           m_type {Type::Unknown}; ///< Restriction type.
    String                         m_wsdlTypeName;         ///< WSDL type name.
    Strings                        m_enumeration;          ///< List of enumerations, if any.
    std::vector<RegularExpression> m_patterns;             ///< Patterns.
};

using SWSRestriction = std::shared_ptr<WSRestriction>;
} // namespace sptk
