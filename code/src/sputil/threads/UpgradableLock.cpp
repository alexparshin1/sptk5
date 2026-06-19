/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "../../../sptk5/threads/UpgradableLock.h"

using namespace std;
using namespace sptk;

namespace sptk {

UpgradableLock::UpgradableLock() = default;

void UpgradableLock::lockShared()
{
    unique_lock lock(m_mutex);
    m_condition.wait(lock, [this] { return !m_exclusive && !m_upgrading; });
    m_sharedCount++;
    m_sharedOwners.insert(this_thread::get_id());
}

bool UpgradableLock::tryLockShared(chrono::milliseconds timeout)
{
    unique_lock lock(m_mutex);
    if (!m_condition.wait_for(lock, timeout, [this] { return !m_exclusive && !m_upgrading; }))
        return false;
    m_sharedCount++;
    m_sharedOwners.insert(this_thread::get_id());
    return true;
}

void UpgradableLock::unlockShared()
{
    unique_lock lock(m_mutex);
    m_sharedCount--;
    m_sharedOwners.erase(this_thread::get_id());
    m_condition.notify_all();
}

void UpgradableLock::lockExclusive()
{
    unique_lock lock(m_mutex);
    auto tid = this_thread::get_id();
    if (m_sharedOwners.count(tid)) {
        // Upgrade from shared to exclusive: release our shared hold atomically
        m_upgrading = true;
        m_sharedCount--;
        m_sharedOwners.erase(tid);
        m_condition.wait(lock, [this] { return m_sharedCount == 0 && !m_exclusive; });
        m_upgrading = false;
    } else {
        // Fresh exclusive acquisition
        m_condition.wait(lock, [this] { return m_sharedCount == 0 && !m_exclusive && !m_upgrading; });
    }
    m_exclusive = true;
}

bool UpgradableLock::tryLockExclusive(chrono::milliseconds timeout)
{
    unique_lock lock(m_mutex);
    auto tid = this_thread::get_id();
    if (m_sharedOwners.count(tid)) {
        // Upgrade from shared to exclusive: release our shared hold atomically
        m_upgrading = true;
        m_sharedCount--;
        m_sharedOwners.erase(tid);
        if (!m_condition.wait_for(lock, timeout, [this] { return m_sharedCount == 0 && !m_exclusive; })) {
            // Timeout: restore shared state
            m_sharedCount++;
            m_sharedOwners.insert(tid);
            m_upgrading = false;
            m_condition.notify_all();
            return false;
        }
        m_upgrading = false;
    } else {
        // Fresh exclusive acquisition
        if (!m_condition.wait_for(lock, timeout, [this] { return m_sharedCount == 0 && !m_exclusive && !m_upgrading; }))
            return false;
    }
    m_exclusive = true;
    return true;
}

void UpgradableLock::unlockExclusive()
{
    unique_lock lock(m_mutex);
    m_exclusive = false;
    m_condition.notify_all();
}

} // namespace sptk
