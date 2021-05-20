#include "CHello.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CHello::m_fieldNames { "action", "first_name", "last_name" };
const Strings CHello::m_elementNames { "action", "first_name", "last_name" };
const Strings CHello::m_attributeNames { "" };

void CHello::checkRestrictions() const
{
    // Check 'required' restrictions
    m_action.throwIfNull("Hello.action");
    m_first_name.throwIfNull("Hello.first_name");
    m_last_name.throwIfNull("Hello.last_name");
}

void CHello::unload(xml::Node* output) const
{

    // Unload elements
    m_action.addElement(output);
    m_first_name.addElement(output);
    m_last_name.addElement(output);
}

void CHello::unload(json::Element* output) const
{

    // Unload elements
    m_action.addElement(output);
    m_first_name.addElement(output);
    m_last_name.addElement(output);
}

void CHello::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "action", dynamic_cast<const WSBasicType*>(&m_action));
    WSComplexType::unload(output, "first_name", dynamic_cast<const WSBasicType*>(&m_first_name));
    WSComplexType::unload(output, "last_name", dynamic_cast<const WSBasicType*>(&m_last_name));
}
