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

