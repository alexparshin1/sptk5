#include "CLoginResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CLoginResponse::m_fieldNames { "jwt" };
const Strings CLoginResponse::m_elementNames { "jwt" };
const Strings CLoginResponse::m_attributeNames { "" };

CLoginResponse::CLoginResponse(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(m_elementNames, {&m_jwt});
}

CLoginResponse::CLoginResponse(const CLoginResponse& other)
: WSComplexType(other),
  m_jwt(other.m_jwt)
{
    WSComplexType::setElements(m_elementNames, {&m_jwt});
}

CLoginResponse::CLoginResponse(CLoginResponse&& other) noexcept
: WSComplexType(std::move(other)),
  m_jwt(std::move(other.m_jwt))
{
    WSComplexType::setElements(m_elementNames, {&m_jwt});
}

void CLoginResponse::checkRestrictions() const
{
    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse.jwt");
}

