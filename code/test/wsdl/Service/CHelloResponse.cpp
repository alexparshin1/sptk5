#include "CHelloResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CHelloResponse::m_fieldNames { "date_of_birth", "verified", "retired", "hour_rate", "vacation_days", "height"};

CHelloResponse::~CHelloResponse()
{
    WSComplexType::clear();
}

void CHelloResponse::_clear()
{
    // Clear elements
    m_date_of_birth.clear();
    m_verified.clear();
    m_retired.clear();
    m_hour_rate.clear();
    m_vacation_days.clear();
    m_height.clear();
}

void CHelloResponse::load(const xml::Element* input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);

    // Load elements
    for (auto* node: *input) {
        auto* element = dynamic_cast<xml::Element*>(node);
        if (element == nullptr) {
            continue;
        }

        if (element->name() == "date_of_birth") {
            m_date_of_birth.load(element);
            continue;
        }

        if (element->name() == "verified") {
            m_verified.load(element);
            continue;
        }

        if (element->name() == "retired") {
            m_retired.load(element);
            continue;
        }

        if (element->name() == "hour_rate") {
            m_hour_rate.load(element);
            continue;
        }

        if (element->name() == "vacation_days") {
            m_vacation_days.load(element);
            continue;
        }

        if (element->name() == "height") {
            m_height.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_date_of_birth.throwIfNull("HelloResponse");
    m_verified.throwIfNull("HelloResponse");
    m_retired.throwIfNull("HelloResponse");
    m_hour_rate.throwIfNull("HelloResponse");
    m_vacation_days.throwIfNull("HelloResponse");
    m_height.throwIfNull("HelloResponse");
}

void CHelloResponse::load(const json::Element* input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);

    // Load elements
    for (auto& itor: input->getObject()) {
        auto& elementName = *itor.first;
        auto* element = itor.second;

        if (elementName == "date_of_birth") {
            m_date_of_birth.load(element);
            continue;
        }

        if (elementName == "verified") {
            m_verified.load(element);
            continue;
        }

        if (elementName == "retired") {
            m_retired.load(element);
            continue;
        }

        if (elementName == "hour_rate") {
            m_hour_rate.load(element);
            continue;
        }

        if (elementName == "vacation_days") {
            m_vacation_days.load(element);
            continue;
        }

        if (elementName == "height") {
            m_height.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_date_of_birth.throwIfNull("HelloResponse");
    m_verified.throwIfNull("HelloResponse");
    m_retired.throwIfNull("HelloResponse");
    m_hour_rate.throwIfNull("HelloResponse");
    m_vacation_days.throwIfNull("HelloResponse");
    m_height.throwIfNull("HelloResponse");
}

void CHelloResponse::load(const FieldList& input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);
    Field* field;

    // Load elements
    if ((field = input.findField("date_of_birth")) != nullptr) {
        m_date_of_birth.load(*field);
    }

    if ((field = input.findField("verified")) != nullptr) {
        m_verified.load(*field);
    }

    if ((field = input.findField("retired")) != nullptr) {
        m_retired.load(*field);
    }

    if ((field = input.findField("hour_rate")) != nullptr) {
        m_hour_rate.load(*field);
    }

    if ((field = input.findField("vacation_days")) != nullptr) {
        m_vacation_days.load(*field);
    }

    if ((field = input.findField("height")) != nullptr) {
        m_height.load(*field);
    }


    // Check 'required' restrictions
    m_date_of_birth.throwIfNull("HelloResponse");
    m_verified.throwIfNull("HelloResponse");
    m_retired.throwIfNull("HelloResponse");
    m_hour_rate.throwIfNull("HelloResponse");
    m_vacation_days.throwIfNull("HelloResponse");
    m_height.throwIfNull("HelloResponse");
}

void CHelloResponse::unload(xml::Element* output) const
{
    SharedLock(m_mutex);

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
    SharedLock(m_mutex);

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
    SharedLock(m_mutex);

    // Unload attributes
    WSComplexType::unload(output, "date_of_birth", dynamic_cast<const WSBasicType*>(&m_date_of_birth));
    WSComplexType::unload(output, "verified", dynamic_cast<const WSBasicType*>(&m_verified));
    WSComplexType::unload(output, "retired", dynamic_cast<const WSBasicType*>(&m_retired));
    WSComplexType::unload(output, "hour_rate", dynamic_cast<const WSBasicType*>(&m_hour_rate));
    WSComplexType::unload(output, "vacation_days", dynamic_cast<const WSBasicType*>(&m_vacation_days));
    WSComplexType::unload(output, "height", dynamic_cast<const WSBasicType*>(&m_height));
}
