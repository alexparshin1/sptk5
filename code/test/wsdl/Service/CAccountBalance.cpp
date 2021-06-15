#include "CAccountBalance.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const sptk::Strings& CAccountBalance::fieldNames(WSFieldIndex::Group group)
{
    static const Strings _fieldNames { "account_number" };
    static const Strings _elementNames { "account_number" };
    static const Strings _attributeNames { "" };

    switch (group) {
        case WSFieldIndex::Group::ELEMENTS: return _elementNames;
        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;
        default: break;
    }

    return _fieldNames;
}

CAccountBalance::CAccountBalance(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_account_number});
}

CAccountBalance::CAccountBalance(const CAccountBalance& other)
: WSComplexType(other),
  m_account_number(other.m_account_number)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_account_number});
}

CAccountBalance::CAccountBalance(CAccountBalance&& other) noexcept
: WSComplexType(std::move(other)),
  m_account_number(std::move(other.m_account_number))
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_account_number});
}

CAccountBalance& CAccountBalance::operator = (const CAccountBalance& other)
{
    m_account_number = other.m_account_number;
    return *this;
}

CAccountBalance& CAccountBalance::operator = (CAccountBalance&& other) noexcept
{
    m_account_number = std::move(other.m_account_number);
    return *this;
}

void CAccountBalance::checkRestrictions() const
{
    // Check 'required' restrictions
    m_account_number.throwIfNull("AccountBalance.account_number");
}

