#include "CLogin.h"
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;

const Strings CLogin::m_fieldNames { "username", "password"};

CLogin::~CLogin()
{
    WSComplexType::clear();
}

void CLogin::_clear()
{
    // Clear elements
    m_username.clear();
    m_password.clear();
}

void CLogin::load(const xml::Element* input)
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

        if (element->name() == "username") {
            m_username.load(element);
            continue;
        }

        if (element->name() == "password") {
            m_password.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_username.throwIfNull("Login");
    m_password.throwIfNull("Login");
}

void CLogin::load(const json::Element* input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);

    // Load elements
    for (auto& itor: input->getObject()) {
        auto& elementName = *itor.first;
        auto* element = itor.second;

        if (elementName == "username") {
            m_username.load(element);
            continue;
        }

        if (elementName == "password") {
            m_password.load(element);
            continue;
        }
    }

    // Check 'required' restrictions
    m_username.throwIfNull("Login");
    m_password.throwIfNull("Login");
}

void CLogin::load(const FieldList& input)
{
    UniqueLock(m_mutex);
    _clear();
    setLoaded(true);
    Field* field;

    // Load elements
    if ((field = input.findField("username")) != nullptr) {
        m_username.load(*field);
    }

    if ((field = input.findField("password")) != nullptr) {
        m_password.load(*field);
    }


    // Check 'required' restrictions
    m_username.throwIfNull("Login");
    m_password.throwIfNull("Login");
}

void CLogin::unload(xml::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_username.addElement(output);
    m_password.addElement(output);
}

void CLogin::unload(json::Element* output) const
{
    SharedLock(m_mutex);

    // Unload elements
    m_username.addElement(output);
    m_password.addElement(output);
}

void CLogin::unload(QueryParameterList& output) const
{
    SharedLock(m_mutex);

    // Unload attributes
    WSComplexType::unload(output, "username", dynamic_cast<const WSBasicType*>(&m_username));
    WSComplexType::unload(output, "password", dynamic_cast<const WSBasicType*>(&m_password));
}
