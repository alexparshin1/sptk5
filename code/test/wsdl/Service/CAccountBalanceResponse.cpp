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

