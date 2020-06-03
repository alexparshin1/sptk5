#include "CAccountBalanceResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CAccountBalanceResponse::m_fieldNames { "account_balance"};

CAccountBalanceResponse::~CAccountBalanceResponse()
{
    WSComplexType::clear();
}

void CAccountBalanceResponse::_clear()
{
    // Clear elements
    m_account_balance.clear();
}

void CAccountBalanceResponse::load(const xml::Element* input)
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
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);

    // Load elements
    for (auto& itor: input->getObject()) {
        auto  elementName = itor.name();
        auto* element = itor.element();

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
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);
    Field* field;

    // Load elements
    if ((field = input.findField("account_balance")) != nullptr) {
        m_account_balance.load(*field);
    }


    // Check 'required' restrictions
    m_account_balance.throwIfNull("AccountBalanceResponse");
}

void CAccountBalanceResponse::unload(xml::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_account_balance.addElement(output);
}

void CAccountBalanceResponse::unload(json::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_account_balance.addElement(output);
}

void CAccountBalanceResponse::unload(QueryParameterList& output) const
{
    SharedLock(m_mutex);

    // Unload attributes
    WSComplexType::unload(output, "account_balance", dynamic_cast<const WSBasicType*>(&m_account_balance));
}
