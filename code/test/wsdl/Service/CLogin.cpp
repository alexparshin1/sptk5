#include "CLogin.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CLogin::m_fieldNames { "username", "password", "servers", "project", "server_count", "type" };
const Strings CLogin::m_elementNames { "username", "password", "servers", "project" };
const Strings CLogin::m_attributeNames { "server_count", "type" };

CLogin::CLogin(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(m_elementNames, {&m_username, &m_password, &m_servers, &m_project});
    WSComplexType::setAttributes(m_attributeNames, {&m_server_count, &m_type});
}

CLogin::CLogin(const CLogin& other)
: WSComplexType(other),
  m_username(other.m_username),
  m_password(other.m_password),
  m_servers(other.m_servers),
  m_project(other.m_project)
{
    WSComplexType::setElements(m_elementNames, {&m_username, &m_password, &m_servers, &m_project});
    WSComplexType::setAttributes(m_attributeNames, {&m_server_count, &m_type});
}

CLogin::CLogin(CLogin&& other) noexcept
: WSComplexType(std::move(other)),
  m_username(std::move(other.m_username)),
  m_password(std::move(other.m_password)),
  m_servers(std::move(other.m_servers)),
  m_project(std::move(other.m_project))
{
    WSComplexType::setElements(m_elementNames, {&m_username, &m_password, &m_servers, &m_project});
    WSComplexType::setAttributes(m_attributeNames, {&m_server_count, &m_type});
}

void CLogin::checkRestrictions() const
{
    // Check 'required' restrictions
    m_username.throwIfNull("Login.username");
    m_password.throwIfNull("Login.password");
}

