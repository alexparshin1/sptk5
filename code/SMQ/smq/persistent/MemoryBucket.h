/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MemoryBucket.h - description                           ║
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

#ifndef __PERSIST_MEMORY_BUCKET_H__
#define __PERSIST_MEMORY_BUCKET_H__

#include <smq/persistent/MemoryMappedFile.h>
#include <smq/persistent/Handle.h>

namespace smq {
namespace persistent {

static constexpr uint16_t allocatedMark = 0x5F7F;
static constexpr uint16_t releasedMark = 0x5E7E;

enum PersistentDataType : uint8_t
{
    PDT_STRING          = 1,
    PDT_LIST_HEADER     = 2,
    PDT_LIST_ITEM       = 4
};

class SP_EXPORT MemoryBucket
{
    friend class Handle;
    struct FreeBlocks
    {
        uint32_t                            m_size;
        std::map<uint32_t,uint32_t>         m_offsetMap;
        std::multimap<uint32_t,uint32_t>    m_sizeMap;

        FreeBlocks(uint32_t size);
        void load(uint32_t offset, uint32_t size);
        void free(uint32_t offset, uint32_t size);
        uint32_t alloc(uint32_t size);
        void clear();
        size_t count() const;
        uint32_t available() const;

        void print() const;
    };

public:

    MemoryBucket(const sptk::String& directoryName, const sptk::String& objectName, uint32_t id, size_t size);

    void load(std::vector<Handle>* handles);

    const uint32_t id() const;
    const void* data() const;

    Handle insert(const void* data, size_t bytes);

    void free(Handle& data);

    void clear();

    bool empty() const;

    size_t size() const;

    size_t available() const;

    static MemoryBucket* find(uint32_t bucketId);

    const FreeBlocks& freeBlocks() { return m_freeBlocks; }

    const sptk::String fileName() const { return m_mappedFile.fileName(); }

protected:

    struct Item
    {
        unsigned            signature:16;
        PersistentDataType  type:8;
        unsigned            size:24;
    };

private:

    mutable std::mutex          m_mutex;
    uint32_t                    m_id;
    sptk::String                m_objectName;
    MemoryMappedFile            m_mappedFile;
    FreeBlocks                  m_freeBlocks;

    static sptk::String formatId(uint32_t bucketId);
};

typedef std::shared_ptr<MemoryBucket> SMemoryBucket;

}
}

#endif