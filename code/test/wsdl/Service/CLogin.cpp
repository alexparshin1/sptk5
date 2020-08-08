#include "CLogin.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CLogin::m_fieldNames { "username", "password" };
const Strings CLogin::m_elementNames { "username", "password" };
const Strings CLogin::m_attributeNames { "" };

void CLogin::_clear()
{
    // Clear elements
    m_username.clear();
    m_password.clear();
}

void CLogin::load(const xml::Element* input)
{
    _clear();
    setLoaded(true);

    // Load elements
    for (const auto* node: *input) {
        const auto* element = dynamic_cast<const xml::Element*>(node);
        if (element == nullptr) continue;

        if (element->name() == "username") {
            m_username.load(element);
            continue;
        }

        if (element->name() == "password") {
            m_password.load(element);
            continue;
        }
    }


    // Check 'required' restrictions
    m_username.throwIfNull("Login");
    m_password.throwIfNull("Login");
}

void CLogin::load(const json::Element* input)
{
    _clear();
    setLoaded(true);
    if (!input->is(json::JDT_OBJECT))
        return;

    // Load elements
    for (auto& itor: input->getObject()) {
        const auto& elementName = itor.name();
        const auto* element = itor.element();

        if (elementName == "username") {
            m_username.load(element);
            continue;
        }

        if (elementName == "password") {
            m_password.load(element);
            continue;
        }
    }


    // Check 'required' restrictions
    m_username.throwIfNull("Login");
    m_password.throwIfNull("Login");
}

void CLogin::load(const FieldList& input)
{
    _clear();
    setLoaded(true);
    const Field* field;

    // Load elements
    if ((field = input.findField("username")) != nullptr) {
        m_username.load(*field);
    }

    if ((field = input.findField("password")) != nullptr) {
        m_password.load(*field);
    }


    // Check 'required' restrictions
    m_username.throwIfNull("Login");
    m_password.throwIfNull("Login");
}

void CLogin::unload(xml::Element* output) const
{

    // Unload elements
    m_username.addElement(output);
    m_password.addElement(output);
}

void CLogin::unload(json::Element* output) const
{

    // Unload elements
    m_username.addElement(output);
    m_password.addElement(output);
}

bool CLogin::isNull() const
{
    // Check elements
    bool elementsAreNull = 
        m_username.isNull()
        && m_password.isNull();

    return elementsAreNull;
}

void CLogin::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "username", dynamic_cast<const WSBasicType*>(&m_username));
    WSComplexType::unload(output, "password", dynamic_cast<const WSBasicType*>(&m_password));
}
