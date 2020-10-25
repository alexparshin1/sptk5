#include "CAccountBalanceResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace test_service;

const Strings CAccountBalanceResponse::m_fieldNames { "account_balance" };
const Strings CAccountBalanceResponse::m_elementNames { "account_balance" };
const Strings CAccountBalanceResponse::m_attributeNames { "" };

void CAccountBalanceResponse::_clear()
{
    // Clear elements
    m_account_balance.clear();
}

void CAccountBalanceResponse::load(const xml::Element* input)
{
    _clear();
    setLoaded(true);

    // Load elements
    for (const auto* node: *input) {
        const auto* element = dynamic_cast<const xml::Element*>(node);
        if (element == nullptr) continue;

        if (element->name() == "account_balance") {
            m_account_balance.load(element);
            continue;
        }
    }


    // Check 'required' restrictions
    m_account_balance.throwIfNull("AccountBalanceResponse");
}

void CAccountBalanceResponse::load(const json::Element* input)
{
    _clear();
    setLoaded(true);
    if (!input->is(json::JDT_OBJECT))
        return;

    // Load elements
    for (auto& itor: input->getObject()) {
        const auto& elementName = itor.name();
        const auto* element = itor.element();

        if (elementName == "account_balance") {
            m_account_balance.load(element);
            continue;
        }
    }


    // Check 'required' restrictions
    m_account_balance.throwIfNull("AccountBalanceResponse");
}

void CAccountBalanceResponse::load(const FieldList& input)
{
    _clear();
    setLoaded(true);
    const Field* field;

    // Load elements
    if ((field = input.findField("account_balance")) != nullptr) {
        m_account_balance.load(*field);

    }


    // Check 'required' restrictions
    m_account_balance.throwIfNull("AccountBalanceResponse");
}

void CAccountBalanceResponse::unload(xml::Element* output) const
{

    // Unload elements
    m_account_balance.addElement(output);
}

void CAccountBalanceResponse::unload(json::Element* output) const
{

    // Unload elements
    m_account_balance.addElement(output);
}

bool CAccountBalanceResponse::isNull() const
{
    // Check elements
    bool elementsAreNull = 
        m_account_balance.isNull();

    return elementsAreNull;
}

void CAccountBalanceResponse::unload(QueryParameterList& output) const
{

    // Unload attributes
    WSComplexType::unload(output, "account_balance", dynamic_cast<const WSBasicType*>(&m_account_balance));
}
