/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/wsdl/WSComplexType.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

void WSComplexType::copyFrom(const WSComplexType& other)
{
    xml::Document xml;
    auto* element = new xml::Element(xml, "temp");
    other.unload(element);
    load(element);
}

void WSComplexType::unload(QueryParameterList& output, const char* paramName, const WSBasicType* elementOrAttribute)
{
    if (elementOrAttribute == nullptr)
        return;

    sptk::QueryParameter* param = output.find(paramName);
    if (param != nullptr)
        *param = elementOrAttribute->field();
}

void WSComplexType::addElement(xml::Node* parent, const char* name) const
{
    if (m_exportable) {
        const char* elementName = name == nullptr ? m_name.c_str() : name;
        unload(new xml::Element(parent, elementName));
    }
}

void WSComplexType::addElement(json::Element* parent) const
{
    if (m_exportable) {
        if (isOptional() && isNull())
            return;
        json::Element* element = nullptr;
        if (parent->is(json::JDT_ARRAY))
            element = parent->push_back();
        else
            element = parent->set(m_name);
        unload(element);
    }
}

String WSComplexType::toString(bool asJSON, bool formatted) const
{
    Buffer          output;

    if (asJSON) {
        json::Document  outputJSON;
        unload(&outputJSON.root());
        outputJSON.exportTo(output, formatted);
    } else {
        xml::Document outputXML;
        auto* element = new xml::Element(outputXML, "type");
        unload(element);
        outputXML.save(output, formatted? 2 : 0);
    }

    return String(output.c_str(), output.bytes());
}

void WSComplexType::throwIfNull(const String& parentTypeName) const
{
    if (!m_loaded)
        throw SOAPException("Element '" + m_name + "' is required in '" + parentTypeName + "'.");
}

void WSComplexType::load(const xml::Node* input)
{
    _clear();
    setLoaded(true);

    // Load elements
    for (const auto* node: *input) {
        const auto* element = dynamic_cast<const xml::Element*>(node);
        if (element == nullptr) continue;

        auto* field = m_fields.find(element->name());
        if (field != nullptr) {
            field->load(node);
        }
    }

    // Load attributes
    for (const auto* attribute: input->attributes()) {
        auto* field = m_fields.find(attribute->name());
        if (field != nullptr) {
            auto* outputField = dynamic_cast<WSBasicType*>(field);
            if (outputField != nullptr) {
                outputField->load(attribute->value());
            }
        }
    }

    checkRestrictions();
}

void WSComplexType::load(const json::Element* input)
{
    _clear();
    setLoaded(true);
    if (!input->is(json::JDT_OBJECT))
        return;

    // Load elements
    for (const auto& itor: input->getObject()) {
        const auto* element = itor.element();

        auto* field = m_fields.find(itor.name());
        if (field != nullptr) {
            field->load(element);
        }
    }

    // Load attributes
    const auto* attributes = input->find("attributes");
    if (attributes != nullptr) {
        if (attributes->is(json::JDT_OBJECT)) {
            for (const auto& attribute: attributes->getObject()) {
                auto* field = m_fields.find(attribute.name());
                if (field != nullptr) {
                    auto* outputField = dynamic_cast<WSBasicType*>(field);
                    if (outputField != nullptr) {
                        outputField->load((String)*attribute.element());
                    }
                }
            }
        }
    }

    checkRestrictions();
}

void WSComplexType::load(const FieldList& input)
{
    _clear();
    setLoaded(true);

    m_fields.forEach([&input](const String& name, WSType* field)
    {
        auto* inputField = input.findField("username");
        auto* outputField = dynamic_cast<WSBasicType*>(field);
        if (inputField != nullptr && outputField != nullptr) {
            outputField->load(*inputField);
        }
        return true;
    });

    checkRestrictions();
}

bool WSComplexType::isNull() const
{
    bool hasValues = false;
    m_fields.forEach([&hasValues](const String&, WSType* field)
                     {
                         if (field->isNull())
                            return true;
                         hasValues = true;
                         return false;
                     });
    return !hasValues;
}
