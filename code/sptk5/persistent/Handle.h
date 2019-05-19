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

namespace sptk {
namespace persistent {

class MemoryBucket;

class SP_EXPORT Handle
{
    friend class MemoryBucket;
    MemoryBucket*   m_bucket {nullptr};

protected:

    void*           m_record {nullptr};

public:

    Handle() {}
    Handle(MemoryBucket& bucket, size_t m_bucketOffset);
    Handle(size_t bucketId, size_t m_bucketOffset);
    Handle(const Handle& other) = default;
    Handle& operator = (const Handle& other) = default;
    bool isNull() const { return m_bucket == nullptr; }
    explicit operator void* () const;
    void* data() const;
    const char* c_str() const;
    size_t size() const;
};

}
}

#endif
