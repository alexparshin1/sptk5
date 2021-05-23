#include "CAccountBalance.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CAccountBalance::m_fieldNames { "account_number" };
const Strings CAccountBalance::m_elementNames { "account_number" };
const Strings CAccountBalance::m_attributeNames { "" };

CAccountBalance::CAccountBalance(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(m_elementNames, {&m_account_number});
}

CAccountBalance::CAccountBalance(const CAccountBalance& other)
: WSComplexType(other),
  m_account_number(other.m_account_number)
{
    WSComplexType::setElements(m_elementNames, {&m_account_number});
}

CAccountBalance::CAccountBalance(CAccountBalance&& other) noexcept
: WSComplexType(std::move(other)),
  m_account_number(std::move(other.m_account_number))
{
    WSComplexType::setElements(m_elementNames, {&m_account_number});
}

void CAccountBalance::checkRestrictions() const
{
    // Check 'required' restrictions
    m_account_number.throwIfNull("AccountBalance.account_number");
}

