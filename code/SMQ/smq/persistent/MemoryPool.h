/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MemoryPool.h - description                             ║
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

#ifndef __PERSIST_MEMORY_MANAGER_H__
#define __PERSIST_MEMORY_MANAGER_H__

#include <smq/persistent/MemoryBucket.h>
#include <sptk5/Loop.h>

namespace smq {
namespace persistent {

class SP_EXPORT MemoryPool
{
public:

    MemoryPool(const sptk::String& directory, const sptk::String& objectName, uint32_t bucketSize);

    void clear();
    void load(std::vector<Handle>* handles);

    Handle insert(const void* data, size_t bytes);
    void free(Handle handle);

private:

    std::mutex                          m_mutex;

    sptk::String                        m_directory;
    sptk::String                        m_objectName;
    uint32_t                            m_bucketSize;
    Loop<SMemoryBucket>                 m_bucketLoop;
    std::map<uint32_t,SMemoryBucket>    m_bucketMap;

    uint32_t      makeBucketIdUnlocked();
    SMemoryBucket createBucketUnlocked(uint32_t id);

    SMemoryBucket createBucket(uint32_t id);
};

}
}

#endif
