#include "CAccountBalanceResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CAccountBalanceResponse::m_fieldNames { "account_balance" };
const Strings CAccountBalanceResponse::m_elementNames { "account_balance" };
const Strings CAccountBalanceResponse::m_attributeNames { "" };

void CAccountBalanceResponse::checkRestrictions() const
{
    // Check 'required' restrictions
    m_account_balance.throwIfNull("AccountBalanceResponse.account_balance");
}

void CAccountBalanceResponse::unload(xml::Node* output) const
{

    // Unload elements
    m_account_balance.addElement(output);
}

void CAccountBalanceResponse::unload(json::Element* output) const
{

    // Unload elements
    m_account_balance.addElement(output);
}

void CAccountBalanceResponse::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "account_balance", dynamic_cast<const WSBasicType*>(&m_account_balance));
}
