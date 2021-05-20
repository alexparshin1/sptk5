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
    m_username.addElement(output);
    m_password.addElement(output);
}

void CLogin::unload(json::Element* output) const
{

    // Unload elements
    m_username.addElement(output);
    m_password.addElement(output);
}

void CLogin::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "username", dynamic_cast<const WSBasicType*>(&m_username));
    WSComplexType::unload(output, "password", dynamic_cast<const WSBasicType*>(&m_password));
}
