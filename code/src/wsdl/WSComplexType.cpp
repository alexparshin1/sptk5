/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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

#include <sptk5/Printer.h>
#include <sptk5/wsdl/WSComplexType.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

void WSComplexType::copyFrom(const WSComplexType& other)
{
    xdoc::Document xml;
    const auto& element = xml.root()->pushNode("temp");
    other.unload(element);
    load(element);
}

void WSComplexType::unload(QueryParameterList& output, const char* paramName, const WSBasicType* elementOrAttribute)
{
    if (elementOrAttribute == nullptr)
    {
        return;
    }

    auto param = output.find(paramName);
    if (param)
    {
        *param = elementOrAttribute->value();
    }
}

void WSComplexType::exportTo(const SNode& parent, const char* name) const
{
    if (m_exportable)
    {
        if (isOptional() && isNull())
        {
            return;
        }

        const String elementName = name == nullptr ? this->name().c_str() : name;
        xdoc::SNode element;
        if (parent->type() == Node::Type::Array)
        {
            element = parent->pushNode(elementName);
        }
        else
        {
            element = parent->pushNode(this->name());
        }
        unload(element);
    }
}

String WSComplexType::toString(bool asJSON, bool formatted) const
{
    Buffer output;

    if (asJSON)
    {
        xdoc::Document outputJSON;
        unload(outputJSON.root());
        outputJSON.exportTo(DataFormat::JSON, output, formatted);
    }
    else
    {
        xdoc::Document outputXML;
        const auto& element = outputXML.root()->pushNode("type");
        unload(element);
        outputXML.exportTo(DataFormat::XML, output, formatted);
    }

    return {output.c_str(), output.bytes()};
}

void WSComplexType::throwIfNull(const String& parentTypeName) const
{
    if (!m_loaded)
    {
        throw SOAPException("Element '" + name() + "' is required in '" + parentTypeName + "'.");
    }
}

void WSComplexType::load(const SNode& input, bool)
{
    _clear();
    setLoaded(true);
    if (input->type() != Node::Type::Object)
    {
        return;
    }

    // Load elements
    for (const auto& node: input->nodes())
    {
        if (auto* field = m_fields.find(node->name());
            field != nullptr)
        {
            field->load(node);
        }
    }

    // Load attributes
    for (const auto& [attr, value]: input->attributes())
    {
        if (auto* field = m_fields.find(attr);
            field != nullptr)
        {
            if (auto* outputField = dynamic_cast<WSBasicType*>(field);
                outputField != nullptr)
            {
                outputField->load(value);
            }
        }
    }

    checkRestrictions();
}

void WSComplexType::load(const FieldList& input, bool nullLargeData)
{
    _clear();
    setLoaded(true);

    m_fields.forEach([&input, nullLargeData](WSType* field) {
        return loadField(input, nullLargeData, field);
    });

    checkRestrictions();
}

bool WSComplexType::loadField(const FieldList& input, bool nullLargeData, WSType* field)
{
    const auto& inputField = input.findField(field->name());

    if (inputField == nullptr)
    {
        return true;
    }

    if (auto* outputField = dynamic_cast<WSBasicType*>(field);
        outputField != nullptr)
    {
        if (nullLargeData)
        {
            outputField->setNull();
        }
        else
        {
            outputField->load(*inputField);
        }
    }
    else
    {
        if (nullLargeData)
        {
            field->clear();
        }
        else
        {
            Document document;
            auto json = inputField->asString();
            document.load(json);
            field->load(document.root());
        }
    }

    return true;
}

bool WSComplexType::isNull() const
{
    bool hasValues = false;
    m_fields.forEach([&hasValues](const WSType* field) {
        if (field->isNull())
        {
            return true;
        }
        hasValues = true;
        return false;
    });
    return !hasValues;
}

void WSComplexType::unload(const SNode& output) const
{
    const auto& fields = getFields();

    // Unload attributes
    if (fields.hasAttributes())
    {
        // Unload attributes
        fields.forEach([output](const WSType* field) {
            if (!field->isNull())
            {
                output->attributes().set(field->name(), field->asString());
            }
            return true;
        },
                       WSFieldIndex::Group::ATTRIBUTES);
    }

    // Unload elements
    fields.forEach([&output](const WSType* field) {
        field->exportTo(output);
        return true;
    },
                   WSFieldIndex::Group::ELEMENTS);
}

void WSComplexType::unload(QueryParameterList& output) const
{
    const auto& fields = getFields();

    fields.forEach([
#ifdef _WIN32
                       this,
#endif
                       &output](const WSType* field) {
        if (const auto* inputField = dynamic_cast<const WSBasicType*>(field);
            inputField != nullptr)
        {
            WSComplexType::unload(output, inputField->name().c_str(), inputField);
        }
        else
        {
            auto param = output.find(field->name());
            if (param)
            {
                *param = field->asString();
            }
        }
        return true;
    },
                   WSFieldIndex::Group::ELEMENTS_AND_ATTRIBUTES);
}
