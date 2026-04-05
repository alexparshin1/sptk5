/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#include <array>
#include <sptk5/db/QueryParameterBinding.h>

namespace sptk {

class QueryParameterList;

/**
 * @brief SQL query parameter.
 *
 * Simplifies the ODBC parameter binding.
 * Automatically handles most of the data conversions on assignments.
 */
class SP_EXPORT QueryParameter
    : public Variant
{
    friend class Query;

    friend class QueryParameterList;

public:
    /**
     * @brief Constructor.
     * @param name char *, parameter name.
     * @param isOutput bool, parameter binding type: input or output.
     */
    explicit QueryParameter(const char* name, bool isOutput = false);

    /**
     * @brief Constructor.
     * @param name              Parameter name.
     * @param isOutput          Parameter binding type: input or output.
     */
    explicit QueryParameter(std::string_view name, bool isOutput = false);

    /**
     * @brief Destructor.
     */
    ~QueryParameter() override = default;

    /**
     * @brief Adds internal parameter binding index.
     */
    void bindAdd(uint32_t bindIndex);

    /**
     * @brief Returns internal parameter binding count.
     */
    [[nodiscard]] uint32_t bindCount() const;

    /**
     * @brief Returns the parameter bing position by index in the binding list.
     */
    [[nodiscard]] uint32_t bindIndex(uint32_t index) const;

    /**
     * @brief Returns the internal small conversion buffer used to convert the date structure to SPTK.
     *
     * Please don't use it directly.
     */
    [[nodiscard]] uint8_t* conversionBuffer()
    {
        return m_timeData.data();
    }

    /**
     * @brief An integer reserved to callback parameter data length.
     */
    int64_t& callbackLength()
    {
        return m_callbackLength;
    }

    /**
     * @brief Returns parameter name.
     */
    [[nodiscard]] std::string name() const;

    /**
     * @brief Returns parameter name by const reference.
     */
    [[nodiscard]] const std::string& nameRef() const
    {
        return m_name;
    }

    /**
     * @brief Set parameter type to output.
     */
    void setOutput();

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(const Variant& param);

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(bool v) override
    {
        setBool(v);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(int16_t v)
    {
        setInteger(v);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(uint16_t v)
    {
        setInteger(v);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(int32_t v) override
    {
        setInteger(v);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(int64_t v) override
    {
        setInt64(v);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(double v) override
    {
        setFloat(v);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(const char* s) override
    {
        setString(s, 0);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(const sptk::String& s) override
    {
        setString(s.c_str(), s.length());
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(const DateTime& dt) override
    {
        setDateTime(dt);
        return *this;
    }

    /**
     * @brief Assign operator.
     */
    QueryParameter& operator=(const MoneyData& value) override
    {
        setMoney(value.quantity, value.scale);
        return *this;
    }

    /**
     * @brief Assignment operator.
     */
    QueryParameter& operator=(const uint8_t* value) override
    {
        setImagePtr(value);
        return *this;
    }

    /**
     * @brief Assign operator.
     * @param buffer             Data buffer.
     */
    QueryParameter& operator=(const Buffer& buffer) override
    {
        setBuffer(buffer.data(), buffer.bytes(), VariantDataType::VAR_BUFFER);
        return *this;
    }

    /**
     * @brief String assignment method.
     *
     * In contrast to the Variant::setString() method, this method tries not to decrease the allocated buffer.
     * @param value const char*, string to assign.
     */
    virtual void setString(const char* value)
    {
        setString(value, 0);
    }

    /**
     * @brief String assignment method.
     *
     * In contrast to the Variant::setString() method, this method tries not to decrease the allocated buffer.
     * @param value const char*, string to assign.
     * @param maxLength size_t, maximum length of the assigned string.
     */
    virtual void setString(const char* value, size_t maxLength);

    /**
     * @brief String assignment method.
     *
     * In contrast to the Variant::setString() method, this method tries not to decrease the allocated buffer.
     * @param value const string&, string to assign.
     */
    void setString(const String& value) override
    {
        setString(value.c_str(), static_cast<uint32_t>(value.length()));
    }

    /**
     * @brief Returns true if parameter is output parameter.
     */
    [[nodiscard]] bool isOutput() const
    {
        return m_binding.m_output;
    }

    /**
     * @brief The last successfull binding information.
     * @return binding information.
     */
    [[nodiscard]] QueryParameterBinding& binding()
    {
        return m_binding;
    }

private:
    QueryParameterBinding   m_binding;            ///< The last successfull binding information.
    std::string             m_name;               ///< Parameter name.
    std::vector<uint32_t>   m_bindParamIndexes;   ///< The list of SQL query parameter numbers with this name.
    std::array<uint8_t, 80> m_timeData {};        ///< Special memory allocated for time structures.
    int64_t                 m_callbackLength {0}; ///< An integer reserved to callback parameter data length.
};

using SQueryParameter = std::shared_ptr<QueryParameter>;

/**
 * @}
 */
} // namespace sptk
