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

