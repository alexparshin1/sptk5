#include "CHelloResponse.h"
using namespace std;
using namespace sptk;
using namespace test_service;

const sptk::Strings& CHelloResponse::fieldNames(WSFieldIndex::Group group)
{
    static const Strings _fieldNames { "date_of_birth", "verified", "retired", "hour_rate", "vacation_days", "height" };
    static const Strings _elementNames { "date_of_birth", "verified", "retired", "hour_rate", "vacation_days", "height" };
    static const Strings _attributeNames { "" };

    switch (group) {
        case WSFieldIndex::Group::ELEMENTS: return _elementNames;
        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;
        default: break;
    }

    return _fieldNames;
}

CHelloResponse::CHelloResponse(const char* elementName, bool optional) noexcept
: WSComplexType(elementName, optional)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_date_of_birth, &m_verified, &m_retired, &m_hour_rate, &m_vacation_days, &m_height});
}

CHelloResponse::CHelloResponse(const CHelloResponse& other)
: WSComplexType(other),
  m_date_of_birth(other.m_date_of_birth),
  m_verified(other.m_verified),
  m_retired(other.m_retired),
  m_hour_rate(other.m_hour_rate),
  m_vacation_days(other.m_vacation_days),
  m_height(other.m_height)
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_date_of_birth, &m_verified, &m_retired, &m_hour_rate, &m_vacation_days, &m_height});
}

CHelloResponse::CHelloResponse(CHelloResponse&& other) noexcept
: WSComplexType(std::move(other)),
  m_date_of_birth(std::move(other.m_date_of_birth)),
  m_verified(std::move(other.m_verified)),
  m_retired(std::move(other.m_retired)),
  m_hour_rate(std::move(other.m_hour_rate)),
  m_vacation_days(std::move(other.m_vacation_days)),
  m_height(std::move(other.m_height))
{
    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_date_of_birth, &m_verified, &m_retired, &m_hour_rate, &m_vacation_days, &m_height});
}

void CHelloResponse::checkRestrictions() const
{
    // Check 'required' restrictions
    m_date_of_birth.throwIfNull("HelloResponse.date_of_birth");
    m_verified.throwIfNull("HelloResponse.verified");
    m_retired.throwIfNull("HelloResponse.retired");
    m_hour_rate.throwIfNull("HelloResponse.hour_rate");
    m_vacation_days.throwIfNull("HelloResponse.vacation_days");
    m_height.throwIfNull("HelloResponse.height");
}

