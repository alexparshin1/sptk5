#include "CAccountBalance.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CAccountBalance::m_fieldNames { "account_number" };
const Strings CAccountBalance::m_elementNames { "account_number" };
const Strings CAccountBalance::m_attributeNames { "" };

void CAccountBalance::checkRestrictions() const
{
    // Check 'required' restrictions
    m_account_number.throwIfNull("AccountBalance.account_number");
}

void CAccountBalance::unload(xml::Node* output) const
{

    // Unload elements
    m_account_number.addElement(output);
}

void CAccountBalance::unload(json::Element* output) const
{

    // Unload elements
    m_account_number.addElement(output);
}

void CAccountBalance::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "account_number", dynamic_cast<const WSBasicType*>(&m_account_number));
}
