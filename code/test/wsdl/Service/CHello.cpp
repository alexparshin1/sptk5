#include "CHello.h"
using namespace std;
using namespace sptk;
using namespace test_service;

const sptk::Strings& CHello::fieldNames(WSFieldIndex::Group group)
{
    static const Strings _fieldNames { "action", "first_name", "last_name" };
    static const Strings _elementNames { "action", "first_name", "last_name" };
    static const Strings _attributeNames { "" };

    switch (group) {
        case WSFieldIndex::Group::ELEMENTS: return _elementNames;
        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;
        default: break;
    }

    return _fieldNames;
}

CHello::CHello(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_action, &m_first_name, &m_last_name});
}

CHello::CHello(const CHello& other)
: WSComplexType(other),
  m_action(other.m_action),
  m_first_name(other.m_first_name),
  m_last_name(other.m_last_name)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_action, &m_first_name, &m_last_name});
}

CHello::CHello(CHello&& other) noexcept
: WSComplexType(std::move(other)),
  m_action(std::move(other.m_action)),
  m_first_name(std::move(other.m_first_name)),
  m_last_name(std::move(other.m_last_name))
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_action, &m_first_name, &m_last_name});
}

void CHello::checkRestrictions() const
{
    // Check 'required' restrictions
    m_action.throwIfNull("Hello.action");
    m_first_name.throwIfNull("Hello.first_name");
    m_last_name.throwIfNull("Hello.last_name");
}

