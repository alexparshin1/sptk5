#include "CAccountBalanceResponse.h"
using namespace std;
using namespace sptk;
using namespace test_service;

const sptk::Strings& CAccountBalanceResponse::fieldNames(WSFieldIndex::Group group)
{
    static const Strings _fieldNames { "account_balance" };
    static const Strings _elementNames { "account_balance" };
    static const Strings _attributeNames { "" };

    switch (group) {
        case WSFieldIndex::Group::ELEMENTS: return _elementNames;
        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;
        default: break;
    }

    return _fieldNames;
}

CAccountBalanceResponse::CAccountBalanceResponse(const char* elementName, bool optional)
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_account_balance});
}

CAccountBalanceResponse::CAccountBalanceResponse(const CAccountBalanceResponse& other)
: WSComplexType(other),
  m_account_balance(other.m_account_balance)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_account_balance});
}

CAccountBalanceResponse::CAccountBalanceResponse(CAccountBalanceResponse&& other) noexcept
: WSComplexType(std::move(other)),
  m_account_balance(std::move(other.m_account_balance))
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_account_balance});
}

void CAccountBalanceResponse::checkRestrictions() const
{
    // Check 'required' restrictions
    m_account_balance.throwIfNull("AccountBalanceResponse.account_balance");
}

