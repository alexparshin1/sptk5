/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       PersistentList.cpp - description                       ║
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

#include <smq/persistent/PersistentList.h>

using namespace std;
using namespace sptk;
using namespace smq;
using namespace persistent;


PersistentList::PersistentList(MemoryPool& pool, const String& name)
: m_pool(pool)
{
    size_t headerLength = sizeof(Header) + name.length();
    m_header = m_pool.insert(nullptr, headerLength);

    auto* header = (Header*) m_header.data();

    header->first.type = PDT_LIST_HEADER;
    strcpy(header->name, name.c_str());
    header->nameLength = name.length();
}

Handle PersistentList::push_front(const void* data, size_t size)
{
    sptk::UniqueLock(m_mutex);

    size_t itemLength = sizeof(Item) + size - 1;
    Handle itemHandle = m_pool.insert(nullptr, itemLength);

    auto* item = (Item*) itemHandle.data();

    item->prior = HandleStorage();
    if (m_handles.empty()) {
        item->next = HandleStorage();
        auto* header = (Header*) m_header.data();
        header->first = itemHandle.storage();
    } else {
        Handle& front = m_handles.front();
        item->next = front.storage();
        Item* frontItem = (Item*) front.data();
        frontItem->prior = itemHandle.storage();
    }

    item->dataLength = size;
    memcpy(item->data, data, size);

    m_handles.push_front(itemHandle);

    return m_handles.front();
}

Handle PersistentList::push_back(const void* data, size_t size)
{
    sptk::UniqueLock(m_mutex);

    size_t itemLength = sizeof(Item) + size - 1;
    Handle itemHandle = m_pool.insert(nullptr, itemLength);

    auto* item = (Item*) itemHandle.data();

    item->next = HandleStorage();
    if (m_handles.empty()) {
        item->prior = HandleStorage();
        auto* header = (Header*) m_header.data();
        header->first = itemHandle.storage();
    } else {
        Handle& back = m_handles.back();
        item->prior = back.storage();
        Item* backItem = (Item*) back.data();
        backItem->next = itemHandle.storage();
    }

    item->dataLength = size;
    memcpy(item->data, data, size);

    m_handles.push_front(itemHandle);

    return m_handles.front();
}


void PersistentList::erase(iterator from, iterator to)
{
    sptk::UniqueLock(m_mutex);
    for(auto& itor = from; itor != to; itor++) {
        auto& handle = *itor;
        m_pool.free(handle);
    }

    auto& prior = from;
    Item* priorItem;
    if (from != m_handles.begin()) {
        prior--;
        priorItem = (Item*) prior->data();
    } else {
        prior = m_handles.end();
        priorItem = nullptr;
    }

    m_handles.erase(from, to);
    auto& next = to;
    Item* nextItem;
    if (next != m_handles.end())
        nextItem = (Item*) next->data();
    else
        nextItem = nullptr;

    if (priorItem != nullptr) {
        if (nextItem != nullptr)
            priorItem->next = next->storage();
        else
            priorItem->next = HandleStorage();
    }

    if (nextItem != nullptr) {
        if (priorItem != nullptr)
            nextItem->prior = prior->storage();
        else
            nextItem->prior = HandleStorage();
    }
}

void PersistentList::clear()
{
    sptk::UniqueLock(m_mutex);
    for (auto& handle: m_handles)
        m_pool.free(handle);
    m_handles.clear();
}


#if USE_GTEST

#ifdef _WIN32
static const char* testPoolDirectory = "C:/Windows/temp/mmf_test";
#else
static const char* testPoolDirectory = "/tmp/mmf_test";
#endif

static void prepareTestDirectory()
{
    if (access(testPoolDirectory, 0) != 0)
        mkdir(testPoolDirectory, 0777);
}

TEST(SMQ_PersistentList, alloc)
{
    prepareTestDirectory();

    MemoryPool pool(testPoolDirectory, "lists", 128 * 1024);
    pool.load(nullptr);
    pool.clear();

    PersistentList list1(pool, "List 00001");
    PersistentList list2(pool, "List 00002");

    for (size_t i = 0; i < 2; i++) {
        String st("Item " + to_string(i));
        list1.push_back(st.c_str(), st.length());
    }
}

#endif
