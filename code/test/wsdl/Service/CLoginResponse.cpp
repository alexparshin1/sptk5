#include "CLoginResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CLoginResponse::m_fieldNames { "jwt" };
const Strings CLoginResponse::m_elementNames { "jwt" };
const Strings CLoginResponse::m_attributeNames { "" };

void CLoginResponse::_clear()
{
    // Clear elements
    m_jwt.clear();
}

void CLoginResponse::load(const xml::Element* input)
{
    _clear();
    setLoaded(true);

    // Load elements
    for (const auto* node: *input) {
        const auto* element = dynamic_cast<const xml::Element*>(node);
        if (element == nullptr) continue;

        if (element->name() == "jwt") {
            m_jwt.load(element);
            continue;
        }
    }


    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse");
}

void CLoginResponse::load(const json::Element* input)
{
    _clear();
    setLoaded(true);

    // Load elements
    for (auto& itor: input->getObject()) {
        const auto& elementName = itor.name();
        const auto* element = itor.element();

        if (elementName == "jwt") {
            m_jwt.load(element);
            continue;
        }
    }


    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse");
}

void CLoginResponse::load(const FieldList& input)
{
    _clear();
    setLoaded(true);
    const Field* field;

    // Load elements
    if ((field = input.findField("jwt")) != nullptr) {
        m_jwt.load(*field);
    }


    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse");
}

void CLoginResponse::unload(xml::Element* output) const
{

    // Unload elements
    m_jwt.addElement(output);
}

void CLoginResponse::unload(json::Element* output) const
{

    // Unload elements
    m_jwt.addElement(output);
}

void CLoginResponse::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "jwt", dynamic_cast<const WSBasicType*>(&m_jwt));
}
