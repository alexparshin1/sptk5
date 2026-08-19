/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-06                                             ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/db/QueryParameter.h>

using namespace std;
using namespace sptk;

void QueryParameter::bindAdd(const uint32_t bindIndex)
{
    m_bindParamIndexes.push_back(bindIndex);
}

uint32_t QueryParameter::bindCount() const
{
    return static_cast<uint32_t>(m_bindParamIndexes.size());
}

uint32_t QueryParameter::bindIndex(const uint32_t index) const
{
    if (index >= m_bindParamIndexes.size())
    {
        throw Exception("Invalid bind index");
    }
    return m_bindParamIndexes[index];
}

QueryParameter::QueryParameter(const char* name, const bool isOutput)
    : m_binding(isOutput)
    , m_name(name == nullptr ? "" : lowerCase(name))
{
    constexpr size_t initialBindIndexesCapacity = 4;
    m_bindParamIndexes.reserve(initialBindIndexesCapacity);
}

QueryParameter::QueryParameter(const std::string_view name, const bool isOutput)
    : m_binding(isOutput)
    , m_name(lowerCase(name))
{
    constexpr size_t initialBindIndexesCapacity = 4;
    m_bindParamIndexes.reserve(initialBindIndexesCapacity);
}

string QueryParameter::name() const
{
    return m_name;
}

void QueryParameter::setOutput()
{
    m_binding.setOutput();
}

QueryParameter& QueryParameter::operator=(const Variant& param)
{
    if (this != &param)
    {
        setData(param);
    }
    return *this;
}

void QueryParameter::setString(const char* value, const size_t maxLength)
{
    size_t valueLength = 0;
    if (value)
    {
        valueLength = maxLength != 0 ? maxLength : strlen(value);
    }

    setBuffer(reinterpret_cast<const uint8_t*>(value), valueLength, VariantDataType::VAR_STRING);
}
