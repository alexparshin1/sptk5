#include "CAccountBalanceResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CAccountBalanceResponse::m_fieldNames { "account_balance" };
const Strings CAccountBalanceResponse::m_elementNames { "account_balance" };
const Strings CAccountBalanceResponse::m_attributeNames { "" };

CAccountBalanceResponse::CAccountBalanceResponse(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(m_elementNames, {&m_account_balance});
}

CAccountBalanceResponse::CAccountBalanceResponse(const CAccountBalanceResponse& other)
: WSComplexType(other),
  m_account_balance(other.m_account_balance)
{
    WSComplexType::setElements(m_elementNames, {&m_account_balance});
}

CAccountBalanceResponse::CAccountBalanceResponse(CAccountBalanceResponse&& other) noexcept
: WSComplexType(std::move(other)),
  m_account_balance(std::move(other.m_account_balance))
{
    WSComplexType::setElements(m_elementNames, {&m_account_balance});
}

void CAccountBalanceResponse::checkRestrictions() const
{
    // Check 'required' restrictions
    m_account_balance.throwIfNull("AccountBalanceResponse.account_balance");
}

