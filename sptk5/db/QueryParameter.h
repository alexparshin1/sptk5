/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        QueryParameter.h - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __SPTK_QUERYPARAMETER_H__
#define __SPTK_QUERYPARAMETER_H__

#include <sptk5/db/QueryParameterBinding.h>
#include <sptk5/IntList.h>

namespace sptk
{

class QueryParameterList;

/**
 * @brief SQL query parameter.
 *
 * Simplifies the ODBC parameter binding.
 * Automatically handles most of the data conversions on assignments.
 */
class SP_EXPORT QueryParameter : public Variant
{
    friend class Query;
    friend class QueryParameterList;

    /**
     * Parameter name
     */
    String              m_name;

    /**
     * The list of SQL query parameter numbers with this name
     */
    IntList             m_bindParamIndexes;

    /**
     * Special memory allocated for time structures
     */
    char*               m_timeData;

    /**
     * An integer reserved to callback parameter data length
     */
    int32_t             m_callbackLength;

    /**
     * Parent param list used for notifications
     */
    QueryParameterList*         m_paramList;

protected:

public:
    /**
     * The last successfull binding information
     */
    QueryParameterBinding       m_binding;


    /**
     * @brief Clears internal parameter binding index
     */
    void bindClear();

    /**
     * @brief Adds internal parameter binding index
     */
    void bindAdd(uint32_t bindIndex);

    /**
     * @brief Returns internal parameter binding count
     */
    uint32_t bindCount();

    /**
     * @brief Returns the parameter bing position by index in the binding list
     */
    uint32_t bindIndex(uint32_t ind);

    /**
     * @brief Returns the internal small conversion buffer used to convert the date structure to SPTK.
     *
     * Please, don't use it directly.
     */
    char* conversionBuffer() {
        return m_timeData;
    }

    /**
     * @brief An integer reserved to callback parameter data length
     */
    int32_t& callbackLength() {
        return m_callbackLength;
    }

    /**
     * @brief Constructor
     * @param name char *, parameter name
     * @param isOutput bool, parameter binding type: input or output
     */
    QueryParameter(const char* name, bool isOutput=false);

    /**
     * @brief Destructor
     */
    ~QueryParameter();

    /**
     * @brief Returns parameter name
     */
    String name() const;

    /**
     * @brief Set parameter type to output
     */
    void setOutput();

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (const Variant& param);

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (int16_t v) {
        setInteger(v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (uint16_t v) {
        setInteger(v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (int32_t v) {
        setInteger(v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (uint32_t v) {
        setInteger((int32_t)v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (int64_t v) {
        setInt64(v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (uint64_t v) {
        setInt64((int64_t)v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (float v) {
        setFloat(v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (double v) {
        setFloat(v);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (const char* s) {
        setString(s, 0);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (const std::string& s) {
        setString(s.c_str(), s.length());
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (DateTime dt) {
        setDateTime(dt);
        return *this;
    }

    /**
     * @brief Assignment operator
     */
    virtual QueryParameter& operator = (const void* value) {
        setImagePtr(value);
        return *this;
    }

    /**
     * @brief Assign operator
     */
    QueryParameter& operator = (const Buffer& buffer) {
        setBuffer(buffer);
        return *this;
    }

    /**
     * @brief String assignment method
     *
     * In contrast to CVariant::setString() method, this method
     * tries not to decrease the allocated buffer.
     * @param value const char*, string to assign
     */
    virtual void setString(const char* value) { setString(value, 0); }

    /**
     * @brief String assignment method
     *
     * In contrast to CVariant::setString() method, this method
     * tries not to decrease the allocated buffer.
     * @param value const char*, string to assign
     * @param maxlen size_t, maximum length of the assigned string
     */
    virtual void setString(const char* value, size_t maxlen);

    /**
     * @brief String assignment method
     *
     * In contrast to CVariant::setString() method, this method
     * tries not to decrease the allocated buffer.
     * @param value const string&, string to assign
     */
    virtual void setString(const std::string& value) {
        setString(value.c_str(), (uint32_t)value.length());
    }

    /**
     * @brief Returns true if parameter is output parameter
     */
    bool isOutput() const {
        return m_binding.m_output;
    }
};

/**
 * @}
 */
}

#endif
