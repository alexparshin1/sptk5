#include "CHelloResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CHelloResponse::m_fieldNames { "date_of_birth", "verified", "retired", "hour_rate", "vacation_days", "height" };
const Strings CHelloResponse::m_elementNames { "date_of_birth", "verified", "retired", "hour_rate", "vacation_days", "height" };
const Strings CHelloResponse::m_attributeNames { "" };

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

void CHelloResponse::unload(xml::Node* output) const
{

    // Unload elements
    m_date_of_birth.addElement(output);
    m_verified.addElement(output);
    m_retired.addElement(output);
    m_hour_rate.addElement(output);
    m_vacation_days.addElement(output);
    m_height.addElement(output);
}

void CHelloResponse::unload(json::Element* output) const
{

    // Unload elements
    m_date_of_birth.addElement(output);
    m_verified.addElement(output);
    m_retired.addElement(output);
    m_hour_rate.addElement(output);
    m_vacation_days.addElement(output);
    m_height.addElement(output);
}

void CHelloResponse::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "date_of_birth", dynamic_cast<const WSBasicType*>(&m_date_of_birth));
    WSComplexType::unload(output, "verified", dynamic_cast<const WSBasicType*>(&m_verified));
    WSComplexType::unload(output, "retired", dynamic_cast<const WSBasicType*>(&m_retired));
    WSComplexType::unload(output, "hour_rate", dynamic_cast<const WSBasicType*>(&m_hour_rate));
    WSComplexType::unload(output, "vacation_days", dynamic_cast<const WSBasicType*>(&m_vacation_days));
    WSComplexType::unload(output, "height", dynamic_cast<const WSBasicType*>(&m_height));
}
