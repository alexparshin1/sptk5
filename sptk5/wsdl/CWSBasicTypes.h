/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSBasicTypes.h  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#ifndef __CWSBASICTYPES_H__
#define __CWSBASICTYPES_H__

#include <sptk5/cxml>
#include <sptk5/CVariant.h>
#include <sptk5/xml/CXmlElement.h>

namespace sptk {

class WSType
{
protected:
    CVariant    m_data;
public:
    WSType() {}
    
    virtual ~WSType() {}
    
    virtual void load(const CXmlNode* attr);
    
    virtual void load(std::string attr);
    
    virtual std::string asString() const;

    CXmlElement* addElement(CXmlElement* parent, std::string name);
};

class WSBool : public WSType
{
public:
    
    virtual ~WSBool() {}
    
    virtual void load(const CXmlNode* attr);
    virtual void load(std::string attr);
    virtual std::string asString() const;

    WSType& operator = (bool value)
    {
        m_data = value;
        return *this;
    }
    
    operator bool () const
    {
        return m_data.asBool();
    }
};

class WSDate : public WSType
{
public:
    
    virtual ~WSDate() {}
    
    virtual void load(const CXmlNode* attr);
    virtual void load(std::string attr);
    virtual std::string asString() const;

    WSType& operator = (CDateTime value)
    {
        m_data = value;
        return *this;
    }
    
    operator CDateTime () const
    {
        return m_data.asDate();
    }
};

class WSDateTime : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
    virtual void load(std::string attr);
    virtual std::string asString() const;

    WSType& operator = (CDateTime value)
    {
        m_data = value;
        return *this;
    }
    
    operator CDateTime () const
    {
        return m_data.asDateTime();
    }
};

class WSDouble : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
    virtual void load(std::string attr);

    WSType& operator = (double value)
    {
        m_data = value;
        return *this;
    }
    
    operator double () const
    {
        return m_data.asFloat();
    }
};

class WSInteger : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
    virtual void load(std::string attr);

    WSType& operator = (int value)
    {
        m_data = value;
        return *this;
    }
    
    operator int () const
    {
        return m_data.asInteger();
    }
};

class WSString : public WSType
{
public:
    WSType& operator = (std::string value)
    {
        m_data = value;
        return *this;
    }
    
    operator std::string () const
    {
        return m_data.asString();
    }
};

class WSTime : public WSType
{
public:
    WSType& operator = (std::string value)
    {
        m_data = value;
        return *this;
    }
    
    operator std::string () const
    {
        return m_data.asString();
    }
};

}
#endif
