#include "CLogin.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CLogin::m_fieldNames { "username", "password" };
const Strings CLogin::m_elementNames { "username", "password" };
const Strings CLogin::m_attributeNames { "" };

void CLogin::checkRestrictions() const
{
    // Check 'required' restrictions
    m_username.throwIfNull("Login.username");
    m_password.throwIfNull("Login.password");
}

void CLogin::unload(xml::Node* output) const
{
    // Unload elements
    getFields().forEach([&output](const String&, const WSType* field)
    {
        field->addElement(output);
        return true;
    }, WSFieldIndex::ELEMENTS);

    // Unload attributes
    getFields().forEach([&output](const String& name, const WSType* field)
    {
        output->setAttribute(name, field->asString());
        return true;
    }, WSFieldIndex::ATTRIBUTES);
}

void CLogin::unload(json::Element* output) const
{
    const auto& fields = getFields();

    // Unload elements
    fields.forEach([&output](const String&, const WSType* field)
    {
        field->addElement(output);
        return true;
    }, WSFieldIndex::ELEMENTS);

    if (fields.hasAttributes()) {
        auto* attributes = output->add_object("attributes");
        // Unload attributes
        fields.forEach([&attributes](const String& name, const WSType* field) {
            attributes->set(name, field->asString());
            return true;
        }, WSFieldIndex::ATTRIBUTES);
    }
}

void CLogin::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "username", dynamic_cast<const WSBasicType*>(&m_username));
    WSComplexType::unload(output, "password", dynamic_cast<const WSBasicType*>(&m_password));
}
