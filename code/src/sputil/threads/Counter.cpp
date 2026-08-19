/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
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

#include <mutex>
#include <sptk5/threads/Counter.h>

using namespace std;
using namespace sptk;
using namespace chrono;

Counter::Counter(const int startingValue)
    : m_counter(startingValue)
{
}

Counter::~Counter()
{
    const scoped_lock lock(m_lockMutex);
    m_condition.notify_all();
}

int Counter::get() const
{
    const scoped_lock lock(m_lockMutex);
    return m_counter;
}

void Counter::set(const int value)
{
    const scoped_lock lock(m_lockMutex);
    if (m_counter != value)
    {
        m_counter = value;
        m_condition.notify_all();
    }
}

Counter& Counter::operator++()
{
    const scoped_lock lock(m_lockMutex);
    ++m_counter;
    m_condition.notify_all();
    return *this;
}

Counter& Counter::operator+=(const int value)
{
    const scoped_lock lock(m_lockMutex);
    m_counter += value;
    m_condition.notify_all();
    return *this;
}

Counter& Counter::operator--()
{
    const scoped_lock lock(m_lockMutex);
    --m_counter;
    m_condition.notify_all();
    return *this;
}

Counter& Counter::operator-=(const int value)
{
    const scoped_lock lock(m_lockMutex);
    m_counter -= value;
    m_condition.notify_all();
    return *this;
}

bool Counter::wait_for(int value, const milliseconds& timeout)
{
    unique_lock lock(m_lockMutex);

    if (m_counter == value)
    {
        return true;
    }

    // Wait until m_counter is matching the value
    return m_condition.wait_for(lock,
                                timeout,
                                [this, value]
                                {
                                    return m_counter == value;
                                });
}

[[maybe_unused]] bool Counter::wait_until(int value, const DateTime& timeoutAt)
{
    unique_lock lock(m_lockMutex);

    // Wait until the semaphore value is greater than 0
    return m_condition.wait_until(lock,
                                  timeoutAt.timePoint(),
                                  [this, value]
                                  {
                                      return m_counter == value;
                                  });
}
