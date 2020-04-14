#include "CHello.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CHello::m_fieldNames { "action", "first_name", "last_name"};

CHello::~CHello()
{
    WSComplexType::clear();
}

void CHello::_clear()
{
    // Clear elements
    m_action.clear();
    m_first_name.clear();
    m_last_name.clear();
}

void CHello::load(const xml::Element* input)
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

        if (element->name() == "action") {
            m_action.load(element);
            continue;
        }

        if (element->name() == "first_name") {
            m_first_name.load(element);
            continue;
        }

        if (element->name() == "last_name") {
            m_last_name.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_action.throwIfNull("Hello");
    m_first_name.throwIfNull("Hello");
    m_last_name.throwIfNull("Hello");
}

void CHello::load(const json::Element* input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);

    // Load elements
    for (auto& itor: input->getObject()) {
        auto& elementName = *itor.first;
        auto* element = itor.second;

        if (elementName == "action") {
            m_action.load(element);
            continue;
        }

        if (elementName == "first_name") {
            m_first_name.load(element);
            continue;
        }

        if (elementName == "last_name") {
            m_last_name.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_action.throwIfNull("Hello");
    m_first_name.throwIfNull("Hello");
    m_last_name.throwIfNull("Hello");
}

void CHello::load(const FieldList& input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);
    Field* field;

    // Load elements
    if ((field = input.findField("action")) != nullptr) {
        m_action.load(*field);
    }

    if ((field = input.findField("first_name")) != nullptr) {
        m_first_name.load(*field);
    }

    if ((field = input.findField("last_name")) != nullptr) {
        m_last_name.load(*field);
    }


    // Check 'required' restrictions
    m_action.throwIfNull("Hello");
    m_first_name.throwIfNull("Hello");
    m_last_name.throwIfNull("Hello");
}

void CHello::unload(xml::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_action.addElement(output);
    m_first_name.addElement(output);
    m_last_name.addElement(output);
}

void CHello::unload(json::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_action.addElement(output);
    m_first_name.addElement(output);
    m_last_name.addElement(output);
}

void CHello::unload(QueryParameterList& output) const
{
    SharedLock(m_mutex);

    // Unload attributes
    WSComplexType::unload(output, "action", dynamic_cast<const WSBasicType*>(&m_action));
    WSComplexType::unload(output, "first_name", dynamic_cast<const WSBasicType*>(&m_first_name));
    WSComplexType::unload(output, "last_name", dynamic_cast<const WSBasicType*>(&m_last_name));
}
