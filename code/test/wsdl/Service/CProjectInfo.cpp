#include "CProjectInfo.h"
using namespace std;
using namespace sptk;
using namespace test_service;

const sptk::Strings& CProjectInfo::fieldNames(WSFieldIndex::Group group)
{
    static const Strings _fieldNames { "id", "expiration" };
    static const Strings _elementNames { "id", "expiration" };
    static const Strings _attributeNames { "" };

    switch (group) {
        case WSFieldIndex::Group::ELEMENTS: return _elementNames;
        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;
        default: break;
    }

    return _fieldNames;
}

CProjectInfo::CProjectInfo(const char* elementName, bool optional)
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_id, &m_expiration});
}

CProjectInfo::CProjectInfo(const CProjectInfo& other)
: WSComplexType(other),
  m_id(other.m_id),
  m_expiration(other.m_expiration)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_id, &m_expiration});
}

CProjectInfo::CProjectInfo(CProjectInfo&& other) noexcept
: WSComplexType(std::move(other)),
  m_id(std::move(other.m_id)),
  m_expiration(std::move(other.m_expiration))
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_id, &m_expiration});
}

void CProjectInfo::checkRestrictions() const
{
    // Check 'required' restrictions
    m_id.throwIfNull("ProjectInfo.id");
    m_expiration.throwIfNull("ProjectInfo.expiration");
}

