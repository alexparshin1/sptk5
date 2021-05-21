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

