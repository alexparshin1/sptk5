/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-13                                             ║
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

#include "sptk5/MoneyData.h"

#include <sptk5/Exception.h>

using namespace std;
using namespace sptk;

namespace {
constexpr auto                             numberOfDividers = 16;
constexpr array<int64_t, numberOfDividers> dividers = {
    1, 10, 100, 1000, 10000, 100000, 1000000L, 10000000L, 100000000LL,
    1000000000LL, 10000000000LL, 100000000000LL, 1000000000000LL,
    10000000000000LL, 100000000000000LL, 1000000000000000LL};
} // namespace

void MoneyData::setScale(const uint8_t scale)
{
    if (scale >= dividers.size())
    {
        throw std::out_of_range("MoneyData: scale is out of range");
    }
    m_scale = scale;
}

int64_t MoneyData::divider(const uint8_t scale)
{
    if (scale >= dividers.size())
    {
        throw Exception("MoneyData: scale is out of range");
    }
    return dividers[scale];
}

MoneyData::MoneyData(const int64_t quantity, const uint8_t scale)
    : m_quantity(quantity)
    , m_scale(scale)
{
    if (scale >= dividers.size())
    {
        throw std::out_of_range("MoneyData: scale is out of range");
    }
}

MoneyData::operator double() const
{
    return static_cast<double>(m_quantity) / static_cast<double>(dividers[m_scale]);
}

MoneyData::operator int64_t() const
{
    return m_quantity / dividers[m_scale];
}

MoneyData::operator int32_t() const
{
    return static_cast<int>(m_quantity / dividers[m_scale]);
}

MoneyData::operator bool() const
{
    return m_quantity != 0;
}
