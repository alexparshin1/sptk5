#include "CLoginResponse.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CLoginResponse::m_fieldNames { "jwt", "|"};

CLoginResponse::~CLoginResponse()
{
    WSComplexType::clear();
}

void CLoginResponse::_clear()
{
    // Clear elements
    m_jwt.clear();
}

void CLoginResponse::load(const xml::Element* input)
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

        if (element->name() == "jwt") {
            m_jwt.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse");
}

void CLoginResponse::load(const json::Element* input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);

    // Load elements
    for (auto& itor: input->getObject()) {
        auto& elementName = *itor.first;
        auto* element = itor.second;

        if (elementName == "jwt") {
            m_jwt.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse");
}

void CLoginResponse::load(const FieldList& input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);
    Field* field;

    // Load elements
    if ((field = input.fieldByName("jwt")) != nullptr) {
        m_jwt.load(*field);
    }


    // Check 'required' restrictions
    m_jwt.throwIfNull("LoginResponse");
}

void CLoginResponse::unload(xml::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_jwt.addElement(output);
}

void CLoginResponse::unload(json::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_jwt.addElement(output);
}

void CLoginResponse::unload(QueryParameterList& output) const
{
    SharedLock(m_mutex);

    // Unload attributes
    WSComplexType::unload(output, "jwt", dynamic_cast<const WSBasicType*>(&m_jwt));
}
