#include "CLoginResponse.h"
using namespace std;
using namespace sptk;
using namespace test_service;

const sptk::Strings& CLoginResponse::fieldNames(WSFieldIndex::Group group)
{
    static const Strings _fieldNames { "jwt" };
    static const Strings _elementNames { "jwt" };
    static const Strings _attributeNames { "" };

    switch (group) {
        case WSFieldIndex::Group::ELEMENTS: return _elementNames;
        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;
        default: break;
    }

    return _fieldNames;
}

CLoginResponse::CLoginResponse(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_jwt});
}

CLoginResponse::CLoginResponse(const CLoginResponse& other)
: WSComplexType(other),
  m_jwt(other.m_jwt)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_jwt});
}

CLoginResponse::CLoginResponse(CLoginResponse&& other) noexcept
: WSComplexType(std::move(other)),
  m_jwt(std::move(other.m_jwt))
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_jwt});
}

CLoginResponse& CLoginResponse::operator = (const CLoginResponse& other)
{
    m_jwt = other.m_jwt;
    return *this;
}

CLoginResponse& CLoginResponse::operator = (CLoginResponse&& other) noexcept
{
    m_jwt = std::move(other.m_jwt);
    return *this;
}

void CLoginResponse::checkRestrictions() const
{
    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse.jwt");
}

