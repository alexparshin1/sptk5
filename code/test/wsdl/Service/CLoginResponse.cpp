#include "CLoginResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CLoginResponse::m_fieldNames { "jwt" };
const Strings CLoginResponse::m_elementNames { "jwt" };
const Strings CLoginResponse::m_attributeNames { "" };

void CLoginResponse::checkRestrictions() const
{
    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse.jwt");
}

void CLoginResponse::unload(xml::Node* output) const
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
