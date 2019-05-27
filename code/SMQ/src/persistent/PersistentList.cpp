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
using namespace chrono;

PersistentList::ItemHandle::operator void*() const
{
    auto* item = (ItemStorage*) header();
    if (item->signature != allocatedMark)
        return nullptr;
    return (void*)((const char*) item->data);
}

void* PersistentList::ItemHandle::data() const
{
    auto* item = (ItemStorage*) header();
    if (item->signature != allocatedMark)
        return nullptr;
    return (void*)((const char*) item->data);
}

const char* PersistentList::ItemHandle::c_str() const
{
    auto* item = (ItemStorage*) header();
    if (item->signature != allocatedMark)
        return nullptr;
    return (const char*) item->data;
}

size_t PersistentList::ItemHandle::size() const
{
    auto* item = (ItemStorage*) header();
    return item->dataLength;
}

PersistentList::PersistentList(MemoryPool& pool, const String& name)
: m_pool(pool), m_name(name)
{
    size_t headerLength = sizeof(ItemStorage) + name.length();
    m_header = m_pool.insert(nullptr, headerLength);

    auto* headerPtr = header();
    headerPtr->type = HT_LIST_HEADER;
    strcpy((char*)headerPtr->data, name.c_str());
    headerPtr->dataLength = name.length();
}

PersistentList::PersistentList(MemoryPool& pool, Handle& header)
: m_pool(pool), m_header(header)
{
    auto* headerPtr = this->header();
    if (headerPtr->type != HT_LIST_HEADER)
        throw Exception("Handle is not a list header");
    m_name = String((const char*)headerPtr->data, headerPtr->dataLength);
}

Handle PersistentList::push_front(const void* data, size_t size)
{
    sptk::UniqueLock(m_mutex);

    size_t itemLength = sizeof(ItemStorage) + size - 1;
    auto itemHandle = ItemHandle(m_pool.insert(nullptr, itemLength));

    auto* item = this->item(itemHandle);

    auto* header = this->header();
    header->next = itemHandle.location();

    if (!m_handles.empty()) {
        Handle& front = m_handles.front();
        item->next = front.location();
    }

    item->dataLength = size;
    memcpy(item->data, data, size);

    m_handles.push_front(itemHandle);

    return m_handles.front();
}

Handle PersistentList::push_back(const void* data, size_t size)
{
    sptk::UniqueLock(m_mutex);

    size_t itemLength = sizeof(ItemStorage) + size - 1;
    auto itemHandle = ItemHandle(m_pool.insert(nullptr, itemLength));

    auto* item = this->item(itemHandle);

    item->type = HT_LIST_ITEM;
    item->next = Location();
    if (m_handles.empty()) {
        auto* header = this->header();
        header->next = itemHandle.location();
    } else {
        Handle& back = m_handles.back();
        ItemStorage* backItem = this->item(back);
        backItem->next = itemHandle.location();
    }

    item->dataLength = size;
    memcpy(item->data, data, size);

    m_handles.push_back(itemHandle);

    return m_handles.back();
}


void PersistentList::erase(iterator from, iterator to)
{
    sptk::UniqueLock(m_mutex);
    for(auto& itor = from; itor != to; itor++) {
        auto& handle = *itor;
        m_pool.free(handle);
    }

    auto& prior = from;
    ItemStorage* priorItem;
    if (from != m_handles.begin()) {
        prior--;
        priorItem = item(*prior);
    } else {
        prior = m_handles.end();
        priorItem = nullptr;
    }

    m_handles.erase(from, to);
    auto& next = to;
    ItemStorage* nextItem;
    if (next != m_handles.end())
        nextItem = item(*next);
    else
        nextItem = nullptr;

    if (priorItem != nullptr) {
        if (nextItem != nullptr)
            priorItem->next = next->location();
        else
            priorItem->next = Location();
    }
}

void PersistentList::clear()
{
    sptk::UniqueLock(m_mutex);
    for (auto& handle: m_handles)
        m_pool.free(handle);
    m_handles.clear();

    header()->next = Location();
}

void PersistentList::load()
{
    sptk::UniqueLock(m_mutex);

    m_handles.clear();

    auto* headerPtr = header();
    if (headerPtr->next.empty())
        return;

    ItemStorage* itemPtr = nullptr;
    for (Location location = headerPtr->next; !location.empty(); location = itemPtr->next) {
        ItemHandle itemHandle(location);
        m_handles.push_back(itemHandle);
        itemPtr = item(itemHandle);
    }
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

    size_t maxItems = 128 * 1024;
    MemoryPool pool(testPoolDirectory, "lists", maxItems);
    SHandles listHandles = make_shared<Handles>();
    pool.load(listHandles, HT_LIST_HEADER);

    DateTime started("now");
    size_t totalItems = 0;
    for (auto& handle: *listHandles) {
        PersistentList list(pool, handle);
        list.load();
        totalItems += list.size();
/*
        for (auto& item: list) {
            String s(item.c_str(), item.size());
            cout << item.c_str() << endl;
        }
        cout << "--------------------------" << endl;
*/
    }
    DateTime ended("now");
    double durationSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
    COUT("Total " << totalItems << " items loaded for " << durationSec << ", " << totalItems / durationSec << "/sec");

    pool.clear();

    PersistentList list1(pool, "List 00001");
    PersistentList list2(pool, "List 00002");

    totalItems = 0;
    started = DateTime::Now();
    for (size_t i = 0; i < maxItems; i++) {
        String st("List 1 Item " + to_string(i));
        list1.push_back(st.c_str(), st.length());
        //String st2("List 2 Item " + to_string(i));
        //list2.push_back(st2.c_str(), st2.length());
        totalItems += 2;
    }
    ended = DateTime::Now();
    durationSec = duration_cast<milliseconds>(ended - started).count() / 1000.0;
    COUT("Total " << totalItems << " items inserted for " << durationSec << ", " << totalItems / durationSec << "/sec");
}

#endif
