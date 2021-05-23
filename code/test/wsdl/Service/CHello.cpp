#include "CHello.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CHello::m_fieldNames { "action", "first_name", "last_name" };
const Strings CHello::m_elementNames { "action", "first_name", "last_name" };
const Strings CHello::m_attributeNames { "" };

CHello::CHello(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(m_elementNames, {&m_action, &m_first_name, &m_last_name});
}

CHello::CHello(const CHello& other)
: WSComplexType(other),
  m_action(other.m_action),
  m_first_name(other.m_first_name),
  m_last_name(other.m_last_name)
{
    WSComplexType::setElements(m_elementNames, {&m_action, &m_first_name, &m_last_name});
}

CHello::CHello(CHello&& other) noexcept
: WSComplexType(std::move(other)),
  m_action(std::move(other.m_action)),
  m_first_name(std::move(other.m_first_name)),
  m_last_name(std::move(other.m_last_name))
{
    WSComplexType::setElements(m_elementNames, {&m_action, &m_first_name, &m_last_name});
}

void CHello::checkRestrictions() const
{
    // Check 'required' restrictions
    m_action.throwIfNull("Hello.action");
    m_first_name.throwIfNull("Hello.first_name");
    m_last_name.throwIfNull("Hello.last_name");
}

