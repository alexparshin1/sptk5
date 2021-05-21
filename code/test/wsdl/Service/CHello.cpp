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

