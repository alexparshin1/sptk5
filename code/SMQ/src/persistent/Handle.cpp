/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Handle.cpp - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday May 19 2019                                     ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <smq/persistent/MemoryBucket.h>
#include <SMQ/smq/persistent/Handle.h>


using namespace std;
using namespace sptk;
using namespace smq::persistent;

Handle::Handle()
: m_bucket(nullptr), m_record(nullptr)
{
}

Handle::Handle(MemoryBucket& bucket, size_t m_bucketOffset)
: m_bucket(&bucket), m_record((HandleStorage*)((const char*)m_bucket->data() + m_bucketOffset))
{
}

Handle::Handle(size_t bucketId, size_t m_bucketOffset)
{
    m_bucket = MemoryBucket::find(uint32_t(bucketId));
    if (m_bucket == nullptr)
        throw Exception("Bucket doesn't exist");
    m_record = (HandleStorage*)((const char*)m_bucket->data() + m_bucketOffset);
}

Handle::operator void*() const
{
    auto* item = (HandleStorage*) m_record;
    if (item->signature != allocatedMark)
        return nullptr;
    return (void*)((const char*) m_record + sizeof(HandleStorage));
}

void* Handle::data() const
{
    auto* item = (HandleStorage*) m_record;
    if (item->signature != allocatedMark)
        return nullptr;
    return (void*)((const char*) m_record + sizeof(HandleStorage));
}

const char* Handle::c_str() const
{
    auto* item = (HandleStorage*) m_record;
    if (item->signature != allocatedMark)
        return nullptr;
    return (const char*) m_record + sizeof(HandleStorage);
}

size_t Handle::size() const
{
    auto* item = (HandleStorage*) m_record;
    if (item->signature != allocatedMark)
        return 0;
    return m_record->size;
}

void Handle::free()
{
    if (m_bucket != nullptr)
        m_bucket->free(*this);
    m_bucket = nullptr;
    m_record = nullptr;
}

uint32_t Handle::storageSize() const
{
    return sizeof(HandleStorage);
}

void Handle::store(void* destination)
{
    memcpy(destination, m_record, sizeof(HandleStorage));
}

void Handle::restore(void* source)
{
    memcpy(m_record, source, sizeof(HandleStorage));
}
