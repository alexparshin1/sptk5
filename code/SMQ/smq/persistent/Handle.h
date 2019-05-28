/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Handle.h - description                                 ║
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

#ifndef __PERSIST_HANDLE_H__
#define __PERSIST_HANDLE_H__

#include <sptk5/sptk.h>

namespace smq {
namespace persistent {

enum HandleType : uint8_t
{
    HT_UNKNOWN         = 0,
    HT_STRING          = 1,
    HT_LIST_HEADER     = 2,
    HT_LIST_ITEM       = 4
};

#pragma pack(push,1)
struct Location
{
    uint16_t    bucketId {0};
    uint32_t    offset {0};
    Location() {}
    Location(uint32_t _bucketId, uint32_t _offset) : bucketId(_bucketId), offset(_offset) {}
    bool empty() const { return bucketId == 0; }
};
#pragma pack(pop)

class MemoryBucket;

#pragma pack(push,1)
struct HandleStorage
{
    uint16_t    signature;
    HandleType  type;
    uint32_t    size;
};
#pragma pack(pop)


class SP_EXPORT Handle
{
    friend class MemoryBucket;
    MemoryBucket*   m_bucket {nullptr};

protected:

    HandleStorage*  m_record {nullptr};

public:

    Handle();
    Handle(MemoryBucket& bucket, size_t bucketOffset);
    Handle(size_t bucketId, size_t bucketOffset);
    Handle(Location& location);
    Handle(const Handle& other) = default;

    virtual ~Handle() = default;

    Handle& operator = (const Handle& other) = default;
    bool isNull() const { return m_bucket == nullptr; }
    void* header() const { return m_record; }
    explicit virtual operator void* () const;
    virtual void* data() const;
    virtual const char* c_str() const;
    virtual size_t size() const;
    HandleType type() const { return m_record == nullptr? HT_UNKNOWN: m_record->type; }
    void free();

    uint32_t bucketId() const;
    uint32_t offset() const;
    Location location() const;

    uint32_t storageSize() const;
    HandleStorage& storage() const { return *m_record; }
    void store(void *destination);
    void restore(void *source);
};

typedef std::vector<Handle>         Handles;
typedef std::shared_ptr<Handles>    SHandles;

}
}

#endif
