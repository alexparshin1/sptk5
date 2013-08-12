/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSBasicTypes.h  -  description
                             -------------------
    begin                : 03 Aug 2012
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
    
    virtual void load(const CXmlNode* attr)
    {
        m_data = attr->text();
    }
    
    virtual std::string asString() const
    {
        return m_data;
    }

    CVariant& value()
    {
        return m_data;
    }

    CXmlElement* addElement(CXmlElement* parent, std::string name)
    {
        CXmlElement* element = new CXmlElement(*parent, name);
        element->text(m_data);
        return element;
    }
};

class WSBool : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
    virtual std::string asString() const;
};

class WSDate : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
    virtual std::string asString() const;
};

class WSDateTime : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
    virtual std::string asString() const;
};

class WSDouble : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
};

class WSInteger : public WSType
{
public:
    virtual void load(const CXmlNode* attr);
};

class WSString : public WSType
{
};

class WSTime : public WSType
{
};

}
#endif
