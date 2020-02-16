#include "CAccountBalance.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CAccountBalance::m_fieldNames { "account_number"};

CAccountBalance::~CAccountBalance()
{
    WSComplexType::clear();
}

void CAccountBalance::_clear()
{
    // Clear elements
    m_account_number.clear();
}

void CAccountBalance::load(const xml::Element* input)
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

        if (element->name() == "account_number") {
            m_account_number.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_account_number.throwIfNull("AccountBalance");
}

void CAccountBalance::load(const json::Element* input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);

    // Load elements
    for (auto& itor: input->getObject()) {
        auto& elementName = *itor.first;
        auto* element = itor.second;

        if (elementName == "account_number") {
            m_account_number.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_account_number.throwIfNull("AccountBalance");
}

void CAccountBalance::load(const FieldList& input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);
    Field* field;

    // Load elements
    if ((field = input.fieldByName("account_number")) != nullptr) {
        m_account_number.load(*field);
    }


    // Check 'required' restrictions
    m_account_number.throwIfNull("AccountBalance");
}

void CAccountBalance::unload(xml::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_account_number.addElement(output);
}

void CAccountBalance::unload(json::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_account_number.addElement(output);
}

void CAccountBalance::unload(QueryParameterList& output) const
{
    SharedLock(m_mutex);

    // Unload attributes
    WSComplexType::unload(output, "account_number", dynamic_cast<const WSBasicType*>(&m_account_number));
}
