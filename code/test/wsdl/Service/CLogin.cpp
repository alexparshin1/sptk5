#include "CLogin.h"
using namespace std;
using namespace sptk;
using namespace test_service;

const sptk::Strings& CLogin::fieldNames(WSFieldIndex::Group group)
{
    static const Strings _fieldNames { "username", "password", "servers", "project", "server_count", "type" };
    static const Strings _elementNames { "username", "password", "servers", "project" };
    static const Strings _attributeNames { "server_count", "type" };

    switch (group) {
        case WSFieldIndex::Group::ELEMENTS: return _elementNames;
        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;
        default: break;
    }

    return _fieldNames;
}

CLogin::CLogin(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_username, &m_password, &m_servers, &m_project});
    WSComplexType::setAttributes(fieldNames(WSFieldIndex::Group::ATTRIBUTES), {&m_server_count, &m_type});
}

CLogin::CLogin(const CLogin& other)
: WSComplexType(other),
  m_username(other.m_username),
  m_password(other.m_password),
  m_servers(other.m_servers),
  m_project(other.m_project)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_username, &m_password, &m_servers, &m_project});
    WSComplexType::setAttributes(fieldNames(WSFieldIndex::Group::ATTRIBUTES), {&m_server_count, &m_type});
}

CLogin::CLogin(CLogin&& other) noexcept
: WSComplexType(std::move(other)),
  m_username(std::move(other.m_username)),
  m_password(std::move(other.m_password)),
  m_servers(std::move(other.m_servers)),
  m_project(std::move(other.m_project))
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_username, &m_password, &m_servers, &m_project});
    WSComplexType::setAttributes(fieldNames(WSFieldIndex::Group::ATTRIBUTES), {&m_server_count, &m_type});
}

CLogin& CLogin::operator = (const CLogin& other)
{
    m_username = other.m_username;
    m_password = other.m_password;
    m_servers = other.m_servers;
    m_project = other.m_project;
    return *this;
}

CLogin& CLogin::operator = (CLogin&& other) noexcept
{
    m_username = std::move(other.m_username);
    m_password = std::move(other.m_password);
    m_servers = std::move(other.m_servers);
    m_project = std::move(other.m_project);
    return *this;
}

void CLogin::checkRestrictions() const
{
    // Check 'required' restrictions
    m_username.throwIfNull("Login.username");
    m_password.throwIfNull("Login.password");
}

