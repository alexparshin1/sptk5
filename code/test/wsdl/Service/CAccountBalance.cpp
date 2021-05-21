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

