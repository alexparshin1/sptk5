/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
 * SQL query parameter.
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
     * Adds internal parameter binding index
     */
    void bindAdd(uint32_t bindIndex);

    /**
     * Returns internal parameter binding count
     */
    uint32_t bindCount() const;

    /**
     * Returns the parameter bing position by index in the binding list
     */
    uint32_t bindIndex(uint32_t ind);

    /**
     * Returns the internal small conversion buffer used to convert the date structure to SPTK.
     *
     * Please, don't use it directly.
     */
    uint8_t* conversionBuffer()
    {
        return m_timeData.data();
    }

    /**
     * An integer reserved to callback parameter data length
     */
    long& callbackLength()
    {
        return m_callbackLength;
    }

    /**
     * Constructor
     * @param name char *, parameter name
     * @param isOutput bool, parameter binding type: input or output
     */
    explicit QueryParameter(const char* name, bool isOutput = false);

    /**
     * Destructor
     */
    ~QueryParameter() override = default;

    /**
     * Returns parameter name
     */
    String name() const;

    /**
     * Set parameter type to output
     */
    void setOutput();

    /**
     * Assign operator
     */
    QueryParameter& operator=(const Variant& param);

    /**
     * Assign operator
     */
    QueryParameter& operator=(int16_t v)
    {
        setInteger(v);
        return *this;
    }

    /**
     * Assign operator
     */
    QueryParameter& operator=(uint16_t v)
    {
        setInteger(v);
        return *this;
    }

    /**
     * Assign operator
     */
    QueryParameter& operator=(int32_t v) override
    {
        setInteger(v);
        return *this;
    }

    /**
     * Assign operator
     */
    QueryParameter& operator=(int64_t v) override
    {
        setInt64(v);
        return *this;
    }

    /**
     * Assign operator
     */
    QueryParameter& operator=(double v) override
    {
        setFloat(v);
        return *this;
    }

    /**
     * Assign operator
     */
    QueryParameter& operator=(const char* s) override
    {
        setString(s, 0);
        return *this;
    }

    /**
     * Assign operator
     */
    QueryParameter& operator=(const std::string& s)
    {
        setString(s.c_str(), s.length());
        return *this;
    }

    /**
     * Assign operator
     */
    QueryParameter& operator=(DateTime dt) override
    {
        setDateTime(dt);
        return *this;
    }

    /**
     * Assignment operator
     */
    QueryParameter& operator=(const uint8_t* value) override
    {
        setImagePtr(value);
        return *this;
    }

    /**
     * Assign operator
     * @param buffer             Data buffer
     */
    QueryParameter& operator=(const Buffer& buffer) override
    {
        setBuffer(buffer.data(), buffer.bytes());
        return *this;
    }

    /**
     * String assignment method
     *
     * In contrast to CVariant::setString() method, this method
     * tries not to decrease the allocated buffer.
     * @param value const char*, string to assign
     */
    virtual void setString(const char* value)
    {
        setString(value, 0);
    }

    /**
     * String assignment method
     *
     * In contrast to CVariant::setString() method, this method
     * tries not to decrease the allocated buffer.
     * @param value const char*, string to assign
     * @param maxlen size_t, maximum length of the assigned string
     */
    virtual void setString(const char* value, size_t maxlen);

    /**
     * String assignment method
     *
     * In contrast to CVariant::setString() method, this method
     * tries not to decrease the allocated buffer.
     * @param value const string&, string to assign
     */
    void setString(const String& value) override
    {
        setString(value.c_str(), (uint32_t) value.length());
    }

    /**
     * Returns true if parameter is output parameter
     */
    bool isOutput() const
    {
        return m_binding.m_output;
    }

    /**
     * The last successfull binding information
     * @return binding information
     */
    QueryParameterBinding& binding()
    {
        return m_binding;
    }

    void reallocateBuffer(const char* value, size_t maxlen, size_t valueLength);

private:
    QueryParameterBinding m_binding;           ///< The last successfull binding information
    String m_name;                             ///< Parameter name
    std::vector<uint32_t> m_bindParamIndexes;  ///< The list of SQL query parameter numbers with this name
    std::array<uint8_t, 80> m_timeData {};     ///< Special memory allocated for time structures
    long m_callbackLength {0};                 ///< An integer reserved to callback parameter data length
    QueryParameterList* m_paramList {nullptr}; ///< Parent param list used for notifications
};

using SQueryParameter = std::shared_ptr<QueryParameter>;

/**
 * @}
 */
} // namespace sptk
