#include "CAccountBalance.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CAccountBalance::m_fieldNames { "account_number" };
const Strings CAccountBalance::m_elementNames { "account_number" };
const Strings CAccountBalance::m_attributeNames { "" };

void CAccountBalance::checkRestrictions() const
{
    // Check 'required' restrictions
    m_account_number.throwIfNull("AccountBalance.account_number");
}

void CAccountBalance::_clear()
{
    // Clear elements
    m_account_number.clear();
}

void CAccountBalance::load(const xml::Element* input)
{
    _clear();
    setLoaded(true);

    // Load elements
    for (const auto* node: *input) {
        const auto* element = dynamic_cast<const xml::Element*>(node);
        if (element == nullptr) continue;

        if (element->name() == "account_number") {
            m_account_number.load(element);
            continue;
        }
    }
    checkRestrictions();
}

void CAccountBalance::load(const json::Element* input)
{
    _clear();
    setLoaded(true);
    if (!input->is(json::JDT_OBJECT))
        return;

    // Load elements
    for (auto& itor: input->getObject()) {
        const auto& elementName = itor.name();
        const auto* element = itor.element();

        if (elementName == "account_number") {
            m_account_number.load(element);
            continue;
        }
    }
    checkRestrictions();
}

void CAccountBalance::load(const FieldList& input)
{
    _clear();
    setLoaded(true);
    const Field* field;

    // Load elements
    if ((field = input.findField("account_number")) != nullptr) {
        m_account_number.load(*field);

    }

    checkRestrictions();
}

void CAccountBalance::unload(xml::Element* output) const
{

    // Unload elements
    m_account_number.addElement(output);
}

void CAccountBalance::unload(json::Element* output) const
{

    // Unload elements
    m_account_number.addElement(output);
}

bool CAccountBalance::isNull() const
{
    // Check elements
    bool elementsAreNull = 
        m_account_number.isNull();

    return elementsAreNull;
}

void CAccountBalance::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "account_number", dynamic_cast<const WSBasicType*>(&m_account_number));
}
